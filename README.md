# Universal Node Repository Summary - Elastic Router Branch
This repository contains the current implementation of the Universal Node and is divided in four sub-portions.
It contains all related files for the Unify Elastic Router Integrated Demo.
Please check individual README's in each sub-package.

## NFs
This folder contains some examples of virtual network functions.
The `docker/elastic router` folder contains the source code of the Elastic Router VNFs.
Thses VNFs should be supported by the name-resolver.

## Use-Cases
This folder contains additional files to deploy different use-cases.
The `elastic-router` folder has all additional sources __other__ than the VNFs. 
Meaning containers or other files which are not being deployed by from the NFFG by the UN orchestrator.
It contains all containers, configuration files that are needed to deploy the elastic router 
use case and supporting functions for the troubleshooting and monitoring.

## Universal Node orchestrator
The Universal Node orchestrator (un-orchestrator) is a module
that, given a Network Function - Forwarding Graph (NF-FG), deploys it on
the current physical server. To this purpose, it interacts with a virtual
switch in order to configure the paths among the NFs, and with an hypervisor
in order to properly start the required NFs.

## Name Resolver
The Name Resolver is a module that returns a set of implementations for a
given NF. It is exploited by the un-orchestrator each time that a NF must
be started in order to translate the 'abstract' name into the proper
suitable software image.

## Virtualizer
The Virtualizer is a module that enables the un-orchestrator to interact with the upper layers of the Unify architecture, by means of the NF-FG defined in WP3. It in fact converts the NF-FG defined by WP3 in the representation accepted by the un-orchestrator.

