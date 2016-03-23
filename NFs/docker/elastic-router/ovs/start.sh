#! /bin/bash

#This gives time to the UN to configure the network namespace of the container
#sleep 10

up=false

while [ "$up" == false ]
do
t1=$(ifconfig | grep -o eth5)
t2='eth5'

if [ "$t1" = "$t2" ]; then
  echo "eth5 up"
  up=true
else
  echo "eth5 down, waiting for all eth ports to be up..."
  sleep 1
fi
done

echo "Ubuntu started"
echo "start ovs"
service openvswitch-switch start

echo "start ssh"
service ssh start

NAME=$VNF_NAME

# rename the ovs interfaces
ip link set dev eth2 down
ip link set dev eth2 name ${NAME}_eth0
ip link set dev ${NAME}_eth0 up
ip link set dev eth3 down
ip link set dev eth3 name ${NAME}_eth1
ip link set dev ${NAME}_eth1 up
ip link set dev eth4 down
ip link set dev eth4 name ${NAME}_eth2
ip link set dev ${NAME}_eth2 up
ip link set dev eth5 down
ip link set dev eth5 name ${NAME}_eth3
ip link set dev ${NAME}_eth3 up


#echo "setup ovs bridge"
ovs-vsctl add-br $NAME
ovs-vsctl set bridge $NAME datapath_type=netdev
ovs-vsctl set bridge $NAME protocols=OpenFlow10,OpenFlow12,OpenFlow13
ovs-vsctl set-fail-mode $NAME secure
ovs-vsctl set bridge $NAME other_config:disable-in-band=true
ovs-vsctl set bridge $NAME other-config:datapath-id=$OVS_DPID


ovs-vsctl add-port $NAME ${NAME}_eth0
ovs-vsctl add-port $NAME ${NAME}_eth1
ovs-vsctl add-port $NAME ${NAME}_eth2
ovs-vsctl add-port $NAME ${NAME}_eth3

#tcp:10.0.10.100:6633
ovs-vsctl set-controller $NAME $CONTROLLER

# keep container running
while true
do
	sleep 1
done

