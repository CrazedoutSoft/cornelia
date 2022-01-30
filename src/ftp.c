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
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>

#define SOCKET unsigned int

#define USER "USER"
#define PASS "PASS"
#define STAT "STAT"
#define NOOP "NOOP"
#define QUIT "QUIT"
#define SYST "SYST"
#define PORT "PORT"
#define LIST "LIST"
#define PWD  "PWD"
#define CWD  "CWD"
#define RETR "RETR"
#define TYPE "TYPE"
#define PASV "PASV"
#define STOR "STOR"

#define BINARY 'I'
#define ASCII  'A'

#define MODE_PASV	0
#define MODE_ACTIVE  	1

#define LOBYTE(w)       ((BYTE)(w))
#define HIBYTE(w)       ((BYTE)(((WORD)(w)>>8)&0xFF))

#define WORD	unsigned int
#define MAX_ALLOC	65536

int setenv(const char *name, const char *value, int overwrite);

typedef struct clientAddr_ {

	char ip[20];
	unsigned int port;
	unsigned int sockfd;

} clientAddr;


typedef struct _ftp_session_ {

	int mode;
 	clientAddr cAddr;
	char type;
	char clientIP[20];
	char workdir[256];
	struct sockaddr_in addr;
	char pasv_ip[20];
	unsigned int pasv_port;

} ftp_session;


int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
    }
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
    }
    if (listen(s, 1) < 0) {
        perror("Unable to listen");
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
	perror("setsockopt(SO_REUSEADDR) failed");
    }

    return s;
}

int sock_write(unsigned int sockfd, char* buffer, int len){
	return write(sockfd, buffer, len);
}

int sock_read(unsigned int sockfd, char* buffer, int len){
	return read(sockfd, buffer, len);
}

char* ftp_resolve_pasv(struct sockaddr_in* sockAddr, char* ipaddr);

int pass(SOCKET sockfd, const char* value);
int user(SOCKET sockfd, const char* value);
int quit(SOCKET sockfd, const char* value);
int noop(SOCKET sockfd, const char* value);
int syst(SOCKET sockfd, const char* value);
int list(SOCKET sockfd, ftp_session* session);
int port(SOCKET sockfd, char* value, ftp_session* session);
int pwd(SOCKET sockfd, ftp_session* session);
int cwd(SOCKET sockfd, const char* value);
int retr(SOCKET sockfd, const char* value, ftp_session* session);
int type(SOCKET sockfd, const char* value, ftp_session* session);
int pasv(SOCKET sockfd, ftp_session* session);
int stor(SOCKET sockfd, const char* value, ftp_session* session);

int open_server_sock(ftp_session* session){

	unsigned int servsock;
	struct sockaddr_in cli_addr;
	unsigned int clilen;

	session->cAddr.sockfd=0;
	servsock=create_socket(session->pasv_port);
         if((session->cAddr.sockfd = accept(servsock, (struct sockaddr *) &cli_addr, &clilen))<0){
            perror("Accept error: pasv\n");
            return -1;
          }

 return 0;
}

int open_socket_by_ip(const char* ip, int port){

        int sockfd;
        struct sockaddr_in servaddr;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
         return -1;
        }
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(ip);
        servaddr.sin_port = htons(port);

        if(connect(sockfd, (struct sockaddr*)&servaddr,sizeof(servaddr))>-1){
          return sockfd;
        }else{
          return -1;
        }
}

