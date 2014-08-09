#!/bin/bash


while true 
do
	temperature=`nc alarm 666 | cut -d" " -f2`
	humidity=$(nc alarm 666 | cut -d" " -f4)
	echo $temperature
	echo $humidity
	sleep 5
done

