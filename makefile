CC=gcc


all:
	$(CC) -o bin/cornelia_d -std=c99 -Wall -O3 -Iopenssl/include/ -Lopenssl/ src/conf.c src/base64.c src/misc.c src/webs.c src/ssl.c src/tls.c -lssl -lcrypto
	$(CC) -o bin/ftp_cornelia -std=c99 -Wall -O3 src/misc.c src/ftp.c

ftp:


clean:
	rm bin/ftp_cornelia
	rm bin/cornelia_d
	rm log/*.log





