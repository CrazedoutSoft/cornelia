#!/bin/bash


if [[ -z "${CORNELIA_HOME}" ]]; then
  export CORNELIA_HOME=$(pwd)
fi

export LD_LIBRARY_PATH=$CORNELIA_HOME/openssl

if [[ $1 == "" ]];
then
$CORNELIA_HOME/bin/cornelia_d -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
fi

if [[ $1 == "http" ]];
then
$CORNELIA_HOME/bin/cornelia_d -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
fi

if [[ $1 == "ssl" ]];
then
$CORNELIA_HOME/bin/cornelia_d -ssl -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
fi

if [[ $1 == "tls" ]];
then
$CORNELIA_HOME/bin/cornelia_d -tls -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
fi




