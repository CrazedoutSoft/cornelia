******************************************************
*  Welcome to Cornelia Web Sever                     *
*  CrazedoutSoft 2022 by Fredrik Roos.               *
*  http://crazedout.com/corny/ info@crazedout.com    *
*  https://github.com/openssl                        *
******************************************************

set env CORNELIA_HOME to where cornelia_d is located.
(export CORNELIA_HOME=<path to cornelia>)

To use TLS/SSL you may have to copy: 
 openssl/libcrypto.so, openssl/libcrypto.so.3, openssl/libssl.so and openssl/libssl.so.3 
to /usr/lib

config file = conf/corny.conf

startup=startup.sh or
ssl_corelia_d -c conf/corny.conf &

shutdown=bin/shutdown.sh or
ps -ef | grep './cornelia_d' | grep -v grep | awk '{print $2}' | xargs -r kill -9
ps -ef | grep './ssl_cornelia_d' | grep -v grep | awk '{print $2}' | xargs -r kill -9

OpenSSL:
If you can't build Cornelia with the OpenSSL headers and libs provided you 
must compile your own OpenSLL libs and replace them in dir openssl
https://github.com/openssl

jGazm:
 1) Run ./setup_linux
 2) JAVA_HOME must be set to current JVM 1.7 (or above) in file jgazm.
