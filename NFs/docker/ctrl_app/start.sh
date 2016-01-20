#! /bin/bash

#This gives time to the UN to configure the network namespace of the container
sleep 3


echo "Control app container started"

echo "add controller interface 10.0.10.100/24 on eth0"
ip addr add 10.0.10.100/24 dev eth0
#ifconfig eth0 0
#ip addr add 10.0.10.100/24 dev eth0

echo "start ryu"
cd ryu_app/
ryu-manager ctrl_app_er_un_v1.py

# keep container running
#while true
#do
#	sleep 1
#done

