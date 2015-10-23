#!/bin/bash

#Author: Sergio Nuccio
#Date: October 21st 2015
#Brief: stop the execution of a native NF 

#command line: sudo ./nfs_manager/scripts/native/stopNativeNF.sh $1 $2

#$1 LSI ID							(e.g., 2)
#$2 NF name							(e.g., firewall)
#$3 number_of_ports						(e.g., 2)
#The next $3 parameters are the names of the port of the NF	(e.g., vEth0 vEth1)

if (( $EUID != 0 )) 
then
    echo "[stopNativeNF] This script must be executed with ROOT privileges"
    exit 0
fi

#open file that eventually specifies other actions to do in order to stop the NF and clean the system (if it exists)
file="$1_$2_stop"

if [ -e $file ]
then
	tmp_file="$1_$2_tmp"
	echo "" > $tmp_file
	echo -ne sudo ./$file $1 $2 $3 > $tmp_file
	port=4
	for((c=0; c<$3; c++))
	do
		echo -ne " " ${!port} >> $tmp_file
		port=`expr $port + 1`
	done
	
	sudo chmod +x $file
	
	echo [`date`]"[stopNativeNF] Executing command: '"`cat $tmp_file`"'"
	
	#command:= <script_name> <LSI_ID> <NF_NAME> <#PORTS> <PORT_NAMES ...>
	bash $tmp_file
	
	# remove files after the execution
	#rm $file
	rm $tmp_file
fi

#delete virtual interfaces related to the NF
current=4

for((c=0; c<$3; c++))
do
	ip link del ${!current}
	current=`expr $current + 1`
done

echo "[stopNativeNF] Native NF stopped"

exit 1;
