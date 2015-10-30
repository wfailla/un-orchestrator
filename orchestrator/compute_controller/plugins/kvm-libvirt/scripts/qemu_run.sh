#!/bin/bash

#Author: Ivano Cerrato
#Date: Oct 30th 2015

#$1 network function name
#$2 disk path
#$3 final part of the command line related to ivshmem

if (( $EUID != 0 ))
then
    echo "[$0] This script must be executed with ROOT privileges"
    exit 0
fi

#FIXE: smp and memory should be provided externally

echo "+++++++++"
echo $3
echo "+++++++++"

echo "[$0] Executing command 'sudo qemu-system-x86_64 -name $1 -cpu host -smp 1 -machine accel=kvm,usb=off -m 1024 -drive file=$2 $3 -snapshot'"

sudo `echo qemu-system-x86_64 -name $1 -cpu host -smp 1 -machine accel=kvm,usb=off -m 1024 -drive file=$2 -snapshot -daemonize`

exit 1
