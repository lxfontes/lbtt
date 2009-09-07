
print("loading...");
/*
mysql example
my = mysqlQuery("SELECT NOW() as t1,NOW() as t2")
print(my.t1 + " " my.t2)

*/


/*
this function receives a request object + torrent + peer
request.newTorrent = boolean ( if set, request.torrent will exist )
request.newPeer = boolean ( if set, request.peer will exist )

request properties = event,left,corrupt,downloaded,uploaded,numwant,port,ip,passkey,hexhash,peerid
torrent properties = hexhash,complete,incomplete,download,lastseen
peer properties = hexpeerid,ip,port,left,downloaded,uploaded,corrupt,passkey,lastseen
*/
function newRequest(request){
if(request.newTorrent == false){
	print(request.info_hash +" "+request.torrent.seeders+" seeders, "+request.torrent.leechers+" leechers "+request.torrent.hosts+" hosts")
}else{
	print("new Torrent " + request.info_hash)
}

/*
string returned here will be presented as "failure msg"
return "failure, you're too ugly"
*/
}

function expirePeer(torrent,peer){
	print("expire peer "+ torrent.info_hash +" "+torrent.seeders+" seeders, "+torrent.leechers+" leechers "+torrent.hosts+" hosts")
}

function expireTorrent(torrent){
	print("expire torrent"+ torrent.info_hash +" "+torrent.seeders+" seeders, "+torrent.leechers+" leechers "+torrent.hosts+" hosts")
}

//this will run @ the start
print("javascript interface initiated");
/* connect mysql if needed
print("MYSQL Connect? " + mysqlConnect("localhost","tb","tr4ck3r","bittorrent"));
*/
