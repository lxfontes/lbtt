var slots = []

/*
mysql example
my = mysqlQuery("SELECT NOW() as t1,NOW() as t2")
print(my.t1 + " " my.t2)

*/


/*
this function receives a request object + torrent + peer
request.newTorrent = boolean ( if set, request.torrent will exist )
request.newPeer = boolean ( if set, request.peer will exist )

request properties = event,left,corrupt,downloaded,uploaded,numwant,port,ip,passkey,infohash,peerid
torrent properties = infohash,complete,incomplete,download,lastseen
peer properties = peerid,ip,port,left,downloaded,uploaded,corrupt,passkey,lastseen
*/

function updateUsage(passkey,upload,download){
	mysqlQuery("update users set uploaded=uploaded+"+upload+",downloaded=downloaded+"+download+" WHERE passkey='"+passkey+"'")
}
function newRequest(request){


if(request.event != "started" && request.newPeer == true){
	//new peers must send the event start
	return "please restart torrent"
}


if(request.event == "started"){
	if(request.newTorrent == true){
		//check if torrent exists
		//mysql lookup
		mq = mysqlQuery("select 1 as valid from torrents where infohash='"+req.infohash+"'")
		if(!mq){
			return "torrent not found"
		}
	}
	
	if(request.newPeer == true){
		//mysql lookup
		mq = mysqlQuery("select 1 as valid from users where passkey='"+req.passkey+"'")
		if(!mq){
			return "user not found"
		}
	}

	//check if this guy has available slots
	if(!slots[request.passkey]){
		slots[request.passkey]=1
	}else if(slots[request.passkey] > 5){
		return "max slots reached"
	}else{
		slots[request.passkey]++
	}


}


if(request.newPeer == false && request.event != "started"){
	//check if previous sent values are sane
	if(request.downloaded < request.peer.downloaded){
		return "invalid download lenght"
	}
}

if(request.event == "stopped"){
	updateUsage(request.peer.passkey,request.uploaded,request.downloaded)
}

print(request.passkey + "@" + request.ip + " " +request.event)
return undefined;
}

function expirePeer(torrent,peer){
	updateUsage(peer.passkey,peer.uploaded,peer.downloaded)
}

function expireTorrent(torrent){
	
}

//this will run @ the start
print("javascript interface initiated");
