#ifndef KEYWORDS_H_
#define KEYWORDS_H_ 1

#include "../../utils/constants.h"

#define DOMAIN_INFORMATIONS                     "frog-domain:informations"
    #define NAME                                "name"
    #define TYPE                                "type"
    #define MANAGEMENT_ADDRESS                  "management-address"
    #define NETWORK_MANAGER_INFO                "frog-network-manager:informations"
    #define OPENCONFIG_IF_INTERFACES            "openconfig-interfaces:interfaces"
    #define OPENCONFIG_IF_INTERFACE             "openconfig-interfaces:interface"
        #define IF_TYPE                         "frog-interface-type"
        #define IF_CONFIG                       "config"
            #define UNNUMBERED                  "unnumbered"
            #define DESCRIPTION                 "description"
            #define ENABLED                     "enabled"
        #define IF_STATE                        "state"
            #define ADMIN_STATUS                "admin-status"
            #define OPER_STATUS                 "oper-status"
            #define STATUS_UP                   "UP"
            #define STATUS_DOWN                 "DOWN"
        #define IF_CAPABILITIES                 "capabilities"
            #define GRE                         "gre"
        #define OPENCONFIG_IF_SUBINTERFACES     "openconfig-interfaces:subinterfaces"
        #define OPENCONFIG_IF_SUBINTERFACE      "openconfig-interfaces:subinterface"
            #define IF_GRE                      "frog-if-gre:gre"
                #define OPTIONS                 "options"
                    #define OPT_LOCAL_IP        "local_ip"
                    #define OPT_REMOTE_IP       "remote_ip"
                    #define OPT_KEY             "key"
        #define OPENCONFIG_IF_ETHERNET          "openconfig-if-ethernet:ethernet"
            #define ETHERNET_CONFIG             "openconfig-if-ethernet:config"
                #define MAC_ADDRESS             "mac-address"
            #define OPENCONFIG_VLAN             "openconfig-vlan:vlan"
                #define VLAN_CONFIG             "openconfig-vlan:config"
                    #define IF_MODE             "interface-mode"
                    #define TRUNK_VLANS         "trunk-vlans"
                        #define TRUNK           "TRUNK"
                        #define ACCESS          "ACCESS"
            #define NEIGHBOR                    "frog-neighbor:neighbor"
                #define NEIGHBOR_NAME           "domain-name"
                #define NEIGHBOR_TYPE           "domain-type"
                #define NEIGHBOR_INTERFACE      "remote-interface"
#endif //KEYWORDS_H_