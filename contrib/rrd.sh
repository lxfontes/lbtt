#!/bin/bash

case "$1" in
create)
rrdtool create "$2" \
--start N --step 300 \
DS:hosts:GAUGE:600:U:U \
DS:seeders:GAUGE:600:U:U \
DS:files:GAUGE:600:U:U \
RRA:MIN:0.5:12:1440 \
RRA:MAX:0.5:12:1440 \
RRA:AVERAGE:0.5:1:1440
;;
update)
eval $(curl -s "$3" | sed 's/\":\ /=/g;s/{/export/g;s/["},]/\ /g')
if [ $? -ne 0 ]; then echo "error";exit;fi
rrdtool update "$2" \
"N:$peers:$seeders:$torrents"
;;
graph)
rrdtool graph "$3" -a PNG --title "Stats" \
"DEF:hosts=$2:hosts:AVERAGE" \
"DEF:seeders=$2:seeders:AVERAGE" \
"DEF:torrents=$2:files:AVERAGE" \
'AREA:hosts#ff0000:Hosts' \
'GPRINT:hosts:LAST:%.0lf' \
'LINE2:seeders#000000:Seeders' \
'GPRINT:seeders:LAST:%.0lf' \
'AREA:torrents#00ff00:Torrents' \
'GPRINT:torrents:LAST:%.0lf' 

;;
esac