int ftp_stor(SOCKET sockfd, const char* value, ftp_session* session){

        int r=0;
        char* buffer = (char*)malloc(1024);
        FILE* fd;
        SOCKET s;

        if(session->mode==MODE_ACTIVE){
          s = open_socket_by_ip(&session->cAddr.ip[0], session->cAddr.port);
        }else{
          s=session->cAddr.sockfd;
        }
        if(s<0){
         printf("Socket fail %s %d\n", &session->cAddr.ip[0], session->cAddr.port);
         free(buffer);
         return -1;
        }

	fd=fopen(value,"w");
	if(fd==NULL){
	 printf("Bad file: %s\n", value);
	 return 0;
	}
	while((r=sock_read(s,buffer,1023))>0){
	  buffer[r]='\0';
	  fwrite(buffer,1,r,fd);
	}
	fclose(fd);

        strcpy(buffer,"226 Closing data connection.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer));
        shutdown(s,SHUT_RDWR);
        shutdown(session->cAddr.sockfd,SHUT_RDWR);

	free(buffer);

 return 1;
}

int ftp_retr(SOCKET sockfd, const char* value, ftp_session* session){

        int r=0;
        char* buffer = (char*)malloc(1024);
	char* mem;
	FILE* fd;
	SOCKET s;

	if(session->mode==MODE_ACTIVE){
          s = open_socket_by_ip(&session->cAddr.ip[0], session->cAddr.port);
	}else{
	  s=session->cAddr.sockfd;
	}
        if(s<0){
         printf("Socket fail %s %d\n", &session->cAddr.ip[0], session->cAddr.port);
         free(buffer);
         return -1;
        }

	fd=fopen(value,session->type==ASCII?"r":"rb");
	if(fd==NULL){
	  strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer));
	}else{
	  mem=(char*)malloc(1024);
	  while((r=fread(mem,1,1024,fd))>0){
	  	sock_write(s,mem,r);
	  }
	  fclose(fd);
	  free(mem);
	  strcpy(buffer,"226 Closing data connection.\r\n");
	  r=sock_write(sockfd, buffer, strlen(buffer));
	  shutdown(s,SHUT_RDWR);
	  shutdown(session->cAddr.sockfd,SHUT_RDWR);
	}
        free(buffer);
return r;
}

void send_err(SOCKET sockfd){

	char* buffer = (char*)malloc(1024);
	strcpy(buffer,"450 Couldn't open the file or directory\r\n");
	sock_write(sockfd,buffer,strlen(buffer));
	shutdown(sockfd, SHUT_RDWR);
	free(buffer);

}

int ftp_list(SOCKET sockfd, ftp_session* session){

	int n = 0;
	int r=0;
	char* buffer = (char*)malloc(MAX_ALLOC);
        char* argv[5];
	SOCKET s;

	if(session->mode==MODE_ACTIVE){
	  s = open_socket_by_ip(&session->cAddr.ip[0], session->cAddr.port);
	}else{
	  s=session->cAddr.sockfd;
	}
	if(s<0){
	 printf("Socket fail %s %d\n", &session->cAddr.ip[0], session->cAddr.port);
	 free(buffer);
	 return -1;
	}

        int pipefd[2];
        if(pipe(pipefd)==-1){
	  send_err(sockfd);
	  free(buffer);
	  return 0;
	}

        argv[0]=(char*)malloc(256);
        strcpy(argv[0],"ls");
        argv[1]=(char*)malloc(16);
        strcpy(argv[1],"-al");
        argv[2]=(char*)malloc(32);
        strcpy(argv[2],"--group-directories-first");
	argv[3]=NULL;

	int pid=fork();
	if(pid==0){
          close(pipefd[0]);
          dup2(pipefd[1], 1);
          dup2(pipefd[1], 2);
          close(pipefd[1]);
	  if(execve("/bin/ls", argv, NULL)==-1){
	    send_err(sockfd);
	  }
	}else if(pid==-1){
	  send_err(sockfd);
	  perror("bad fork() list dir\n");
	}else{
	  close(pipefd[1]);
	  char* ptr;
	  while((r=read(pipefd[0], buffer, MAX_ALLOC))>0){
	    ptr=strstr(buffer, "\n")+1;
	    r=sock_write(s,ptr,strlen(ptr));
	  }
	  strcpy(buffer,"226 Closing data connection.\r\n");
	  r=sock_write(sockfd, buffer, strlen(buffer));
	  shutdown(s,SHUT_RDWR);
	}

	while(1){
	  if(argv[n]==NULL) break;
	  free(argv[n++]);
	}

	free(buffer);

return r;
}

