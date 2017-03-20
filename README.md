# YOGI

YOGI - A framework for software decoupling

## Introduction

YOGI is a framework for breaking complex software up into several smaller,
loosely coupled, single-purpose processes that are easier to develop, maintain
and test. These simpler processes can be written in different languages and
communicate with each other on a virtual network, using a variety of message
passing schemes such as publish-subscribe or RPC-like methods. This virtual
network, a.k.a. YOGI network, is the foundation of the YOGI framework.

**Example.** Industrial machines are often very complex systems that require a
lot of software distributed on various computers or embedded systems. The
software might include a substantial amount of legacy code that does not
integrate well with the rest of the design or impede the use of more suitable
languages for the problem. C++ might be a suitable choice for interfacing with
hardware in real-time while Python would be more convenient for test scripts
and Javascript might be desirable for implementing a user interface. The YOGI
framework enables splitting the different responsibilities of the software up
into separate processes, each using the most natural language for its task.
Legacy code can be wrapped up into its own process and thus isolated from the
rest of the design.

## Overview

This section provides an overview of the different core parts of the YOGI
framework, starting with a description of its virtual network along with the
design of processes using the framework. It then summarises the language and
operating system support before discussing the built-in development tools and
the design of graphical user interfaces. Finally, this section briefly explains
the additional ready-to-use processes that come with the YOGI framework.

### YOGI Network

The YOGI Network is the core of the YOGI framework. It is a virtual network
interconnecting a set of processes. These processes can communicate with each
other using different message passing schemes over different communication
channels. The available message passing schemes are:
- **Publish-Subscribe.** This scheme allows a sender to send a message to an
  arbitrary number of receivers.
- **Scatter-Gather.** Using this method, a sender can request information from
  an arbitrary number of clients. It is a request-response scheme that supports
  one or more responders. This method is basically RPC on steroids.
The currently available communication channels are TCP/IP and process-local
connections.


TODO: Leafs, Nodes, Terminals, Bindings, Hub, TCP, Messaging, Protobuf, ...

### Framework for Processes

TODO: ProcessInterface, TCP client, configuration, logging, ...

### Supported Languages and Operating Systems

TODO: Linux, Windows, ARM

### Development Tools

TODO: Hub, yogi-gui

### Designing User Interfaces

TODO: Hub, yogi-gui, JS, nodejs

### Ready-to-use Processes

TODO: Watcher

## Installation

This section describes how to setup and build YOGI for the purpose of working
on the framework itself. For developing applications using the framework, please
use one of the provided pre-built packages.


### Installation on Debian 9 (Stretch)

The install.sh file in the top level directory can be used to install the
required dependencies, build all sub-projects and install them on a Debian 9
(Stretch) system. The script may work on other Debian-based distributions but
this has not been tested. The install script first installs the required
Debian packages, updates npm (NodeJS' package manager) to a more recent version
and then builds and installs all sub-projects.

## Examples

TODO

