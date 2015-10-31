#!/bin/bash

#Author: Sergio Nuccio
#Date: October 31st 2015
#Brief: dummy native network function that does nothing

#command line: 
#	sudo ./nativeNF_example.sh $1 $2 $3 $4 $5

#dependencies: iptables, ebtables

#$1 LSI ID								(e.g., 2)
#$2 NF name								(e.g., firewall)
#$3 number_of_ports							(it is supposed to be 2 for this NF)
#$4 and $5 names of port1 and port2 respectively			(e.g., vEth0 vEth1)

if (( $EUID != 0 )) 
then
    echo "[nativeNF_example] This script must be executed with ROOT privileges"
    exit 0
fi

#enable ipv4 forwarding
sysctl -w net.ipv4.ip_forward=1

current=4
for (( c=0; c < 2; c++ ))
do
	
	ebtables -A forward -i ${!current} -j mark --set-mark ${!current}_mark --mark-target CONTINUE
	current=`expr $current + 1`
done 

#write file that specifies actions to do in order to stop the NF and clean the system
stop_file="$1_$2_stop"

echo "" > $stop_file
current=4
for (( c=0; c < 2; c++))
do
	echo ebtables -D forward -i ${!current} >> $stop_file
	current=`expr $current + 1`
done


echo "[nativeNF_example] script executed"

exit 1
