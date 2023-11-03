# gcc 9.4.0
CC=gcc

all:	webs misc

nossl:	no_ssl misc

no-ssl-no-warn: no_ssl_no_warn misc

webs:   
	$(CC) -o bin/cornelia_d -std=c99 -Wall -Wextra -O3 -Iopenssl/include/ -Lopenssl/ src/conf.c src/base64.c src/misc.c src/webs.c src/ssl.c src/tls.c -lcrypt -lssl -lcrypto

misc:
	$(CC) -o bin/mkpasswd -Wall -Wextra -O3 src/mkpasswd.c -lcrypt
	$(CC) -o bin/ftp_cornelia -std=c99 -Wall -Wextra -O3 -Iopenssl/include/ -Lopenssl/ src/ftps.c src/misc.c src/ftp.c -lcrypt -lssl -lcrypto
	$(CC) -o bin/findip src/findlip.c
	$(CC) -o bin/jspz -std=c99 -Wall -O3 src/jspz.c

no_ssl:	
	$(CC) -o bin/cornelia_d -std=c99 -DNO_SSL=TRUE -Wall -Wextra -O3 src/conf.c src/base64.c src/misc.c src/webs.c -lcrypt
	$(CC) -o bin/ftp_cornelia -std=c99 -Wall -DNO_SSL=TRUE -Wextra -O3 src/misc.c src/ftp.c -lcrypt

no_ssl_no_warn:
	$(CC) -o bin/cornelia_d -std=c99 -DNO_SSL=TRUE src/conf.c src/base64.c src/misc.c src/webs.c -lcrypt
	$(CC) -o bin/ftp_cornelia -std=c99 -DNO_SSL=TRUE src/misc.c src/ftp.c -lcrypt

clean:
	-rm bin/jspz
	-rm bin/findip
	-rm bin/ftp_cornelia
	-rm bin/cornelia_d
	-rm log/*.log
	-rm -rf JSPZWD 


