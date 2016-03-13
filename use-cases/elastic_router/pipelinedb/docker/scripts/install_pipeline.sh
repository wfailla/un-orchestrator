#!/bin/bash

apt-get update
apt-get -f install

# install some basics
apt-get install -y wget nano libxml2 libxml2-dev libreadline6 libreadline6-dev check g++ flex bison zlib1g-dev libpq-dev libcurl4-openssl-dev libpython-dev libncurses5-dev
wget -q https://github.com/pipelinedb/pipelinedb/archive/0.8.6.tar.gz
#wget -q http://www.pipelinedb.com/download/0.8.5/ubuntu14
#dpkg --install ubuntu14
tar xvzf 0.8.6.tar.gz
cd pipelinedb-0.8.6
./configure --with-python
make install
useradd -m pipeline -s "/bin/bash"
