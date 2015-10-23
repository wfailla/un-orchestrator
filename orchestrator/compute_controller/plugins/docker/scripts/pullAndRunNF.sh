#!/bin/bash

#Author: Ivano Cerrato
#Date: June 16th 2014
#Brief: pull a NF from a docker repository, and run it. 

#command line: sudo ./pullAndRunNF.sh $1 $2 $3 $4 [$5 ...]

#$1 LSI ID				(e.g., 2)
#$2 NF name				(e.g., firewall)
#$3 registry/nf[:tag] 	(e.g., localhost:5000/pcap:latest)
#$4 number_of_ports		(e.g., 2)
#The next $4 parameters are port names to be provided to the container (e.g., vEth0 vEth1)
#The next $4 parameters are IPv4 addresses / network to be associated with that ports (e.g., 10.0.0.1/24)
#	0 if no IPv4 address must be associated
#The next $4 parameters are Eth addresses to be associated with that ports (e.g., aa:aa:aa:aa:aa:aa)
#	0 if no Ethernet address must be associated

tmp_file="$1_$2_tmp"

if (( $EUID != 0 )) 
then
    echo "[pullAndRunNF] This script must be executed with ROOT privileges"
    exit 0
fi

echo -ne "" >> downloaded

find=`cat downloaded | grep $3 | wc -l`

if [ $find -eq 0 ]
then 
#	IVANO: uncomment the following rows in case you want that the images are downloaded from a repositotory.
#		Otherwise, the code supposes that the VNFs images are available locally.

#	#The image must be downloaded from the ropository
#
#	sudo docker pull $3
#	#docker pull returns 0 in case of success
#	ret=`echo $?`
#
#	if [ $ret -eq 0 ]
#	then
#		echo "[pullAndRunNF] Image '"$3"' retrieved"
#	else
#		echo "[pullAndRunNF] Impossible to retrieve image '"$3"'"
#		exit 0
#	fi
	
	echo $3 >> downloaded
fi

echo -ne "sudo docker run -d --name $1_$2 "   > $tmp_file

echo --net=\"none\"  --privileged=true  $3 >> $tmp_file

echo [`date`]"[pullAndRunNF] Executing command: '"`cat $tmp_file`"'"

ID=`bash $tmp_file`

#docker run returns 0 in case of success
ret=`echo $?`

if [ $ret -eq 0 ]
then
	echo [`date`]"[pullAndRunNF] Container $2 started with ID: '"$ID"'"
else
	echo "[pullAndRunNF] An error occurred while starting the container"
	rm $tmp_file
	exit 0
fi

#Save the binding lsi-nf-docker id on a file
file="$1_$2"
echo $ID >> $file

rm $tmp_file

#The following code configures the network for the container just created.
#It is based on the description provided at
#	http://docs.docker.com/articles/networking/#container-networking

PID=`docker inspect --format '{{ .State.Pid }}' $ID`

sudo mkdir -p /var/run/netns
sudo ln -s /proc/$PID/ns/net /var/run/netns/$PID
	
current=5
currentIp=`expr $current + $4`
currentEthernet=`expr $currentIp + $4`

for (( c=0; c<$4; c++ ))
do	
	ip link set ${!current} netns $PID
	ip netns exec $PID ip link set dev ${!current} name eth$c
			
	if [ ${!currentEthernet} != 0 ]
 	then
 		ip netns exec $PID ip link set eth$c address ${!currentEthernet}
 	fi
 	
 	if [ ${!currentIp} != 0 ]
 	then
 		sudo ip netns exec $PID ip addr add ${!currentIp} dev eth$c
 	fi
 	
 	ip netns exec $PID ip link set eth$c up
	
	current=`expr $current + 1`
	currentIp=`expr $currentIp + 1`
	currentEthernet=`expr $currentEthernet + 1`
done

exit 1
