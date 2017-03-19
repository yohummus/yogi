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

## Installation
TODO

### Installing on Debian 9 (Stretch)
1. sudo apt-get install cmake g++ libboost-all-dev googletest libprotobuf-dev protobuf-compiler libqt5core5a libqt5network5 libqt5websockets5-dev nodejs nodejs-legacy npm python3-protobuf python-protobuf python3-setuptools python-setuptools python3-future python-future python-typing python-enum34
2. sudo npm install -g npm
3. TBD...
