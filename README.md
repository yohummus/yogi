# YOGI

YOGI - A framework for software decoupling

## Introduction

YOGI is a framework for breaking complex software up into several smaller,
loosely coupled, single-purpose processes that are easier to develop, maintain
and test. These simpler processes can be written in different languages and
communicate with each other on a virtual network, using a variety of message
passing schemes such as publish-subscribe or RPC-like methods. This virtual
network, a.k.a. _YOGI Network_, is the foundation of the YOGI framework.

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

#### Leafs and Nodes
Logically, a YOGI Network consists of two different types of entities: _Leafs_
and _Nodes_. A Leaf can only connect to a single other entity (either another
Leaf or a Node) and supports so-called _Terminals_ which represent communication
endpoints for a process. Nodes, on the other hand, support an arbitrary number
of connections to either type of entity and know every available Terminal in the
network. Nodes are responsible for routing messages between Terminals and should
be used sparsely and strategically in order to reduce communication overhead.
A single, central Node (see _YOGI-Hub_ below) is often sufficient in a YOGI
Network. However, it is worth considering using additional Nodes if Leafs
running on the same system communicate with each other.

**Example.** A Raspberry Pi runs a number of processes, each connecting via
TCP/IP to a central server running a Node. The processes need to exchange a lot
of data which is currently flowing over ethernet to the server, then back to the
Raspberry Pi and finally into the target process. This potentially unnecessary
load on the physical network can be reduced by introducing a new Node on the
Raspberry Pi. This new Node would then connect to the central server and all
other processes connect to the new Node instead. By doing so, messages which
only flow between two processes on the Raspberry Pi are kept within the system
and do not propagate onto the physical network.

#### Terminals and Bindings
_Terminals_ are communication endpoints used in user processes. Processes
typically create many Terminals, some for control data, some for process
data and some for debugging or logging purposes.

Terminals have three main properties: A _name_, a _signature_ and a _type_. The
name of a Terminal is a path-like string used to identify the Terminal within
the network (e.g. _/Left Motor/Voltage_). Signatures identify the structure of
the messages that are transmitted over the Terminal. There are pre-defined
(a.k.a. _official_) signatures for messages representing common data types like
integers or a list of strings. If none of the official signatures are suitable,
custom messages with a user-defined signature can be created. Lastly, Terminals
have a certain type denoting how they exchange messages. The different types
are:
- **Deaf-Mute Terminals.** Those Terminals do not exchange data at all. They are
  useful when a process only needs to detect the presence of a Terminal.
- TODO
- TODO


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

