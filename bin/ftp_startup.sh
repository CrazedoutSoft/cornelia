#!/bin/bash

clear
echo "************************************"
echo "* Starting Cornelia FTP Web Server *"
echo "* (c) CrazedoutSoft 2022 / F.Roos  *"
echo "************************************"


if [[ -z "${CORNELIA_HOME}" ]]; then
  export CORNELIA_HOME=$(pwd)
  echo env 'CORNELIA_HOME' missing. Setting CORNELIA_HOME to $(pwd)
fi

bin/ftp_cornelia $1 $2 $3 $4 &


