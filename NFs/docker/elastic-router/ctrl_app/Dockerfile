FROM      osrg/ryu
MAINTAINER iMinds & Acreo

#RUN apt-get update && apt-get install -y \
#	iproute \
#	net-tools \
#	iputils-ping \
#	curl

RUN apt-get update
RUN apt-get install -yq iproute net-tools iputils-ping curl

COPY ./ryu_app ./ryu_app

ADD start.sh start.sh
RUN chmod +x start.sh

RUN apt-get install -yq python3 python3-zmq python3-pip libffi-dev libsodium-dev
RUN pip3 install PyNaCl
RUN wget https://github.com/acreo/DoubleDecker/archive/master.tar.gz -O dd.tar.gz
RUN tar xzf dd.tar.gz && rm dd.tar.gz
WORKDIR /root/DoubleDecker-master/python/
RUN python3 setup.py build
RUN python3 setup.py install

CMD ./start.sh
