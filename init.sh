#!/bin/bash  
echo "This script is creating a tree of iamroot"

#./iamroot $streamID:127.0.0.1:59000 -i 127.0.0.1 -t $port1 -u $port2
workDir="/home/francisco/Documents/RCI/Project/RCI-Peer-to-Peer-Stream-Delivery/"


numberOfIAMROOT=11
streamID="maregaa"
streamPort="40300"
tcpSessions=3
bestpops=2



port1=40380 #$(($streamPort+1))
port2=$(($port1+1))

#io.elementary.terminal -e nc -p $streamPort -l

counter=1
while [ $counter -le $numberOfIAMROOT ]
do
	io.elementary.terminal -e "./init2.sh $streamID $streamPort $port1 $port2 $tcpSessions $bestpops"
	echo $counter
	((counter++))
	port1=$(($port1+2))
	port2=$(($port2+2))

	sleep 1
done

