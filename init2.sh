#!/bin/bash  
echo "This script is creating a iamroot"

cd ~/home/francisco/Documents/RCI/Project/RCI-Peer-to-Peer-Stream-Delivery/

./iamroot $1:127.0.0.1:$2 -i 127.0.0.1 -t $2 -u $3