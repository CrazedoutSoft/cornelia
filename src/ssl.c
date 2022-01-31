/*
Copyright (c) 2022 CrazedoutSoft / Fredrik Roos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../include/misc.h"
#include "../include/webs.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>

#define SA struct sockaddr;

SSL_CTX *sslctx;
SSL *cSSL;

void InitializeSSL();
void DestroySSL();
void ShutdownSSL();
int passwd_error=0;

char* read_ssl_passwd(char* passwd, int size){

	FILE* fd;
	char* path = (char*)malloc(1024);
	sprintf(path,"%s/cert/.passwd", getenv("CORNELIA_HOME"));
	if((fd=fopen(path,"r"))!=NULL){
	 if(fgets(passwd, size, fd)==NULL) passwd_error=1;
	 fclose(fd);
	}else{
	  passwd_error=1;
	  printf("Can't read ssl passphrase from cert/.passwd\n");
	}
	clip(passwd);

 return passwd;
}

int create_socket(int port) {

    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket: ssl");
	return -1;
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind: ssl");
	return -1;
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen: ssl");
	return -1;
    }

    return s;
}

int ssl_read(void* cSSL, char* buffer, int len){

	    return SSL_read((SSL*)cSSL, (char*)buffer, len);

}

int ssl_write(void* cSSL, const char* buffer, int len){

	  return SSL_write(cSSL, buffer, len);

}

int my_pem_password_cb_fun(char *buf, int size, int rwflag, void *u){

	strncpy(buf, (char *)u, size);
	buf[size - 1] = '\0';
	(void)(rwflag);
	return strlen(buf);

}

void init_ssl_server(const server_conf* serv_conf){

	struct sockaddr_in cli_addr,cli;
	struct sockaddr_in* pV4Addr;
	unsigned int clilen;
	int sockfd, newsockfd=0;
        char  cip[16];
	char passwd[64];

	InitializeSSL();

	sslctx = SSL_CTX_new(SSLv23_server_method());
	SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);

	read_ssl_passwd(&passwd[0],64);
	SSL_CTX_set_default_passwd_cb_userdata (sslctx, (void *)&passwd[0]);
      	SSL_CTX_set_default_passwd_cb (sslctx, my_pem_password_cb_fun);

	int use_cert = SSL_CTX_use_certificate_file(sslctx, &serv_conf->cert[0], SSL_FILETYPE_PEM);
	int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, &serv_conf->cert_key[0], SSL_FILETYPE_PEM);

	(void)(use_cert);
	(void)(use_prv);

	sockfd=create_socket(serv_conf->ssl_port);
	printf("Cornelia listening on %d [HTTPS/SSL]\n", serv_conf->ssl_port);
	int loop=1;

	while(loop){

	  if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))<0){
	    perror("Accept error: sewsock ssl\n");
	    return;
	  }

	  cSSL = SSL_new(sslctx);
	  SSL_set_fd(cSSL, newsockfd);
	  int ssl_err = SSL_accept(cSSL);

	  if(ssl_err <= 0){
	    //perror("Accept error SSL_accept() ssl\n");
	  }else{
	    int pid = fork();
            if(pid>0){
              memset(&cip[0],0,16);
	      pV4Addr = (struct sockaddr_in*)&cli;
              inet_ntop(AF_INET, &pV4Addr->sin_addr, &cip[0],INET_ADDRSTRLEN );

	      struct timeval tv;
              tv.tv_sec = 0;
	      tv.tv_usec = serv_conf->keep_alive_timeout;
              setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

              handle_request(newsockfd,&cip[0],cSSL);
              shutdown(newsockfd,SHUT_RDWR);
              loop=0;
            }
	  }
	}

}

void InitializeSSL(){

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
}

void DestroySSL(){

    	ERR_free_strings();
    	EVP_cleanup();
}

void ShutdownSSL(){

    	SSL_shutdown(cSSL);
    	SSL_free(cSSL);
}

