#!/bin/bash

if [[ $1 == "" ]];
then
ps -ef | grep './cornelia_d' | grep -v grep | awk '{print $2}' | xargs -r kill -9
fi

if [[ $1 == "http" ]];
then
ps -ef | grep './cornelia_d -c' | grep -v grep | awk '{print $2}' | xargs -r kill -9
fi

if [[ $1 == "ssl" ]];
then
ps -ef | grep './cornelia_d -ssl' | grep -v grep | awk '{print $2}' | xargs -r kill -9
fi

if [[ $1 == "tls" ]];
then
ps -ef | grep './cornelia_d -tls' | grep -v grep | awk '{print $2}' | xargs -r kill -9
fi


