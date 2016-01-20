FROM      osrg/ryu
MAINTAINER iMinds

#RUN apt-get update && apt-get install -y \
#	iproute \
#	net-tools \
#	iputils-ping \
#	curl

RUN apt-get update
RUN apt-get install -y iproute net-tools iputils-ping curl

COPY ./ryu_app ./ryu_app

ADD start.sh start.sh
RUN chmod +x start.sh

CMD ./start.sh
