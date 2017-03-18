# yogi
YOGI - A framework for software decoupling

## Introduction
YOGI is a framework for breaking complex software up into several smaller,
loosely coupled, single-purpose processes that are easier to develop, maintain
and test. These simpler processes communicate with each other on a virtual
network, using a variety of message passing schemes such as publish-subscribe
or RPC-like methods. This virtual network, a.k.a. YOGI network, is the
foundation of the YOGI framework.

## Installing on Debian 9 (Stretch)
1. sudo apt install cmake g++ libboost-all-dev googletest libprotobuf-dev protobuf-compiler libqt5core5a libqt5network5 libqt5websockets5-dev nodejs nodejs-legacy npm python3-protobuf python3-setuptools python3-future
2. sudo npm install -g npm
3. TBD...
