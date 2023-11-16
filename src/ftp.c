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


#ifndef NO_SSL
#include "../include/ftps.h"
#endif
#include "../include/ftp.h"

#include <sys/wait.h>

unsigned int anonymous_allowed = 0;
int trace = 0;

int create_socket(int port) {

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

int has_read(ftp_session* session){
	return ((session->cred >> READ) & 1U) > 0;
}

int has_write(ftp_session* session){
	return ((session->cred >> WRITE) & 1U) > 0;
}

void set_cred(ftp_session* session, unsigned int cred){
	 session->cred |= 1UL << cred;
}

int sock_write(int sockfd, char* buffer, int len, void* ssl){
	if(ssl==NULL){
	  return write(sockfd, buffer, len);
	}else{
	 int r = -1;
         #ifndef NO_SSL
         r = ssl_write(ssl, buffer, len);
         #endif
	 return r;
	}
}

int sock_read(int sockfd, char* buffer, int len, void* ssl){
	if(ssl==NULL){
	  return read(sockfd, buffer, len);
	}else{
	  int r = -1;
	  #ifndef NO_SSL
          r = ssl_read(ssl, buffer, len);
          #endif
	  return r;
	}
}

FILE* popen(const char* cmd, const char* mode);

char* ftp_resolve_pasv(struct sockaddr_in* sockAddr, char* ipaddr);

int pass(SOCKET sockfd, ftp_session* session, const char* value);
int user(SOCKET sockfd, ftp_session* session, const char* value);
int quit(SOCKET sockfd, ftp_session*session, const char* value);
int noop(SOCKET sockfd, const char* value);
int syst(SOCKET sockfd, ftp_session* session, const char* value);
int list(SOCKET sockfd, ftp_session* session);
int port(SOCKET sockfd, char* value, ftp_session* session);
int pwd(SOCKET sockfd, ftp_session* session);
int cwd(SOCKET sockfd, const char* value, ftp_session* session);
int retr(SOCKET sockfd, const char* value, ftp_session* session);
int type(SOCKET sockfd, const char* value, ftp_session* session);
int pasv(SOCKET sockfd, ftp_session* session);
int stor(SOCKET sockfd, const char* value, ftp_session* session);
int mkd(SOCKET sockfd, ftp_session* session, const char* value);
int rnfr(SOCKET sockfd, ftp_session* session, const char* value);
int rnto(SOCKET sockfd, ftp_session* session, const char* value);
int dele(SOCKET sockfd, ftp_session* session, const char* value);
int login(ftp_session* session);

char* find_user_passwd(const char* user, char* buffer, int len){

	FILE *fd;
	char *ptr=NULL;
	char* ftp_passwd = (char*)malloc(1024);

	sprintf(ftp_passwd,"%s/conf/ftp.passwd", getenv("CORNELIA_HOME"));

	if((fd=fopen(ftp_passwd,"r"))!=NULL){
	  while((fgets(buffer,len,fd))!=NULL){
	   if(buffer[0]=='#') continue;
	   if((ptr=strstr(buffer,":"))!=NULL){
	     int i = ptr-buffer;
	     buffer[i]='\0';
	     if(strcmp(user,buffer)==0){
		free(ftp_passwd);
		fclose(fd);
	        return clip(ptr+1);
	     }
	   }
	  }
	  fclose(fd);
	}

	free(ftp_passwd);

  return NULL;
}

int open_server_sock(ftp_session* session){

	unsigned int servsock;
	struct sockaddr_in cli_addr;
	unsigned int clilen;

	session->pasv_sockfd=0;
	servsock=create_socket(session->pasv_port);
         if((session->pasv_sockfd = accept(servsock, (struct sockaddr *) &cli_addr, &clilen))<0){
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
          s=session->pasv_sockfd;
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
	while((r=sock_read(s,buffer,1023,NULL))>0){
	  buffer[r]='\0';
	  fwrite(buffer,1,r,fd);
	}
	fclose(fd);

        strcpy(buffer,"226 Closing data connection.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer), session->tls);
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
	  s=session->pasv_sockfd;
	}
        if(s<0){
         printf("Socket fail %s %d\n", &session->cAddr.ip[0], session->cAddr.port);
         free(buffer);
         return -1;
        }

	fd=fopen(value,session->type==ASCII?"r":"rb");
	if(fd==NULL){
	  strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
	}else{
	  mem=(char*)malloc(1024);
	  while((r=fread(mem,1,1024,fd))>0){
	  	sock_write(s,mem,r,NULL);
	  }
	  fclose(fd);
	  free(mem);
	  strcpy(buffer,"226 Closing data connection.\r\n");
	  r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
	  shutdown(s,SHUT_RDWR);
	  shutdown(session->cAddr.sockfd,SHUT_RDWR);
	}
        free(buffer);

return r;
}

void send_err(SOCKET sockfd, ftp_session* session){

	char* buffer = (char*)malloc(1024);
	strcpy(buffer,"450 Couldn't open the file or directory\r\n");
	sock_write(sockfd,buffer,strlen(buffer),session->tls);
	shutdown(sockfd, SHUT_RDWR);
	free(buffer);

}

int ftp_list(SOCKET sockfd, ftp_session* session){

	int n = 0;
	int r=0;
	char* buffer = (char*)malloc(1024);
        char* argv[5];
	SOCKET s;


	if(session->mode==MODE_ACTIVE){
	  s = open_socket_by_ip(&session->cAddr.ip[0], session->cAddr.port);
	}else{
	  s=session->pasv_sockfd;
	}
	if(s<0){
	 printf("Socket fail %s %d\n", &session->cAddr.ip[0], session->cAddr.port);
	 free(buffer);
	 return -1;
	}

        int pipefd[2];
        if(pipe(pipefd)==-1){
	  send_err(sockfd,session);
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
	    send_err(sockfd,session);
	  }
	}else if(pid==-1){
	  send_err(sockfd,session);
	  perror("bad fork() list dir\n");
	}else{
	  close(pipefd[1]);
	  memset(buffer,0,1024);
	  int rn = 0;
	  while((r=read(pipefd[0], buffer, 1024))>0){
	      buffer[r]='\0';
	      rn=sock_write(s,buffer,strlen(buffer),NULL);
	      (void)(rn);
	  }
	  close(pipefd[0]);
	  shutdown(s,SHUT_RDWR);
	  strcpy(buffer,"226 Closing data connection.\r\n");
	  r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
	}

	while(1){
	  if(argv[n]==NULL) break;
	  free(argv[n++]);
	}

	free(buffer);

return r;
}

char* handle_file_spaces(char* buffer){

    int i=0;
    int n=0;
    char* verb = (char*)malloc(16);

    memset(verb,0,16);
    for(i=0;i<(int)strlen(buffer);i++){
       if(buffer[i]==' ' && !n) {
        memcpy(verb,buffer,i);
        if(strcmp(verb,STOR)==0 || strcmp(verb,RETR)==0){
         n++;
        }else break;
      }else if(buffer[i]==' ' && n){
        buffer[i]='_';
      }
    }

 return buffer;
}

int parse_request(SOCKET sockfd, char* buffer, ftp_session* session){

	char *ptr;
	char verb[12];
	char *value = NULL;
	char *value2 = NULL;
	int r = 0;

    if(trace) printf("%s\n", buffer);

    handle_file_spaces(buffer);

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
	  ptr=strtok(NULL," ");
	  if(ptr!=NULL){
	   value2=(char*)malloc(strlen(ptr));
	   memset(value2,0, strlen(ptr));
	   strcpy(value2, clip(ptr));
	   value=clip(value2);
	  }
	}

	if(trace) printf("%s %s %s\n", &verb[0], value, value2);

	if(strcmp(&verb[0],USER)==0) r=user(sockfd, session, value);
	else if(strcmp(&verb[0],PASS)==0) r=pass(sockfd, session, value);
	else if(strcmp(&verb[0],QUIT)==0) r=quit(sockfd, session, value);
	else if(strcmp(&verb[0],SYST)==0) r=syst(sockfd, session, value);
	else if(strcmp(&verb[0],PORT)==0) r=port(sockfd, value, session);
	else if(strcmp(&verb[0],LIST)==0) r=list(sockfd, session);
	else if(strcmp(&verb[0],RETR)==0) r=retr(sockfd, value, session);
	else if(strcmp(&verb[0],STOR)==0) r=stor(sockfd, value, session);
	else if(strcmp(&verb[0],PWD)==0) r=pwd(sockfd,session);
	else if(strcmp(&verb[0],XPWD)==0) r=pwd(sockfd,session);
	else if(strcmp(&verb[0],CWD)==0) r=cwd(sockfd, value,session);
	else if(strcmp(&verb[0],XCWD)==0) r=cwd(sockfd, value,session);
	else if(strcmp(&verb[0],TYPE)==0) r=type(sockfd,value,session);
	else if(strcmp(&verb[0],PASV)==0) r=pasv(sockfd,session);
	else if(strcmp(&verb[0],RNFR)==0) r=rnfr(sockfd,session,value);
	else if(strcmp(&verb[0],RNTO)==0) r=rnto(sockfd,session,value);
	else if(strcmp(&verb[0],DELE)==0) r=dele(sockfd,session,value);
	else if(strcmp(&verb[0],MKD)==0) r=mkd(sockfd,session,value);
	else{
	  strcpy(buffer,"502 Command not implemented.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
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

int check_cred(SOCKET sockfd, ftp_session* session, unsigned int type){

	char* buffer;

	if(((session->cred >> type) & 1U) > 0){
	  return 1;
	}
	else{
	  buffer = (char*)malloc(64);
	  strcpy(buffer, "550 Access is denied.\r\n");
	  sock_write(sockfd, buffer, strlen(buffer),session->tls);
	  free(buffer);
	}

	return 0;
}

int dele(SOCKET sockfd, ftp_session* session, const char* value){

	char* buffer;

	if(!check_cred(sockfd, session, WRITE)) return 1;

	buffer = (char*)malloc(256);
	if(remove(value)==0){
	    strcpy(buffer,"250 Requested file action okay, completed.\r\n");
	}else{
	  sprintf(buffer,"550 Requested action not taken; file unavailable...\r\n");
	}
	sock_write(sockfd, buffer, strlen(buffer),session->tls);
	free(buffer);

	(void)(session);

 return 1;
}

int rnto(SOCKET sockfd, ftp_session* session, const char* value){

	char* buffer;

	if(!check_cred(sockfd,session, WRITE)) return 1;

	buffer = (char*)malloc(256);
	char err[] = "550 Requested action not taken; file unavailable...\r\n";

	if(session->tmp_value!=NULL){
	  if(rename(session->tmp_value, value)==0){
	    strcpy(buffer,"250 Requested file action okay, completed.\r\n");
	  }else{
	    strcpy(buffer,&err[0]);
	  }
	  free(session->tmp_value);
	}else{
	  strcpy(buffer,&err[0]);
	}
	sock_write(sockfd, buffer, strlen(buffer),session->tls);
	free(buffer);
 return 1;
}

int rnfr(SOCKET sockfd, ftp_session* session, const char* value){

	char* buffer;

	if(!check_cred(sockfd,session, WRITE)) return 1;

	buffer = (char*)malloc(256);
	FILE* fd;
	if((fd=fopen(value,"r"))!=NULL){
	  strcpy(buffer,"350 RNFR accepted. Please supply new name for RNTO.\r\n");
	  session->tmp_value=(char*)malloc(1024);
	  strcpy(session->tmp_value, value);
	  fclose(fd);
	}else{
	  sprintf(buffer,"550 Requested action not taken; file unavailable...\r\n");
	}
	sock_write(sockfd, buffer, strlen(buffer),session->tls);
	free(buffer);

  return 1;
}

int mkd(SOCKET sockfd, ftp_session* session, const char* value){

	char* buffer;

	if(!check_cred(sockfd, session, WRITE)) return 1;

	buffer = (char*)malloc(256);
	if(mkdir(value, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0){
	  sprintf(buffer, "257 \"%s\" : The directory was successfully created\r\n", value);
	}else{
	  sprintf(buffer,"550 Requested action not taken; file unavailable...\r\n");
	}
	sock_write(sockfd, buffer, strlen(buffer),session->tls);
	free(buffer);


 return 1;
}

int pasv(SOCKET sockfd, ftp_session* session){

	int r;
	char* buffer;

	buffer=(char*)malloc(1024);
	char hilow[8];
	session->pasv_port++;
	session->mode=MODE_PASV;
	// TO_DO: fix port range... TO DO: in one word fucks whith my eyes in nano.
	sprintf(buffer,"227 Entering Passive Mode (%s,%s)\r\n", &session->pasv_ip[0], toHiLow(session->pasv_port,&hilow[0]));
	r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
	open_server_sock(session);

	free(buffer);

 return r;
}

int type(SOCKET sockfd, const char* value, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(256);

	strcpy(buffer,"200 Command OK.\r\n");
	r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
	session->type = value[0];

	free(buffer);

 return r;
}

int cwd(SOCKET sockfd, const char* value, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(4097);
        char* path = (char*)malloc(4097);
        char* tmp = (char*)malloc(4097);

	strcpy(tmp, &session->workdir[0]);

	if(value[0]=='/'){
		sprintf(path,"%s/%s", getenv("CORNELIA_HOME"), value);
	}else{
	if(session->workdir[strlen(&session->workdir[0])-1] =='/'){
	  sprintf(path,"%s%s", &session->workdir[0], value);
	}else{
	  sprintf(path,"%s/%s", &session->workdir[0], value);
	}
	}
	strcpy(&session->workdir[0],trimpath(path));

	if(strlen(&session->workdir[0]) < strlen(&session->root[0])) {
	  strcpy(&session->workdir[0],tmp);
	  r=-1;
	}
        else r=chdir(path);

        if(r>-1) strcpy(buffer,"250 Okay.\r\n");
	else sprintf(buffer,"550 %s: No such file or directory..\r\n",value);
        r=sock_write(sockfd,buffer,strlen(buffer),session->tls);

        free(buffer);
	free(path);
	free(tmp);

  return r;
}

int pwd(SOCKET sockfd, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(5000);
        char* dir = (char*)malloc(4096);
	char* c_home = getenv("CORNELIA_HOME");

	if(getcwd(dir,4096)==NULL){
	  strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
	}else{
	  sprintf(buffer,"257 %s\r\n", &dir[strlen(c_home)]);
	}

	(void)(session);

	r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
	free(buffer);
	free(dir);
	(void)(r);

  return r;
}


int stor(SOCKET sockfd, const char* value, ftp_session* session){

	int r;
        char* buffer;
	char* file;

	if(!check_cred(sockfd,session, WRITE)) return 1;

	buffer = (char*)malloc(256);
	file=(char*)malloc(4097);

        FILE* fd;

	sprintf(file,"%s", value);
        fd=fopen(file,"w");
        if(fd==NULL){
          strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
        }else{
	  fclose(fd);
          strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
          r=ftp_stor(sockfd, value, session);
	}

       free(buffer);
       free(file);

 return r;
}

int retr(SOCKET sockfd, const char* value, ftp_session* session){

	int r;
	char* buffer;
	char* file;

	if(!check_cred(sockfd,session, READ)) return 1;

        buffer = (char*)malloc(256);
	file = (char*)malloc(4097);
	FILE* fd;

	sprintf(file,"%s",value);
	fd=fopen(file,"r");
	if(fd==NULL){
	  strcpy(buffer,"451 Requested action aborted: local error in processing.\r\n");
	  r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
	}else{
	  fclose(fd);
          strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
          r=sock_write(sockfd,buffer,strlen(buffer),session->tls);
          ftp_retr(sockfd, file, session);
	}
	(void)(r);
        free(buffer);
        free(file);

 return 1;
}


int list(SOCKET sockfd, ftp_session* session){

        int r;
        char* buffer = (char*)malloc(256);

	strcpy(buffer,"150 File status okay; about to open data connection.\r\n");
	r=sock_write(sockfd,buffer,strlen(buffer),session->tls);

	r=ftp_list(sockfd, session);

	(void)(r);
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
	session->cAddr.port=port;
	strcpy(&session->cAddr.ip[0], &ip[0]);

	strcpy(buffer,"200 PORT command successful.\r\n");
	r=sock_write(sockfd,buffer,strlen(buffer),session->tls);

	free(buffer);

 return r;
}

int quit(SOCKET sockfd, ftp_session* session, const char* value){

        int r;
        char* buffer = (char*)malloc(256);
        strcpy(buffer,"200 Goodbye.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer), session->tls);
	(void)(r);
        free(buffer);
	(void)(value);

 return -1;
}

int syst(SOCKET sockfd, ftp_session* session, const char* value){

        int r;
        char* buffer = (char*)malloc(256);
        strcpy(buffer,"215 Cornelia FTP server on Linux.\r\n");
        r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
        free(buffer);
	(void)(value);

 return r;
}

int pass(SOCKET sockfd, ftp_session* session, const char* value){

        int r,ret;
        char* buffer = (char*)malloc(256);
	strcpy(&session->pass[0],value);
	if(login(session)==0){
	  ret=1;
          strcpy(buffer,"230 Login successful.\r\n");
	}else{
	  ret=-1;
	  strcpy(buffer,"530 Login incorrect.\r\n");
	}
        r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
        free(buffer);
	(void)(r);

 return ret;
}


int user(SOCKET sockfd, ftp_session* session, const char* value){

	int r;
	char* buffer = (char*)malloc(256);
	strcpy(&session->user[0],value);
	strcpy(buffer,"331 Please, specify the password.\r\n");
	r=sock_write(sockfd, buffer, strlen(buffer),session->tls);
	free(buffer);

 return r;
}

int login(ftp_session* session){

	char* passwd;
	char buffer[128];

	if(anonymous_allowed && (strcmp(&session->user[0],ANONYMOUS)==0)){
	  set_cred(session,READ);
	  return 0;
	}
	else {
	  set_cred(session,READ);
	  set_cred(session,WRITE);
	}
	passwd=find_user_passwd(&session->user[0],&buffer[0],128);
	if(passwd==NULL) return -1;
	return strcmp(passwd,crypt(&session->pass[0],SALT));

  return -1;
}

void handle_session(unsigned int sockfd, ftp_session* session){

	char* buffer = (char*)malloc(1024);
	int r;

	if(chdir(&session->workdir[0])==-1){
	 printf("Error: can't chdir.\n");
	 return;
	}

	strcpy(buffer,"220-Cornelia Web Server\r\n");
	sock_write(sockfd,buffer, strlen(buffer),session->tls);
	strcpy(buffer,"220 Please visit http://crazedout.com\r\n");
	sock_write(sockfd, buffer, strlen(buffer),session->tls);
	int loop=1;

	while(loop){
	  memset(buffer,0,1024);
	  r=sock_read(sockfd,buffer,1024,session->tls);
	  if(r<1) {
	   break;
	  }
      if(trace) printf("%s\n", buffer);
	  r=parse_request(sockfd, buffer, session);
	  if(r<1) loop=0;
	}


	shutdown(sockfd, SHUT_RDWR);
	free(buffer);
}

char* parse_pasv_ip(char* ip){

	for(int i = 0; i < (int)strlen(ip); i++){
	  if(ip[i]=='.') ip[i]=',';
	}

 return ip;
}

void init_server(char* pasv_ip, int port, const char* root, int mode) {

    	int sock;
	struct sockaddr_in* pV4Addr;
	struct sockaddr_in addr,cli;
        unsigned int len = sizeof(addr);
	int loop=1;
	unsigned int port_base = 29936;
    	sock = create_socket(port);
	int connections=0;

	#ifndef NO_SSL
	 SSL_CTX *ctx = NULL;
	 SSL *ssl = NULL;
	#endif

	if(mode==FTP){
	  printf("*** Cornelia FTP Server listening on %s:%d at %s ***\n", pasv_ip, port, root);
	}else if(mode==FTPS){
	  #ifndef NO_SSL
	   printf("*** Cornelia FTPS Server listening on %s:%d at %s ***\n", pasv_ip, port, root);
	   ctx = create_context();
           configure_context(ctx,"cert/cert.crt","cert/mykey.key");
	  #endif
	}

	while(loop){
          int client = accept(sock, (struct sockaddr*)&addr, &len);
	  connections++;
          if (client < 0) {
              perror("Unable to accept");
          }else{
	    if(mode==FTPS){
	      #ifndef NO_SSL
	      ssl = SSL_new(ctx);
              SSL_set_fd(ssl, client);
	      if (SSL_accept(ssl) <= 0) {
                perror("Error accept TLS.\n");
              }
	     #endif
	    }
	    int pid=fork();
	    if(pid==0){
		ftp_session session;
		memset(&session,0,sizeof(ftp_session));
		session.addr=addr;
		#ifndef NO_SSL
	         session.tls = ssl;
		 session.ctx = ctx;
	        #endif
		if(root[0]=='/'){
		  sprintf(&session.workdir[0], "%s", root);
		}else{
		  sprintf(&session.workdir[0], "%s/%s", getenv("CORNELIA_HOME"), root);
		}
		strcpy(&session.root[0], &session.workdir[0]);
		strcpy(&session.pasv_ip[0], parse_pasv_ip(pasv_ip));
		session.pasv_port=port_base + (connections*30);
	        pV4Addr = (struct sockaddr_in*)&cli;
                inet_ntop(AF_INET, &pV4Addr->sin_addr, &session.clientIP[0], INET_ADDRSTRLEN );
	    	handle_session(client,&session);
		#ifndef NO_SSL
		 SSL_shutdown(session.tls);
		 SSL_free(session.tls);
		 SSL_CTX_free(ctx);
		#endif
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
	printf("ftp_cornelia -root <dir root> -bind <ip> -port <port> -lip [list availbale ip adresses] -anonymous_allowed\n");
	printf("Example:\n");
	printf(">ftp_cornelia -lip\n");
	printf("eth0 IP Address 169.254.255.169\nlo IP Address 127.0.0.1\nwifi0 IP Address 192.168.10.145\n");
	printf("\n>ftp_cornelia -bind 192.168.10.145 -port 8021\n\n");
}

int main(int args, char* argv[]){

	char* dir = (char*)malloc(1024);
	char* bind = (char*)malloc(20);
	char* port = (char*)malloc(20);
	char* root = (char*)malloc(2048);
	int mode = FTP;

	dir = getcwd(dir,1024);

	if(getenv("CORNELIA_HOME")==NULL){
	  printf("env CORNELIA_HOME should be set to cornelia workdir.\n");
	  printf("export CORNELIA_HOME=<dir of cornelia>\n");
	  printf("Asuming: %s - Lets's try it..\n", dir);
	  setenv("CORNELIA_HOME",dir,1);
	}
	free(dir);

	if(args<2) {
	  printf("\nAll default values. Try ftp_cornelia --help\r\n");
	}

	for(int i = 0; i < args; i++){

	  if(strcmp(argv[i],"-bind")==0){
	   if(i<args-1) strcpy(bind,argv[i+1]);
	  }
	  else if(strcmp(argv[i],"-lip")==0) {
	    list_ip();
	    return 0;
	  }
	  else if(strcmp(argv[i],"-trace")==0) {
	    trace=1;
	  }
	  else if(strcmp(argv[i],"-tls")==0) {
	    #ifndef NO_SSL
	      mode = FTPS;
	    #else
	     printf("\nWARN! Compiled with NO_SSL. -tls is not available.\n");
	    #endif
	  }
	  else if(strcmp(argv[i],"-port")==0){
	   if(i<args-1) strcpy(port,argv[i+1]);
	  }
	  else if(strcmp(argv[i],"-root")==0){
	   if(i<args-1) strcpy(root,argv[i+1]);
	   if(strstr(root,"../")!=NULL){
	     printf("Root is not allowed to be relative '%s'. Defaulting to 'ftp'\n", root);
	     strcpy(root,"ftp");
	   }
	  }
	  else if(strcmp(argv[i],"-anonymous_allowed")==0){
	    anonymous_allowed=1;
	  }
	  else if(strcmp(argv[i],"--help")==0){
	    usage();
	    free(bind);
	    free(port);
	    free(root);
	    return 0;
	  }
	}

	char ip[128];
	FILE* pd;
	int nr = 0;
	if(strlen(bind)==0){
	 pd = popen("bin/findip 8031","r");
	 if(pd!=NULL){
	   while(fgets(&ip[0],128,pd)!=NULL) {
	     if(nr==1) {
	        ip[strlen(&ip[0])-1] = '\0';
		strcpy(bind,&ip[0]);
	        break;
	     }
	     nr++;
	   }
	   fclose(pd);
	 }else{
	   strcpy(bind,LOCALHOST);
	 }
	 printf("Bind adress missing - defaulting to %s\n", bind);
	}

	if(strlen(port)==0){
	 strcpy(port, "8021");
	 printf("Port missing - defaulting to 8021\n");
	}
	if(strlen(root)==0){
	 strcpy(root, "ftp");
	 printf("Root dir missing - defaulting to $CORNELIA_HOME/ftp\n");
	}

	init_server(bind, atoi(port), root, mode);

	free(root);
	free(bind);
	free(port);

  return 0;
}