int parse_request(SOCKET sockfd, char* buffer, ftp_session* session){

	char *ptr;
	char verb[12];
	char *value = NULL;
	int r = 0;

	memset(&verb[0],0,strlen(&verb[0]));
	ptr = strtok(buffer," ");
	if(ptr!=NULL){
	  if(strlen(ptr)>4){
	    strcpy(&verb[0],clip(ptr));
	  }else{
	    strcpy(&verb[0],ptr);
	  }
	  ptr=strtok(NULL," ");
	  if(ptr!=NULL){
	   value=(char*)malloc(strlen(ptr));
	   memset(value,0, strlen(ptr));
	   strcpy(value, clip(ptr));
	   value=clip(value);
	  }
	}

	if(strcmp(&verb[0],USER)==0) r=user(sockfd, value);
	else if(strcmp(&verb[0],PASS)==0) r=pass(sockfd, value);
	else if(strcmp(&verb[0],QUIT)==0) r=quit(sockfd, value);
	else if(strcmp(&verb[0],SYST)==0) r=syst(sockfd, value);
	else if(strcmp(&verb[0],PORT)==0) r=port(sockfd, value, session);
	else if(strcmp(&verb[0],LIST)==0) r=list(sockfd, session);
	else if(strcmp(&verb[0],RETR)==0) r=retr(sockfd, value, session);
	else if(strcmp(&verb[0],STOR)==0) r=stor(sockfd, value, session);
	else if(strcmp(&verb[0],PWD)==0) r=pwd(sockfd,session);
	else if(strcmp(&verb[0],CWD)==0) r=cwd(sockfd, value);
	else if(strcmp(&verb[0],TYPE)==0) r=type(sockfd,value,session);
	else if(strcmp(&verb[0],PASV)==0) r=pasv(sockfd,session);
	else{
	  strcpy(buffer,"502 Command not implemented.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer));
	}

	free(value);

 return r;
}

char* toHiLow(int num, char* buffer){

	double n = (double)num;
	double nn = n / 256.0;
	int hi = (int)nn;
	int low = ((int)n)-(hi*256);
	sprintf(buffer,"%d,%d", hi,low);

 return buffer;
}

int pasv(SOCKET sockfd, ftp_session* session){

	int r;
	char* buffer = (char*)malloc(1024);
	char hilow[8];
	session->pasv_port++;
	//TODO: Need to fix pasv_port range.
	sprintf(buffer,"227 Entering Passive Mode (%s,%s)\r\n", &session->pasv_ip[0], toHiLow(session->pasv_port,&hilow[0]));
	r=sock_write(sockfd, buffer, strlen(buffer));
	open_server_sock(session);

	free(buffer);

 return r;
}

int type(SOCKET sockfd, const char* value, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(256);

	strcpy(buffer,"200 Command OK.\r\n");
	r=sock_write(sockfd, buffer, strlen(buffer));
	session->type = value[0];

	free(buffer);

 return r;
}

int cwd(SOCKET sockfd, const char* value){

        int r;
        char* buffer = (char*)malloc(1024);

	if(strstr(value,"../")!=NULL){
	 strcpy(buffer,"501 Syntax error in parameters or argument.\r\n");
	 r=sock_write(sockfd, buffer, strlen(buffer));
	}else{
	  r=chdir(value);
          if(r>-1) strcpy(buffer,"250 Okay.\r\n");
	  else sprintf(buffer,"550 %s: No such file or directory..\r\n",value);
          r=sock_write(sockfd,buffer,strlen(buffer));
	}


        free(buffer);

  return r;
}

void empty(char* c){}

int pwd(SOCKET sockfd, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(1024);
        char* dir = (char*)malloc(1024);

	empty(getcwd(dir,1024));

	sprintf(buffer,"257 %s/\r\n",&dir[strlen(&session->workdir[0])]);
	r=sock_write(sockfd,buffer,strlen(buffer));

	do_nothing(r);
	free(buffer);
	free(dir);

  return r;
}


int stor(SOCKET sockfd, const char* value, ftp_session* session){

	int r;
        char* buffer = (char*)malloc(256);
	char* file = (char*)malloc(1024);

        FILE* fd;

	sprintf(file,"%s/%s", &session->workdir[0], value);
        fd=fopen(file,"w");
        if(fd==NULL){
          strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer));
        }else{
	  fclose(fd);
          strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer));
          r=ftp_stor(sockfd, value, session);
	}

       free(buffer);
       free(file);

 return r;
}

