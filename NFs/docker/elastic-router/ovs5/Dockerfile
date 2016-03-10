FROM      ubuntu
MAINTAINER iMinds

RUN apt-get update
RUN apt-get install -y openvswitch-switch openvswitch-common openssh-server

RUN echo 'root:root' | chpasswd
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config

ADD authorized_keys /root/.ssh/authorized_keys

ADD start.sh start.sh
RUN chmod +x start.sh

CMD ./start.sh
