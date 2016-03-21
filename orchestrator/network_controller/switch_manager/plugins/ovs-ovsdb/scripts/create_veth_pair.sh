#!/bin/bash

#Author: UNIFY Consortium
#Date: 2016-01-18
#Brief: Create a veth pair

#$1 name of the first end
#$2 name of the second end

if (( $EUID != 0 ))
then
    echo "[$0] This script must be executed with ROOT privileges"
    exit 0
fi

echo "[$0] Making sure this interface is deleted first: $2"

ip link delete $2


echo "[$0] Creating the veth pair $1 - $2"

ip link add $1 type veth peer name $2

echo "[$0] Veth pair $1 - $2 created"

ip link set $1 up
ip link set $2 up

exit 1
