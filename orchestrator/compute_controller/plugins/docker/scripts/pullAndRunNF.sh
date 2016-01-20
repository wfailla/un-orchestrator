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

tmp_file="$1_$2_tmp"

if (( $EUID != 0 ))
then
    echo "[$0] This script must be executed with ROOT privileges"
    exit 0
fi

echo -ne "sudo docker run -d --name $1_$2 -e NAME=$2 "   > $tmp_file

#check if NF name contains ctrl, then it needs Cf-Or interface (leave out --net=none option to create interface to docker0 on localhost)
#TODO: cf-or connection should be defined in nffg
if [[ $2 == *"ovs"* ]] || [[ $2 == *"ctrl"* ]]
then
	echo  --privileged=true  $3 >> $tmp_file
else
	echo --net=\"none\"  --privileged=true  $3 >> $tmp_file
fi


echo [`date`]"[$0] Executing command: '"`cat $tmp_file`"'"

ID=`bash $tmp_file`

#docker run returns 0 in case of success
ret=`echo $?`

if [ $ret -eq 0 ]
then
	echo [`date`]"[$0] Container $2 started with ID: '"$ID"'"
else
	echo "[$0] An error occurred while starting the container"
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

# rename cf_or interface
#if [[ $2 == *"ctrl"* ]]
#then
#	ip netns exec $PID ip link set eth0 down
#	ip netns exec $PID ip link set dev eth0 name cf_or
#	ip netns exec $PID ip link set cf_or up
#fi

for (( c=0; c<$4; c++ ))
do	
	ip link set ${!current} netns $PID
	
	# elastic router use case specific hack:
	# if deploying ovs type of Dockers, name the interfaces differently so ctrl_app can detect them 
	# if deploying ctrl app, eth0 already exists from cfor interface
	# this hack could be solved if interface names are directly imported from the nffg (together with ip addresses)
	if [[ $2 == *"ovs"* ]] || [[ $2 == *"ctrl"* ]]
	then
		eth_name="${2}_eth$c"
	else
		eth_name="eth$c"
	fi
	echo "[$0] Inserting port ${!current} inside a container. It will have name $eth_name"
	
	ip netns exec $PID ip link set dev ${!current} name $eth_name
				
 	ip netns exec $PID ip link set $eth_name up
	
	current=`expr $current + 1`
done

exit 1
