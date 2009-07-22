var teste=0;

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
function newRequest(request){
teste++;
print("ip "+request.ip+" event "+request.event)
if(request.newPeer == false){
	print("peer ip "+request.peer.ip)
}
if(request.newTorrent == false){
	print("hosts in this torrent " + (request.torrent.complete + request.torrent.incomplete))
}
return undefined;
}

function expirePeer(torrent,peer){
	print("peer " + peer.downloaded)
	print("torrent " + torrent.incomplete)
}

function expireTorrent(torrent){
	
}

//this will run @ the start
print("javascript interface initiated");
