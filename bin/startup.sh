#!/bin/bash

LD_LIBRARY_PATH=$CORNELIA_HOME/openssl $CORNELIA_HOME/bin/cornelia_d -c $CORNELIA_HOME/conf/corny.conf & 2> $CORNELIA_HOME/log/error.log
LD_LIBRARY_PATH=$CORNELIA_HOME/openssl $CORNELIA_HOME/bin/cornelia_d -ssl -c $CORNELIA_HOME/conf/corny.conf & 2> $CORNELIA_HOME/log/error.log
LD_LIBRARY_PATH=$CORNELIA_HOME/openssl $CORNELIA_HOME/bin/cornelia_d -tls -c $CORNELIA_HOME/conf/corny.conf & 2> $CORNELIA_HOME/log/error.log




