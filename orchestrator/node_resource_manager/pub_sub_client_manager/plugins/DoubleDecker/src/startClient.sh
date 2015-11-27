#!/bin/bash
./node_resource_manager/pub_sub_client_manager/plugins/DoubleDecker/src/ddclient -c $1 -k node_resource_manager/pub_sub_client_manager/plugins/DoubleDecker/src/$2 -n $3\
        -d $4\
         &
