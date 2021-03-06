# gcc 9.4.0
CC=gcc


all:
	$(CC) -o bin/cornelia_d -std=c99 -Wall -Wextra -O3 -Iopenssl/include/ -Lopenssl/ src/conf.c src/base64.c src/misc.c src/webs.c src/ssl.c src/tls.c -lcrypt -lssl -lcrypto
	$(CC) -o bin/mkpasswd -Wall -Wextra -O3 src/mkpasswd.c -lcrypt
	$(CC) -o bin/ftp_cornelia -std=c99 -Wall -Wextra -O3 src/misc.c src/ftp.c -lcrypt
	$(CC) -o bin/findip src/findlip.c
	$(CC) -o bin/jspz -Wall -O3 src/jspz.c


clean:
	-rm bin/jspz
	-rm bin/findip
	-rm bin/ftp_cornelia
	-rm bin/cornelia_d
	-rm log/*.log
	-rm -rf JSPZWD 



