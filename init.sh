#!/bin/bash  
echo "This script is creating a tree of iamroot"

#./iamroot $streamID:127.0.0.1:59000 -i 127.0.0.1 -t $port1 -u $port2
workDir="/home/francisco/Documents/RCI/Project/RCI-Peer-to-Peer-Stream-Delivery/"
numberOfIAMROOT=1
streamID="francisco1"
streamPort="58002"
port1=30002
port2=$(($port1+1))


counter=1
while [ $counter -le $numberOfIAMROOT ]
do
	io.elementary.terminal -e "./init2.sh $streamID $streamPort $port1 $port2"
	echo $counter
	((counter++))
	port1=$(($port1+2))
	port2=$(($port2+2))
done
