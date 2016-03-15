FROM ubuntu:16.04

MAINTAINER Bertrand Pechenot <berpec@acreo.se>
# based on petergrace/opentsdb 
# Thanks to Peter Grace (original author)
# Thanks to Pontus Sköldström and Tamas Levai

#Install packages
RUN DEBIAN_FRONTEND=noninteractive apt-get -y update \
&& DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends \
                                                  -y \
                                                     curl \
                                                     make \
                                                     autoconf \
                                                     git-core \
                                                     openjdk-6-jdk \
                                                     supervisor \
                                                     openssh-server \
                                                     automake \
                                                     gnuplot-nox \
                                                     unzip \
                                                     python3 \
                                                     python3-zmq \
                                                     python3-requests \
						     python3-setuptools \ 
                                                     python3-pip \
                                                     libffi-dev \
                                                     libsodium-dev \
                                                     python3-dev \
                                                     build-essential \
&&  DEBIAN_FRONTEND=noninteractive apt-get clean all

#Install HBase and scripts
RUN mkdir -p /opt/sei-bin/ 
RUN mkdir -p /data/hbase
RUN mkdir -p /root/.profile.d

ADD http://apache.org/dist/hbase/hbase-0.94.27/hbase-0.94.27.tar.gz /opt/downloads/
RUN tar xzvf /opt/downloads/hbase-*gz 
RUN rm /opt/downloads/hbase-*gz 
RUN ["/bin/bash", "-c", "mv hbase-* /opt/hbase"]

ADD start_hbase.sh /opt/sei-bin/
ADD hbase-site.xml /opt/hbase/conf/
EXPOSE 4242 5555 60000 60010 60030

#Install OpenTSDB and scripts
RUN git clone --depth 1 -b next --single-branch git://github.com/OpenTSDB/opentsdb.git /opt/opentsdb 
WORKDIR /opt/opentsdb 
RUN ./build.sh

ADD start_opentsdb.sh create_tsdb_tables.sh /opt/sei-bin/

#Install Supervisord
RUN mkdir -p /var/log/supervisor
ADD supervisor-hbase.conf supervisor-serf.conf supervisor-system.conf supervisor-tsdb.conf supervisor-dd.conf /etc/supervisor/conf.d/

#Configure SSHD properly
#ADD supervisor-sshd.conf /etc/supervisor/conf.d/
#RUN mkdir -p /root/.ssh
#RUN chmod 0600 /root/.ssh
#RUN sed -ri 's/UsePAM yes/#UsePAM yes/g; s/#UsePAM no/UsePAM no/g;' /etc/ssh/sshd_config
#RUN mkdir -p /var/run/sshd
#RUN chown 0:0 /var/run/sshd
#RUN chmod 0744 /var/run/sshd
#ADD create_ssh_key.sh /opt/sei-bin/

#Install serf and scripts
# ADD https://dl.bintray.com/mitchellh/serf/0.6.4_linux_amd64.zip /tmp/
ADD https://dl.bintray.com/mitchellh/serf/0.1.0-dev.1382472282_linux_amd64.zip /tmp/
RUN mkdir /tmp/serf_src/
WORKDIR /tmp/serf_src/
RUN unzip /tmp/0.1.0-dev.1382472282_linux_amd64.zip
RUN mv /tmp/serf_src/serf /usr/bin
RUN rm -rf /tmp/0.1.0-dev.1382472282_linux_amd64.zip /tmp/serf_src/

ADD serf-start.sh serf-join.sh /opt/sei-bin/

# Install DoubleDecker proxy
# ADD ./doubledecker /opt/sei-bin/doubledecker
ADD start_ddproxy.sh ddproxy.py /opt/sei-bin/
WORKDIR /root

RUN pip3 install PyNaCl
ADD https://github.com/acreo/DoubleDecker/archive/master.tar.gz /root/
RUN tar xzf master.tar.gz
RUN rm master.tar.gz
WORKDIR /root/DoubleDecker-master/python/
RUN python3 setup.py build
RUN python3 setup.py install

ENV CLIENT_NAME opentsdb
ENV DEALER_PORT tcp://172.17.0.1:5555
ENV CUSTOMER_NAME a
RUN chmod +x /opt/sei-bin/*
# default run command
CMD /usr/bin/supervisord
