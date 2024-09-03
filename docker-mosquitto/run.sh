#!/bin/bash
#Steps to install:
# - run "docker run -it -p 1883:1883 -p 9001:9001 eclipse-mosquitto" to get a default config
# - copy config file from docker: "docker cp <instance name>:/mosquitto/config/mosquitto.conf mosquitto.conf
# - modify things in mosquitto.conf
# - run the following command which uses the config we modified + volumes to persist data

docker run -it --rm -p 1883:1883 -p 9001:9001 -v ./mosquitto.conf:/mosquitto/config/mosquitto.conf -v /mosquitto/data -v /mosquitto/log eclipse-mosquitto