int retr(SOCKET sockfd, const char* value, ftp_session* session){

	int r;
        char* buffer = (char*)malloc(256);
	char* file = (char*)malloc(1024);
	FILE* fd;

	sprintf(file,"%s/%s", &session->workdir[0], value);
	fd=fopen(file,"r");
	if(fd==NULL){
	  printf("file:%s\n", value);
	  strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer));
	}else{
	  fclose(fd);
          strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer));
          ftp_retr(sockfd, file, session);
	}
	do_nothing(r);
        free(buffer);
        free(file);

 return 1;
}


int list(SOCKET sockfd, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(256);

	strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
	r=sock_write(sockfd,buffer,strlen(buffer));

	r=ftp_list(sockfd, session);

	do_nothing(r);
	free(buffer);

 return 1;
}


int port(SOCKET sockfd, char* value, ftp_session* session){

        int r;
	int p1=0,p2=0,port;
	char* ptr;
        char* buffer = (char*)malloc(256);
	char ip[20];

	memset(&ip[0],0,20);

	ptr=strtok(value,",");
	if(ptr!=NULL){
	 strcat(&ip[0],ptr);
	 strcat(&ip[0],".");

	 ptr=strtok(NULL,",");
	 strcat(&ip[0],ptr);
	 strcat(&ip[0],".");

	 ptr=strtok(NULL,",");
	 strcat(&ip[0],ptr);
	 strcat(&ip[0],".");

	 ptr=strtok(NULL,",");
	 strcat(&ip[0],ptr);

	 ptr=strtok(NULL,",");
	 p1 = atoi(ptr);

	 ptr=strtok(NULL,",");
	 p2 = atoi(ptr);
	}

	port = p1*256+p2;

	session->mode=MODE_ACTIVE;
	session->type=ASCII;
	session->cAddr.port=port;
	strcpy(&session->cAddr.ip[0], &ip[0]);

	strcpy(buffer,"200 PORT command successful.\r\n");
	r=sock_write(sockfd,buffer,strlen(buffer));

	free(buffer);

 return r;
}

