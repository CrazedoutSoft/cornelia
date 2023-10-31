ps -ef | grep 'ftp_cornelia' | grep -v grep | awk '{print $2}' | xargs -r kill -9
