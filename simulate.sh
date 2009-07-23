#!/bin/sh
tracker() {
PEERID=$1
PORT=$2
PASSKEY=$3
IP=$((PORT-900))
if [ ! -z "$4" ]; then
EVENT="&event=$4"
fi
curl -s "http://localhost:8080/announce?info_hash=%86%a4%ba%8d%2f%97%de%10%25%e8%3f%d7%99%3agl%11%22%e1%d0&peer_id=-$PEERID-z8%835%8e%92xIG%28%c2U&port=$PORT&uploaded=30&downloaded=10&left=202375168&corrupt=0&key=EB57DF4D&numwant=200&no_peer_id=1$EVENT&passkey=$PASSKEY&compact=1&ip=99.22.11.$IP" 2>&1 > /dev/null
echo -n "."
}

status(){
curl -i "http://www.l3f.org/lbtt/status"
echo
}



echo -n "starting "
for port in $(seq 1000 1099);do
tracker 12$port $port asda started
done
echo
sleep 10

echo -n "updating "
for port in $(seq 1000 1099);do
tracker 12$port $port asda
done
echo
sleep 10

echo -n "stopping "
for port in $(seq 1000 1099);do
tracker 12$port $port asda stopped
done
echo

sleep 10
status

