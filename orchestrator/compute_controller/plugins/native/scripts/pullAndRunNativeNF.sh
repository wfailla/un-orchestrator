#!/bin/bash

#Author: Sergio Nuccio
#Date: October 21st 2015
#Brief: pull a native NF from a remote repository or a folder on this system, and run it. 

#command line: 
#	sudo ./nfs_manager/scripts/native/pullAndRunNativeNF.sh $1 $2 $3 $4 [$5 ...]

#$1 LSI ID										(e.g., 2)
#$2 NF name										(e.g., firewall)
#$3 registry/nf[:tag] 									(e.g., localhost:5000/pcap:latest)
#$4 number_of_ports									(e.g., 2)
#The next $4 parameters are the names of the port of the NF				(e.g., vEth0 vEth1)
#The next $4 parameters are IPv4 addresses / network to be associated with that ports 	(e.g., 10.0.0.1/24)
#	0 if no IPv4 address must be associated
#The next $4 parameters are Eth addresses to be associated with that ports 		(e.g., aa:aa:aa:aa:aa:aa)
#	0 if no Ethernet address must be associated

if (( $EUID != 0 )) 
then
    echo "[pullAndRunNativeNF] This script must be executed with ROOT privileges"
    exit 0
fi



tmp_file="$1_$2_tmp"

echo "" > $tmp_file

#Retrieve the archive, and rename it
archive_file=`echo $1"_"$tmp_file"_"$2.tar.gz`

tmp=$3

remote=false

begin=${tmp:0:7}
# file:// means that the NF is local
if [ $begin == "file://" ]
then
	#The NF is in the local file system
	path=${tmp:7:${#tmp}}
	
	cp $path $archive_file
else
	#The NF must be retrieved from a remote URL
	remote=true
	sudo wget -O $archive_file $3
	#wget returns 0 in case of success
fi

ret=`echo $?`

if [ $ret -eq 0 ]
then
	echo "[pullAndRunNativeNF] Function '"$3"' retrieved"
else
	echo "[pullAndRunNativeNF] Unable to retrieve function '"$3"'"
	rm $tmp_file
	if [ $remote == true ]
	then
		rm $archive_file
	fi
	exit 0
fi

#extract the archive in a temporary directory

temp_dir=`echo $1"_"$tmp_file"_"$2`

mkdir $temp_dir

tar -xzf $archive_file -C $temp_dir

ret=`echo $?`

if [ $ret -eq 0 ]
then

	echo "[pullAndRunNativeNF] Successful extracted from archive"

else

	echo "[pullAndRunNativeNF] Unable to extract native function '"$2"' from the archive"
	
	rm $tmp_file
	rm $archive_file
	
	exit 0
	
fi

#check if the start script/executable exists

start="$temp_dir/start"

if [ ! -f $start ]
then
	echo "[pullAndRunNativeNF] Start script for native function '"$2"' not found!"
	exit 0
fi	


#configure network for native network function
current=5
currentIp=`expr $current + $4`
currentEthernet=`expr $currentIp + $4`
for (( c=0; c<$4; c++ ))
do

	#create virtual interface connected to the port of the vswitch (already created)
	#name of the ovs port on the switch: <lsi_id>_<nf_name>_<i> => ${1}_${2}_$((c+1))
	#!not needed anymore
 	#ip link add link ${1}_${2}_$((c+1)) name ${!current} type macvtap

 	if [ ${!currentEthernet} != 0 ]
 	then
 		ip link set ${!current} address ${!currentEthernet}
 	fi
 	
 	if [ ${!currentIp} != 0 ]
 	then
		#configure ip address for the interface???
		ifconfig ${!current} ${!currentIp} up
	fi
	
	ip link set ${!current} up
	ip link set ${!current} promisc on
	ip link set ${!current} arp on
	
	
	current=`expr $current + 1`
	currentIp=`expr $currentIp + 1`
	currentEthernet=`expr $currentEthernet + 1`
done 

#prepare the command

sudo chmod +x $start

echo -ne sudo ./$start $1 $2 $4 > $tmp_file

#passing port names to the script
current=5
for((c=0; c<$4; c++))
do
	echo -ne " " ${!current} >> $tmp_file
	current=`expr $current + 1`
done

#command:= <script_name> <LSI_ID> <NF_NAME> <#PORTS> <PORT_NAMES ...>

echo [`date`]"[pullAndRunNativeNF] Executing command: '"`cat $tmp_file`"'"

bash $tmp_file

rm $tmp_file
rm $archive_file

#check if stop file exists

stop="$temp_dir/stop"

if [ -f $stop ]
then

	chmod +x $stop
	
	#write a file that specifies the command to invoke the stop file
	stop_command="$temp_dir/$1_$2_stop"
	
	echo -ne sudo ./$stop $1 $2 $4 > $stop_command
	
	#passing port names to the stop script
	current=5
	
	for((c=0; c<$4; c++))
	do
		echo -ne " " ${!current} >> $stop_command
		current=`expr $current + 1`
	done
	
	chmod +x $stop_command
fi	

exit 1
