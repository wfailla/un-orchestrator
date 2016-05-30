#! /bin/sh

name=${1:-ramon}
interface=${2:-veth0}
ramon_port=${3:-55555}
config_port=${4:-54736}

RATEMON_CLIENT=./ratemon_client.py

xterm -T ${name}_dd -e "${RATEMON_CLIENT} -k /etc/doubledecker/a-keys.json ${name} a -p ${ramon_port} -q ${config_port} --ramon_args -i ${interface} -s 20 -e 1 -k 10 -m 1 -q ${config_port}; read" &
