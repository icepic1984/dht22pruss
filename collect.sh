#!/bin/bash
#Data is read every 10 seconds
#Heartbeat is 20 seconds
#
#RRA1: every 10 seconds for 3 days
#RRA2: every hour for 7 days
#RRA3: every day for 3 years

ROBINFILE=/mnt/data/temp.rrd

if [ ! -f $ROBINFILE ]
then
	echo "Create $ROBINFILE"
	rrdtool create $ROBINFILE                \
		--step 10                         \
		DS:temperature:GAUGE:20:U:U       \
		DS:humidity:GAUGE:20:U:U          \
		RRA:AVERAGE:0.5:1:25920           \
		RRA:AVERAGE:0.5:360:168           \
		RRA:AVERAGE:0.5:8640:1095         \
		RRA:MAX:0.5:1:25920               \
		RRA:MAX:0.5:360:168               \
		RRA:MAX:0.5:8640:1095             \
		RRA:MIN:0.5:1:25920               \
		RRA:MIN:0.5:360:168               \
		RRA:MIN:0.5:8640:1095
fi

while true 
do
	humidity=`nc alarm 666 | cut -d" " -f2`
	temperature=$(nc alarm 666 | cut -d" " -f4)
	rrdtool update $ROBINFILE N:$temperature:$humidity
	echo "Temp: $temperature Hum: $humidity"
	sleep 10 
done

