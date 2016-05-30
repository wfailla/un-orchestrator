#! /bin/sh

for n in {1..4};
do
    interface=veth$(($n*2-2))
    ramon_port=$((55555+$n))
    config_port=$((54736+$n))
    ./ramon_dd.sh ramon${n} ${interface} ${ramon_port} ${config_port}
done