int quit(SOCKET sockfd, const char* value){

        int r;
        char* buffer = (char*)malloc(256);
        strcpy(buffer,"200 Goodbye.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer));
	do_nothing(r);
        free(buffer);

 return -1;
}

int syst(SOCKET sockfd, const char* value){

        int r;
        char* buffer = (char*)malloc(256);
        strcpy(buffer,"215 Linux.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer));
        free(buffer);

 return r;
}

int pass(SOCKET sockfd, const char* value){

        int r;
        char* buffer = (char*)malloc(256);
        strcpy(buffer,"230 Login successful.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer));
        free(buffer);

 return r;
}


int user(SOCKET sockfd, const char* value){

	int r;
	char* buffer = (char*)malloc(256);
	strcpy(buffer,"331 Please, specify the password.\r\n");
	r=sock_write(sockfd, buffer, strlen(buffer));
	free(buffer);

 return r;
}

char* ftp_resolve_pasv(struct sockaddr_in* sockAddr, char* ipaddr){

	int i = 0;
	strcpy(ipaddr,inet_ntoa(sockAddr->sin_addr));
	for(i = 0; i < (signed)strlen(ipaddr);i++){
	if(ipaddr[i]=='.') ipaddr[i] = ',';}
        sprintf(ipaddr,"%s,%d,%d",ipaddr,
        HIBYTE(ntohs(sockAddr->sin_port)),
        LOBYTE(ntohs(sockAddr->sin_port)));

 return ipaddr;
}

void handle_session(unsigned int sockfd, ftp_session* session){

	char* buffer = (char*)malloc(1024);
	int r;

	printf("session pasv:%d\n", session->pasv_port);

	sprintf(&session->workdir[0], "%s/ftp", getenv("CORNELIA_HOME"));

	if(chdir(&session->workdir[0])==-1){
	 printf("Error: can't chdir.\n");
	 return;
	}

	strcpy(buffer,"220-Cornelia Web Server\r\n");
	sock_write(sockfd,buffer, strlen(buffer));
	strcpy(buffer,"220 Please visit http://crazedout.com\r\n");
	sock_write(sockfd, buffer, strlen(buffer));
	int loop=1;

	while(loop){
	  memset(buffer,0,1024);
	  r=sock_read(sockfd,buffer,1024);
	  if(r<1) {
	   break;
	  }
	  r=parse_request(sockfd, buffer, session);
	  if(r<1) loop=0;
	}
	shutdown(sockfd, SHUT_RDWR);
	free(buffer);
}

char* parse_pasv_ip(char* ip){

	for(int i = 0; i <strlen(ip); i++){
	  if(ip[i]=='.') ip[i]=',';
	}

 return ip;
}

void init_server(char* pasv_ip, int port) {

    	int sock;
	struct sockaddr_in* pV4Addr;
	struct sockaddr_in addr,cli;
        unsigned int len = sizeof(addr);
	int loop=1;
	unsigned int port_base = 29936;
    	sock = create_socket(port);
	int connections=0;

	printf("Cornelia FTP Server listening on %s %d\n", pasv_ip, port);
	while(loop){
          int client = accept(sock, (struct sockaddr*)&addr, &len);
	  connections++;
          if (client < 0) {
              perror("Unable to accept");
          }else{
	    int pid=fork();
	    if(pid==0){
		ftp_session session;
		memset(&session,0,sizeof(ftp_session));
		session.addr=addr;
		strcpy(&session.pasv_ip[0], parse_pasv_ip(pasv_ip));
		session.pasv_port=port_base + (connections*30);
	        pV4Addr = (struct sockaddr_in*)&cli;
                inet_ntop(AF_INET, &pV4Addr->sin_addr, &session.clientIP[0], INET_ADDRSTRLEN );
	    	handle_session(client,&session);
	        loop=0;
	    }
	  }
	}

}

int list_ip() {

   struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

void usage(){
	printf("Cornelia simple FTP Server\n");
	printf("CrazedoutSoft (c) 2022\n\n");
	printf("usage:\n");
	printf("ftp_cornelia -bind <ip> -port <port> -lip [list availbale ip adresses]\n");
	printf("Example:\n");
	printf(">ftp_cornelia -lip\n");
	printf("eth0 IP Address 169.254.255.169\nlo IP Address 127.0.0.1\nwifi0 IP Address 192.168.10.145\n");
	printf("\n>ftp_cornelia -bind 192.168.10.145 -port 8021\n\n");
}

int main(int args, char* argv[]){

	char* dir = (char*)malloc(1024);
	char* bind = (char*)malloc(20);
	char* port = (char*)malloc(20);

	get_work_dir(dir,1024);

	if(getenv("CORNELIA_HOME")==NULL){
	  printf("env CORNELIA_HOME should be set to cornelia_d workdir\n");
	  printf("export CORNELIA_HOME=<dir of cornelia_d>\n");
	  printf("Asuming: %s - Lets's try it..\n", dir);
	  setenv("CORNELIA_HOME",dir,1);
	}
	free(dir);

	if(args<2) {
	  usage();
	  free(dir);
	  free(bind);
	  free(port);
	  return 0;
	}

	for(int i = 0; i < args; i++){

	  if(strcmp(argv[i],"-bind")==0){
	   if(i<args-1) strcpy(bind,argv[i+1]);
	  }
	  else if(strcmp(argv[i],"-lip")==0) {
	    list_ip();
	    return 0;
	  }
	  else if(strcmp(argv[i],"-port")==0){
	   if(i<args-1) strcpy(port,argv[i+1]);
	  }
	}

	if(strlen(bind)==0){
	 strcpy(bind, "127.0.0.1");
	 printf("Bind adress missing. defaulting to 127.0.0.1\n");
	}
	if(strlen(port)==0){
	 strcpy(port, "8021");
	 printf("port missing. defaulting to 8021\n");
	}

	init_server(bind, atoi(port));

	free(dir);
	free(bind);
	free(port);

  return 0;
}
