#!/bin/bash

if [[ -z "${CORNELIA_HOME}" ]]; then
  export CORNELIA_HOME=$(pwd)
  echo env 'CORNELIA_HOME' missing. Setting CORNELIA_HOME to $(pwd)
fi

FILE=$CORNELIA_HOME/bin/cornelia_d
if test -f "$FILE"; then
    echo ""
else
    echo "$FILE does not exist. Run 'build_all' or 'make'"
    exit
fi

if [[ $1 == "--help" ]];
then
$CORNELIA_HOME/bin/cornelia_d --help
exit
fi


export LD_LIBRARY_PATH=$CORNELIA_HOME/openssl

clear
echo "************************************"
echo "* Starting Cornelia Web Server     *"
echo "* (c) CrazedoutSoft 2022 / F.Roos  *"
echo "************************************"


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

if [[ $1 == "all" ]];
then
$CORNELIA_HOME/bin/cornelia_d -http -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
$CORNELIA_HOME/bin/cornelia_d -ssl -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
$CORNELIA_HOME/bin/cornelia_d -tls -c $CORNELIA_HOME/conf/corny.conf 2> $CORNELIA_HOME/log/error.log &
fi




