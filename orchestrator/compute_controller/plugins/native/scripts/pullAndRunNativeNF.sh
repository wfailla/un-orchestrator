#!/bin/bash

#Author: Sergio Nuccio
#Date: October 21st 2015
#Brief: pull a native NF from a remote repository or a folder on this system, and run it. 

#command line: 
#	sudo ./nfs_manager/scripts/native/pullAndRunNativeNF.sh $1 $2 $3 $4 [$5 ...]

#$1 LSI ID				(e.g., 2)
#$2 NF name				(e.g., firewall)
#$3 registry/nf[:tag] 			(e.g., localhost:5000/pcap:latest)
#$4 number_of_ports			(e.g., 2)
#The next $4 parameters are port names to be provided to the container (e.g., vEth0 vEth1)
#The next $4 parameters are IPv4 addresses / network to be associated with that ports (e.g., 10.0.0.1/24)
#	0 if no IPv4 address must be associated
#The next $4 parameters are Eth addresses to be associated with that ports (e.g., aa:aa:aa:aa:aa:aa)
#	0 if no Ethernet address must be associated

if (( $EUID != 0 )) 
then
    echo "[pullAndRunNativeNF] This script must be executed with ROOT privileges"
    exit 0
fi

tmp_file="$1_$2_tmp"

echo "" > $tmp_file

#Retrieve the script, and rename it
script_name=`echo $1"_"$tmp_file"_"$2`

tmp=$3

begin=${tmp:0:7}
# file:// means that the NF is local
if [ $begin == "file://" ]
then
	#The NF is in the local file system
	path=${tmp:7:${#tmp}}
	
	cp $path $script_name
else
	#The NF must be retrieved from a remote url
	sudo wget -O $script_name $3
	#wget returns 0 in case of success
fi

ret=`echo $?`

if [ $ret -eq 0 ]
then
	echo "[pullAndRunNativeNF] Function '"$3"' retrieved"
else
	echo "[pullAndRunNativeNF] Impossible to retrieve function '"$3"'"
	rm $tmp_file
	exit 0
fi

#configure network for native network function
current=5
currentIp=`expr $current + $4`
currentEthernet=`expr $currentIp + $4`
for (( c=0; c<$4; c++ ))
do
	#create virtual interface connected to the port of the vswitch (already created)
	#name of the ovs port on the switch: <nf_name>p<#port>b<lsi_id>  => ${2}p${c+1}b${1}
 	echo ip link add link ${2}p$((c+1))b${1} name ${!current} type macvtap

 	if [ ${!currentEthernet} != 0 ]
 	then
 		echo ip link set ${!current} address ${!currentEthernet}
 	fi
 	
 	if [ ${!currentIp} != 0 ]
 	then
		#configure ip address for the interface???
		echo ifconfig ${!current} ${!currentIp} up
	fi
	
	echo ip link set ${!current} up
	
	current=`expr $current + 1`
	currentIp=`expr $currentIp + 1`
	currentEthernet=`expr $currentEthernet + 1`
done 

#prepare the command

sudo chmod +x $script_name

echo -ne sudo ./$script_name $1 $4 > $tmp_file

#passing port names to the script
current=5
for((c=0; c<$4; c++))
do
	echo -ne " " ${!current} >> $tmp_file
	current=`expr $current + 1`
done

echo [`date`]"[pullAndRunNativeNF] Executing command: '"`cat $tmp_file`"'"

bash $tmp_file

rm $tmp_file
rm $script_name

exit 1
