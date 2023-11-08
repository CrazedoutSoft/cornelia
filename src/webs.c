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

/*
	This project was initially started to teach my son Alfred C programming.
	Cornelia is his younger sister. She is Small, Fast and Furious...
	/Fredrik.
*/

#include "../include/mkpasswd.h"
#include "../include/tls.h"
#include "../include/ssl.h"
#include "../include/conf.h"
#include "../include/misc.h"
#include "../include/base64.h"
#include "../include/webs.h"

#define SA struct sockaddr
#define AUTH_REQUEST_SENT	0
#define AUTH_OK			1
#define BAD_AUTH		-1
#define CGI_BIN	 		"/cgi-bin/"

#define CONN_KEEP_ALIVE		1
#define CONN_CLOSE		0
#define MAX_ALLOC		65536

#define HTTP_POST		"POST"
#define HTTP_GET		"GET"
#define HTTP_PUT		"PUT"
#define HTTP_HEAD		"HEAD"
#define HTTP_OPTIONS		"OPTIONS"
#define AUTHORIZATION 		"Authorization="

#define D_404_NOT_FOUND		"404 Not Found"
#define D_200_OK		"200 OK"


int c_debug = 0;
server_conf serv_conf;
auth_conf   a_conf;

char* bad_request;
char* internal_server_error;
char* forbidden;
char* unauthorized;
char* http_options;
char* ACAOrigin;
char  conf_file[1024] = "conf/corny.conf";
char  cip[16];
void dump_request(http_request* r);
user_endpoint* uep = NULL;


void usleep(unsigned long);


void init_server() {

	int loop=1;
	int connfd;
	SOCKET port = serv_conf.port;
        int sockfd;
	unsigned int len;
        struct sockaddr_in servaddr, cli;
        struct sockaddr_in* pV4Addr;
        struct in_addr ipAddr;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
	  perror("Fatal: Socket creation failed.\n");
	  exit(-1);
        }
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);

        if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
	  perror("Fatal: Socket bind failed.\n\nTry bin/restart.sh\n\n");
	  exit(-1);
        }

        if ((listen(sockfd, 5)) != 0) {
	  perror("Fatal: sock listen failed.\n\nTry bin/restart.sh\n\n");
	  exit(-1);
        }

        len = sizeof(cli);
        printf("\nCornelia listening on %d [HTTP]\n", serv_conf.port);

        while(loop){
	  connfd = accept(sockfd, (SA*)&cli, &len);
	  int pid = fork();
	  if(pid==0){

	    memset(&cip[0],0,16);
            pV4Addr = (struct sockaddr_in*)&cli;
            ipAddr = pV4Addr->sin_addr;
            inet_ntop(AF_INET, &ipAddr, &cip[0], INET_ADDRSTRLEN );

	    struct timeval tv;
	    tv.tv_sec = 0;
	    tv.tv_usec = serv_conf.keep_alive_timeout;
	    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	    handle_request(connfd,cip,NULL);
	    shutdown(connfd,SHUT_RDWR);

	    loop=0;
	    if(c_debug) printf("exit\n");
	  }
        }
        if (connfd < 0) {
	  perror("Fatal:Server accept failed.\n");
	  exit(-1);
        }

}

int get_file_size(const http_request* request){

        FILE *fd;
        int   size=-1;
        char  *tmp = (char*)malloc(MAX_ALLOC);
        sprintf(tmp,"%s%s%s",&request->virtual_path[0],&request->path[0],&request->file[0]);

	 if((fd=fopen(tmp,"rb"))!=NULL){
           fseek(fd,0L,SEEK_END);
           size = ftell(fd);
           fseek(fd,0L,SEEK_SET);
           fclose(fd);
        }
        free(tmp);

return size;
}

int readline_ssl(const http_request* request, char* buffer, int len){

        char sb[2];
        int n = 0;
        char c = 0;
	int r=0;

          while((r=socket_read(request,&sb[0],1))>0 && n<len-1){
	   if(r==-1) {
	    return -1;
	   }
           c = sb[0];
           if(c=='\r') continue;
             if(c == '\n'){
             buffer[n] = '\0';
             break;
            }
            buffer[n++] = c;
          }
	  if(n==0) buffer[0]='\0';


 return n;
}

int readline(const http_request* request, char* buffer, int len){

        char sb[2];
        int n = 0;
        char c = 0;
	int r=0;

	struct pollfd fd;
	int ret;

	if(request->cSSL!=NULL){
	  return readline_ssl(request, buffer, len);
	}

	fd.fd = request->sockfd;
	fd.events = POLLIN;
	ret = poll(&fd, 1, (serv_conf.keep_alive_timeout/1000));

	switch (ret) {
    	  case -1:
	  case 0:
          return -1; // Socket timed out.
      	default:

          while((r=socket_read(request,&sb[0],1))>0 && n<len-1){
	   if(r==-1) return -1;
           c = sb[0];
           if(c=='\r') continue;
             if(c == '\n'){
             buffer[n] = '\0';
             break;
            }
            buffer[n++] = c;
          }
	  if(n==0) buffer[0]='\0';

          break;
	}

 return n;
}

int socket_read(const http_request* request, char* buffer, int len){

	int r = 0;
	if(request->cSSL==NULL){
	  r=read(request->sockfd, buffer, len);
	}else{
	  #ifndef NO_SSL
	  r=ssl_read(request->cSSL, buffer, len);
	  #endif
	}

  return r;
}

int socket_write(const http_request* request, const char* buffer, int len){

	int r = 0;
	if(request->cSSL==NULL){
	  r=write(request->sockfd, buffer, len);
	}else{
	 #ifndef NO_SSL
	 r=ssl_write(request->cSSL, buffer, len);
	 #endif
	}

  return r;
}

void send_options_reply(http_request* request){

	socket_write(request, http_options, strlen(http_options));

}

void send_bad_request(http_response* response, char* code){

     	char* buffer=(char*)malloc(4000);
	response->content_length=strlen(bad_request);
	strcpy(&response->content_type[0],"text/html");
        get_head(response, &buffer[0], code);
	socket_write(response->request, &buffer[0], strlen(&buffer[0]));
	socket_write(response->request,"\r\n",2);
        socket_write(response->request, bad_request, strlen(bad_request));
	socket_write(response->request, "\n\n",2);
	free(buffer);
}

void send_bad_request2(http_request* request){

        char* buffer=(char*)malloc(4000);
        http_response response;
        response.request=request;
        response.content_length=strlen(bad_request);
        strcpy(&response.content_type[0],"text/html");
        get_head(&response, &buffer[0], D_404_NOT_FOUND);

        socket_write(request, &buffer[0], strlen(&buffer[0]));
        socket_write(request,"\r\n",2);
        socket_write(request, bad_request, strlen(bad_request));
	socket_write(request, "\n\n",2);

        free(buffer);
}

void send_forbidden(http_request* request){

     	char* buffer=(char*)malloc(4000);
	http_response response;
	response.request=request;
        response.content_length=strlen(forbidden);
        strcpy(&response.content_type[0],"text/html");
        get_head(&response, &buffer[0], "403 Forbidden");

	socket_write(request, &buffer[0], strlen(&buffer[0]));
        socket_write(request,"\r\n",2);
        socket_write(request, forbidden, strlen(forbidden));
	socket_write(request, "\n\n",2);

	free(buffer);
}

void send_internal_error(http_response* response){

     	char* buffer=(char*)malloc(4000);
        response->content_length=strlen(internal_server_error);
        strcpy(&response->content_type[0],"text/html");
        get_head(response, &buffer[0], "500 Internal Server Error");
        socket_write(response->request, &buffer[0], strlen(&buffer[0]));
        socket_write(response->request,"\r\n",2);
        socket_write(response->request, internal_server_error, strlen(internal_server_error));
	socket_write(response->request, "\n\n",2);
	free(buffer);
}

void list_dir (const char* dir, char* buffer) {

   DIR *dp;
   struct dirent *ep;
   char* tmp = (char*)malloc(1024);
   char* fold = (char*)malloc(MAX_ALLOC);
   char* reg = (char*)malloc(MAX_ALLOC);

   memset(tmp, 0, 1024);
   memset(fold,0, MAX_ALLOC);
   memset(reg, 0, MAX_ALLOC);

   dp = opendir (dir);
   if (dp != NULL){

      while ((ep = readdir (dp))){
        if(strlen(ep->d_name)<2) continue;
        if(strcmp(ep->d_name,"..")==0){
          sprintf(tmp,"<a href=\"%s\"><img src=\"/res/back.gif\" /> </a><a href=\"%s\">%s</a><br/>\n", ep->d_name, ep->d_name, ep->d_name);
	  strcat(buffer,tmp);
	  continue;
        }else if(ep->d_type==4){
          sprintf(tmp,"<a href=\"%s/\"><img src=\"/res/folder.gif\" /> </a><a href=\"%s/\">%s</a><br/>\n", ep->d_name, ep->d_name, ep->d_name);
	  strcat(fold,tmp);
	  continue;
        }else if(ep->d_type==8) {
          sprintf(tmp,"<a href=\"%s\"><img src=\"/res/text.gif\" /> </a><a href=\"%s\">%s</a><br/>\n", ep->d_name,ep->d_name, ep->d_name);
	  strcat(reg,tmp);
        }
      }

      if(strlen(buffer)+strlen(fold)>MAX_ALLOC){
	 int size=strlen(buffer)+strlen(fold)+1;
	 buffer=(char*)realloc(buffer, size);
         strcat(buffer, fold);
      }else{
	strcat(buffer,fold);
      }

      if(strlen(buffer)+strlen(reg)>MAX_ALLOC){
	 int size=strlen(buffer)+strlen(reg)+1;
	 buffer=(char*)realloc(buffer, size);
         strcat(buffer, reg);
      }else{
	strcat(buffer,reg);
      }

      (void) closedir (dp);
    }
  else{
    perror ("Couldn't open the directory for listing");
  }

  free(reg);
  free(fold);
  free(tmp);

}

void send_list_dir(http_request* request){

	char* buffer = (char*)malloc(MAX_ALLOC);
	char* dir = (char*)malloc(MAX_ALLOC);
	char* tmp = (char*)malloc(MAX_ALLOC);
	char head[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n";

	memset(buffer,0,MAX_ALLOC);
	memset(dir,0,MAX_ALLOC);
	memset(tmp,0,MAX_ALLOC);

	sprintf(dir,"%s/%s%s%s", &serv_conf.workdir[0], &request->virtual_path[0], &request->path[0], &request->file[0]);
	if(file_exists(dir)){
	  list_dir(dir,buffer);
	  sprintf(tmp,"<!DOCTYPE html>\n<html><head><title>%s</title></head><body><b><i>Cornelia Web Server by CrazedoutSoft (c)</i></b><br><h1>Index of %s</h1>\n",
		&request->path[0], &request->path[0]);

	  memset(dir,0,1024);
	  sprintf(dir,"%sContent-Length: %d\r\n", &head[0], (int)strlen(buffer)+(int)strlen(tmp)+14);

	  socket_write(request,dir,strlen(dir));
          socket_write(request,"\r\n",2);
	  socket_write(request,tmp,strlen(tmp));
          socket_write(request, buffer, strlen(buffer));
	  socket_write(request, "</body></html>", 14);
	}else {
	  send_bad_request2(request);
	}

	free(tmp);
	free(buffer);
	free(dir);

}

int find_default_page(http_request* request){

	char* ptr;
	char* fi = (char*)malloc(MAX_ALLOC);
	int found=0;
	char* tmp = (char*)malloc(strlen(&serv_conf.default_page[0])+1);

	strcpy(tmp,&serv_conf.default_page[0]);
	ptr=strtok(tmp,",");
	if(ptr!=NULL){
	 sprintf(fi, "%s/%s%s%s", &serv_conf.workdir[0], &request->virtual_path[0], &request->path[0], ptr);
	  if(file_exists(fi)) {
	   strcpy(&request->file[0],ptr);
	   found=1;
	   goto ret; // Yes i'm using goto - so sue me!
	  }
	}
	while((ptr=strtok(NULL,","))!=NULL){
	 sprintf(fi, "%s/%s%s%s", &serv_conf.workdir[0], &request->virtual_path[0], &request->path[0],ptr);
	  if(file_exists(fi)) {
	   strcpy(&request->file[0],ptr);
	   found=1;
	   goto ret;
	  }
	}

 ret:
	free(fi);
	free(tmp);

 return found;
}

char* get_content_type(char* file, char* ct){

	char* ptr;
	char ext[64];
	int n = 0;
	int all_ok=0;

	if((ptr=strstr(file,"."))!=NULL){
	 strcpy(&ext[0],ptr);
	}

	while(1){
	 if(serv_conf.content_types[n]==NULL) break;
	  if(strcmp(&serv_conf.content_types[n]->file_ext[0], &ext[0])==0) {
 	    strcpy(&ct[0], &serv_conf.content_types[n]->content_type[0]);
	    all_ok=1;
	    break;
	  }
	 n++;
	}

	if(!all_ok) {
	  strcpy(ct,"text/text");
	}

	// TODO: Bad fix for type="module"
	if(ends_with(file,".js")==0){
	  strcpy(ct,"text/javascript");
	}

 return ct;
}

char* get_head(http_response* response, char* head, char* code){

	char* tmp = (char*)malloc(1024);
	memset(head,0,1024);
	sprintf(tmp,"%s %s\n", response->request->httpv, code);
	strcat(head,tmp);
	sprintf(tmp,"Server: %s\n", &serv_conf.server_name[0]);
	strcat(head,tmp);
 	if(strlen(ACAOrigin)>0){
	  sprintf(tmp,"%s", ACAOrigin);
	  strcat(head,tmp);
	}
	sprintf(tmp,"%s: %s\n", "Content-Type", &response->content_type[0]);
	strcat(head,tmp);
	sprintf(tmp,"%s: %d\n", "Content-Length", response->content_length);
	strcat(head,tmp);
	sprintf(tmp,"%s: %s\n", "Connection", &response->request->connection[0]);
	strcat(head,tmp);
	free(tmp);

 return head;
}

int exec_cgi(http_response* response, const char* exe_ptr){

        int n = 0;
        int r = 0;
	char* argv[128];

        char *buffer;
        char *headb;
        char *file_path = (char*)malloc(MAX_ALLOC);
	char *executable = (char*)malloc(1024);
	int  clen=0;

        int pipefd[2];
        int pin[2];
	int pid;
	int abort=0;

        n=pipe(pipefd);
        n=pipe(pin);
        n=0;

        sprintf(file_path,"%s%s%s",
		&response->request->virtual_path[0],&response->request->path[0],&response->request->file[0]);

	if(strstr(exe_ptr,"jgazm")!=NULL){
	  strcpy(executable, exe_ptr);
	  argv[0]=(char*)malloc(strlen(file_path)+1);
 	  strcpy(argv[0],"-f");
	  argv[1]=(char*)malloc(strlen(file_path)+1);
 	  strcpy(argv[1],file_path);
	  argv[2]=NULL;
	}else if(strcmp(exe_ptr,"[shell]")!=0){
	  strcpy(executable, exe_ptr);
	  argv[0]=(char*)malloc(strlen(file_path)+1);
 	  strcpy(argv[0],file_path);
	  argv[1]=NULL;
	}else{
	  strcpy(executable, file_path);
	  argv[0]=(char*)malloc(strlen(&file_path[0]));
 	  strcpy(argv[0],&file_path[0]);
	  argv[1]=NULL;
	}

        if ((pid=fork()) == 0){
          close(pipefd[0]);
          dup2(pin[0], 0);
          dup2(pipefd[1], 1);
          dup2(pipefd[1], 2);
          close(pipefd[1]);
          if(execve(executable, argv, response->envp)==-1){
	   fprintf(stderr,"%s\n", executable);
	   abort=1;
	  }
        }else if(pid==-1){
	  perror("Bad fork() in exec_cgi\n");
	}
	else{
	  buffer = (char*)malloc(1024);
	  headb = (char*)malloc(1024);
          close(pipefd[1]);
	  if(!abort){
            if(strcmp(&response->request->method[0],HTTP_POST)==0 && response->request->post_data!=NULL) {
	      clen = response->content_length;
	      r=write(pin[1], response->request->post_data, clen);
	    }
            sprintf(headb,"HTTP/1.1 200 OK\nConnection: %s\n", &response->request->connection[0]);
	    n=socket_write(response->request, headb, strlen(headb));
            while((r=read(pipefd[0], buffer, 1024))){
              n=socket_write(response->request, buffer, r);
	    }
	  }

	  (void)(r);
	  close(pipefd[0]);
	  close(pin[0]);
	  close(pin[1]);
          free(buffer);
          free(headb);
	}

        free(file_path);
        free(executable);
        n = 0;
        while(1){
	 if(argv[n]==NULL) break;
	   free(argv[n++]);
        }

	if(abort) send_internal_error(response);

 return n;
}

int write_plain_file(const http_response* response, int len, char*path, char* fil){


	FILE *fd;
	int r=0;
	char *file;

	char* tmp = (char*)malloc(2048+1);
	if(tmp==NULL) {
	 fprintf(stderr,"Can't malloc for %s", fil);
	 return -1;
	}

	file = (char*)malloc(2048);
	memset(file,0,2048);
	memset(tmp,0,2048+1);

	sprintf(file,"%s%s%s", &response->request->virtual_path[0], path, fil);

	int n = 0;
	if((fd=fopen(file,"rb"))!=NULL){
	  while((r=fread(tmp, 1, 2048, fd))>0){
	    socket_write(response->request, tmp, r);
	    n+=r;
	  }
	  if(n!=len) fprintf(stderr,"Bytes read: %d are less than content-length: %d\n", n, len);
	  fclose(fd);
	}else{
	  fprintf(stderr,"Bad file: %s%s\n", path, file);
	  free(file);
	  free(tmp);
	  return -1;
	}

	free(tmp);
	free(file);

 return 0;
}

char* getExecutable(const char* file){

	int n = 0;
	while(1){
	  if(serv_conf.exec_c[n]==NULL) break;
	  if( strstr( file,&serv_conf.exec_c[n]->ext[0]) !=NULL){
	   return &serv_conf.exec_c[n]->exec[0];
	  }
	  n++;
	}

 return NULL;
}

char* get_user(const char* basic, char* buff){

	char* ptr;
	if((ptr=strstr(basic,":"))!=NULL){
	  int i=ptr-basic;
	  strcpy(buff, basic);
	  buff[i] = '\0';
	}

  return buff;
}

char* get_passwd(const char* basic){
	return strstr(basic,":")+1;
}

int get_user_pass_from_file(const char* file, const char* base64){

	FILE* fd;
	int ret=0;
	char *buffer = (char*)malloc(128);
	if((fd=fopen(file,"r"))!=NULL){
	 while((fgets(buffer, 128, fd))!=NULL){
	  if(strcmp(clip(buffer), base64)==0){
	   ret=1;
	  }
 	 }
	 fclose(fd);
	}
	free(buffer);

 return ret;
}

int handle_auth(http_request* request){

	 int len;
	 char* tmp = (char*)malloc(2024);
	 char* cmp = (char*)malloc(1024);
	 int handled=AUTH_OK;
	 char* basic;
	 int n = 0;
	 char* decode;
	 char* user;
	 char* passwd;

	while(1){
	 if(serv_conf.auth[n]==NULL) break;
	 if(!startsw(&request->path[0], &serv_conf.auth[n]->path[0])){
	  if((basic=get_header(request,AUTHORIZATION))!=NULL){
		decode=(char*)base64_decode((unsigned char*)basic+6, strlen(basic+6), &len);
	 	decode[len]='\0';
		user=get_user(decode, tmp);
		passwd=get_passwd(decode);
		sprintf(cmp, "%s:%s", user, crypt(passwd,SALT));
		if((strstr(&serv_conf.auth[n]->base64auth[0],".passwd"))!=NULL){
		  sprintf(tmp,"%s/conf/%s", &serv_conf.workdir[0], &serv_conf.auth[n]->base64auth[0]);
	          if(get_user_pass_from_file(tmp, cmp)){
		    free(cmp);
		    free(tmp);
		   return AUTH_OK;
		}
		}else{
		  if(strcmp(&serv_conf.auth[n]->base64auth[0],cmp)==0){
		    free(cmp);
		    free(tmp);
		   return AUTH_OK;
		}
	      }
	    }
	    strcpy(tmp,"HTTP/1.1 401 Unauthorized\r\n");
	    socket_write(request,tmp,strlen(tmp));

	    strcpy(tmp,"Connection: close\r\n");
	    socket_write(request,tmp,strlen(tmp));

	    sprintf(tmp,"WWW-Authenticate: Basic realm=%s\r\n\r\n",&serv_conf.auth[n]->realm[0]);
	    socket_write(request,tmp,strlen(tmp));

	    socket_write(request,unauthorized,strlen(unauthorized));

	    handled=AUTH_REQUEST_SENT;
	  }
  	  n++;
	}

	free(cmp);
	free(tmp);

 return handled;
}


void doPut(http_request* request){
	printf("PUT %s\n", request->post_data);
	(void)(request);
}
/*
void doHead(http_request* request){

	if(c_debug) printf("[do head]\n");
	(void)(request);

}
*/

void doGetPost(http_request *request){

	char tmp[1024];
	char ext[128];
	int auth_mode=-1;
	char* exe_ptr;
	http_response response;
	memset(&response,0,sizeof(http_response));
	response.envp[0]=NULL;
	response.request=request;

	if(c_debug) printf("[doGetPost]\n");
	if(strcmp(&request->method[0],HTTP_OPTIONS)==0){
	 if(c_debug) printf("[send_options_reply]\n");
	 send_options_reply(request);
	 return;
	}

	auth_mode=handle_auth(request);
	switch(auth_mode){
	 case AUTH_REQUEST_SENT:
	  return;
	 case BAD_AUTH:
	  //send_bad_auth(request);
	  return;
	}

  	response.content_length = get_file_size(request);
	strcpy(&response.content_type[0],get_content_type(&request->file[0], &ext[0]));

	//dump_request(request);

	if(c_debug) printf("[content-length read]\n");

	if(response.content_length<1){
	  send_bad_request(&response, D_404_NOT_FOUND);
	}else{
	  parse_env(&response);
	  if(strcmp(&response.request->method[0],HTTP_PUT)==0){
	   doPut(request);
	  }/*else if(strcmp(&response.request->method[0],HTTP_HEAD)==0){
	   doHead(request);
	  }*/
	  else if((exe_ptr=getExecutable(&request->file[0])) || strcmp(&response.request->method[0],HTTP_POST)==0){
	    exec_cgi(&response, exe_ptr);
	  }else{
	    get_head(&response,&tmp[0], D_200_OK);
	    socket_write(request,&tmp[0],strlen(&tmp[0]));
	    socket_write(request,"\r\n",2);
	    if(write_plain_file(&response, response.content_length,
	        &request->path[0], &request->file[0])==-1){
	      send_bad_request(&response, D_404_NOT_FOUND);
	    }
	  }
	}
	free_response(&response);

}

int handle_virtual_files(http_request* request){

	char* virt;
	int n = 0;
	int res=0;
	char* tmp;
	char* origins = (char*)malloc(4096);

	while(1){
	  if(serv_conf.v_files[n]!=NULL){
	    virt = (char*)malloc(strlen(&request->path[0])+strlen(&request->file[0])+2);
	    sprintf(virt,"%s%s",&request->path[0],&request->file[0]);
	    if(strcmp(virt,&serv_conf.v_files[n]->name[0])==0){
	      sprintf(&request->path[0],"%s",&serv_conf.v_files[n]->path[0]);
	      sprintf(&request->file[0],"%s",&serv_conf.v_files[n]->file[0]);
	    }
	   free(virt);
	  }else break;
	 n++;
	}

	if(c_debug) printf("[start uep %d]\n", uep==NULL);
	tmp = (char*)malloc(strlen(&request->path[0])+strlen(&request->file[0])+2);
	sprintf(tmp,"%s%s", &request->path[0],&request->file[0]);
	if(uep!=NULL && strcmp(tmp,uep->endpoint)==0){
	  if(c_debug) printf("[start uep]\n");
	  printf("eup:%s %s\n", tmp, uep->endpoint);
	  socket_write(request,"HTTP/1.1 200 OK\n",16);
	  socket_write(request,"Server: Cornelia\n",17);
	  socket_write(request,"Connection: close\n",18);

	  if(strlen(ACAOrigin)>0){
	  	sprintf(origins,"%s", ACAOrigin);
		socket_write(request,origins,(int)strlen(origins));
	  }

	  tmp = (char*)realloc(tmp,255);
	  if(uep->content_type!=NULL){
	   sprintf(tmp,"Content-Type: %s\n", uep->content_type);
	  }else sprintf(tmp,"Content-Type: application/json\n");
	  socket_write(request, tmp, strlen(tmp));

	  if(uep->response!=NULL) {
	    tmp = realloc(tmp,128);
	    sprintf(tmp,"Content-Length: %d\n\n", (int)strlen(uep->response));
	    socket_write(request, tmp, strlen(tmp));
	    tmp = realloc(tmp,strlen(uep->response)+2);
 	    sprintf(tmp,"%s", uep->response);
	    socket_write(request, tmp, strlen(tmp));
	  }
 	 res=1;
	 if(c_debug) printf("[end uep]\n");
	}
	free(origins);
  	free(tmp);

 return res;
}

int parse_http(char* buffer, http_request* request){

        char* ptr;
	int res = 0;

	if(c_debug) printf("[parse_http]\n");

	ptr=strtok(&buffer[0]," ");
	if(ptr==NULL) return -1;
	strcpy(&request->method[0],ptr);


	ptr=strtok(NULL," ");
	if(ptr==NULL) return -1;
	split(ptr, &request->path[0], &request->file[0], &request->query_string[0], MAX_QUERYSTRING);

	ptr=strtok(NULL," ");
	if(ptr==NULL) return -1;
	strcpy(&request->httpv[0],ptr);

	if(strcmp(&request->path[0], CGI_BIN)==0){
	  strcpy(&request->path[0], &serv_conf.cgi_bin[0]);
	}

	if(c_debug) printf("[start virtual_files]%s %s\n", &request->path[0], &request->file[0]);
	res = handle_virtual_files(request);
	if(c_debug) printf("[end virtual_files]%s %s\n", &request->path[0], &request->file[0]);

	if(strlen(&request->file[0])==0) {
	    return -1;
	}

 return res;
}

void dump_request(http_request* r){

	printf("'%s' '%s' '%s' '%s'\n", &r->method[0], &r->path[0], &r->file[0], &r->httpv[0]);
	int n = 0;
	printf("query_string:%s\n", &r->query_string[0]);
	while(1){
	 if(r->headers[n]==NULL) break;
	 printf("%s\n", r->headers[n++]);
	}
}

char* get_header(const http_request *request, const char* header){

	int n=0;
	char *ptr;

	if(c_debug) printf("[get_header]\n");
	while(1){
	 if(request->headers[n]==NULL) break;
	 if(!startsw(request->headers[n], header)){
	   ptr = strstr(request->headers[n],"=");
	   if(ptr!=NULL) return ptr+1;
	   else return NULL;
	 }
  	 n++;
	}

	if(c_debug) printf("[exit get_header]\n");

 return NULL;
}

void parse_headers(char* buffer, http_request* request){

	char* ptr;
	int  index;
	char* name=(char*)malloc(128);
	char* value=(char*)malloc(1024);
	char* tmp=(char*)malloc(MAX_ALLOC);

	if((ptr=strstr(buffer,":"))!=NULL){
	  index=ptr-buffer;
	  buffer[index]='\0';
	  strcpy(name,buffer);
	  strcpy(value,ptr+2);
	  sprintf(tmp,"%s=%s%c", name,value,'\0');
	  request->headers[request->headers_len]=(char*)malloc(strlen(tmp)+2);
	  memset(request->headers[request->headers_len],0, strlen(tmp)+2);
	  strcpy(request->headers[request->headers_len++], tmp);
	}
	request->headers[request->headers_len]=NULL;

	free(tmp);
	free(name);
	free(value);
}

void parse_env(http_response* res){

	int n = 0;
	char *tmp;
	char buff[1024];
	char cpy[1024];

	tmp = (char*)malloc(6048);

	while(1){
	  memset(&buff[0],0,1024);
	  memset(&cpy[0],0,1024);
	  if(res->request->headers[n]==NULL) break;
	  strcpy(tmp, res->request->headers[n]);
	  res->envp[n]=(char*)malloc(strlen(tmp)+6);
	  sprintf(res->envp[n],"HTTP_%s", toupperc(&cpy[0],tmp,'='));
	  if(n>=MAX_ENV_VARS-10) break;
	  n++;
	}

	if(strcmp(&res->request->method[0],"POST")==0 && res->request->post_data!=NULL){
	  res->content_length=atoi(get_header(res->request,"Content-Length="));
	  sprintf(tmp,"CONTENT_LENGTH=%d", res->content_length);
	  res->envp[n] = (char*)malloc(strlen(tmp)+1);
	  strcpy(res->envp[n], tmp);
	  n++;

	  sprintf(tmp,"CONTENT_TYPE=%s", get_header(res->request,"Content-Type="));
	  res->envp[n] = (char*)malloc(strlen(tmp)+1);
	  strcpy(res->envp[n], tmp);
	  strcpy(&res->content_type[0], get_header(res->request,"Content-Type="));
	  n++;
	}

	sprintf(tmp,"QUERY_STRING=%s",&res->request->query_string[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	sprintf(tmp,"REQUEST_METHOD=%s",&res->request->method[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	sprintf(tmp,"REQUEST_URI=%s%s\?%s",&res->request->path[0],&res->request->file[0],&res->request->query_string[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	strcpy(tmp,"REDIRECT_STATUS=200");
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	sprintf(tmp,"SCRIPT_FILENAME=%s%s%s", &res->request->virtual_path[0], &res->request->path[0],&res->request->file[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	sprintf(tmp,"SCRIPT_NAME=%s", &res->request->file[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	sprintf(tmp,"SERVER_NAME=%s", &serv_conf.server_name[0]);
	res->envp[n] = (char*)malloc(strlen(tmp)+1);
	strcpy(res->envp[n], tmp);
	n++;

	free(tmp);
}

void read_post_data(http_request *request, unsigned int len){

	int r = 0;
	unsigned int n = 0;
	unsigned int m_len = len<serv_conf.max_post_data?len:serv_conf.max_post_data;
	char buff[2];

	if(len>serv_conf.max_post_data){
	  send_forbidden(request);
	  return;
	}

	request->post_data = malloc(m_len);
	memset(request->post_data, 0, m_len);
	while((r=socket_read(request, &buff[0], 1))){
	  request->post_data[n++]=buff[0];
	  if(n>=m_len) break;
	}
}

virtual_host* get_virtual_host(char* host){

	int n = 0;
	char* tmp = (char*)malloc(256);
	char* htmp = (char*)malloc(256);
	virtual_host* h_ptr = NULL;
	char *ptr;

	strcpy(htmp, host);
	if((ptr=strstr(htmp,":"))!=NULL){
	 htmp[ptr-htmp]='\0';
	}

	while(1){
	  if(serv_conf.v_hosts[n]==NULL) break;
	  sprintf(tmp,"%s", &serv_conf.v_hosts[n]->name[0]);
	  if(strcmp(tmp,htmp)==0){
	    h_ptr=serv_conf.v_hosts[n];
	    break;
	  }
	 n++;
	}
	free(htmp);
	free(tmp);

 return h_ptr;
}



void handle_request(SOCKET sockfd, char* clientIP, void* cSSL){

	int r=CONN_CLOSE;

	if(c_debug) printf("[handle_request]\n");

	for(int i = 0; i < serv_conf.max_keep_alive_requests; i++){
	  r = exec_request(sockfd, clientIP, cSSL);
	  if(r!=CONN_KEEP_ALIVE) {
	   return;
	  }
	}
	if(c_debug) printf("[exit handle_request]\n");

}

int exec_request(SOCKET sockfd, char* clientIP, void* cSSL){

	int r=0;
	char* buffer = (char*)malloc(2048);
	char* tmp = (char*)malloc(4096);
	int n = 0;
	char* ptr;
	char* host;
	int ret = CONN_CLOSE;
	FILE* logfd;

	http_request request;
	memset(&request,0,sizeof(http_request));

	request.sockfd = sockfd;
	request.cSSL = cSSL;

	memset(buffer,0,2048);

	r=readline(&request, buffer, 2048);
	if(r<1) return CONN_CLOSE;

	if(c_debug) printf("[readline]\n");

	if(strlen(&serv_conf.logfile[0])>0){
	  sprintf(tmp, "%s/%s", &serv_conf.workdir[0], &serv_conf.logfile[0]);
	  if((logfd=fopen(tmp,"a"))!=NULL){
	    fprintf(logfd,"%s|%s|%d|%s\n", buffer, clientIP, serv_conf.port, clip(get_date_time(tmp)));
	    fclose(logfd);
	  }
	}

	strcpy(&request.virtual_path[0],&serv_conf.www_root[0]);
	memset(tmp,0,4048);

	int parse_h = parse_http(buffer,&request);

	if(parse_h==1) {
         free(buffer);
         free(tmp);
	 return ret;
	}

	if(c_debug) printf("[exit parse_http]\n");

	while((r=readline(&request,buffer,1024))>0){
	 if(request.headers_len>=MAX_HTTP_HEADERS) {
		fprintf(stderr,"Header count greater than %d\n", MAX_HTTP_HEADERS);
		break;
	 }
	 parse_headers(buffer,&request);
	 n++;
	}

	if(c_debug) printf("[exit read_headers]\n");

	// Set virtual path if Host header matches.
	if((host=get_header(&request,"Host="))!=NULL){
	  virtual_host* h = get_virtual_host(host);
	  if(h!=NULL){
	   // Setting virtual_path.
	   strcpy(&request.virtual_path[0], &h->path[0]);
	  }else{
	   // No virtual path defaulting to www_root.
	   strcpy(&request.virtual_path[0], &serv_conf.www_root[0]);
	  }
	}

	if(c_debug) printf("exit [read virtual_path]\n");

	if(parse_h<0 && find_default_page(&request)==0){
	 if(c_debug) printf("[no default page]\n");
	  if((strcmp(&serv_conf.allow_dir_listing[0],"yes"))==0){
	    send_list_dir(&request);
	  }else {
	    send_forbidden(&request);
	  }
	  close(sockfd);
	  free(buffer);
	  free(tmp);
	 //return CONN_CLOSE;
	}
	if(c_debug) printf("[exit default page]\n");

	if(get_header(&request, "Connection=")!=NULL){
		strcpy(&request.connection[0],get_header(&request, "Connection="));
	}else{
		strcpy(&request.connection[0],"Close");
	}

	if(c_debug) printf("[exit connection]\n");

	if(strcmp(&request.method[0],HTTP_POST)==0 || strcmp(&request.method[0],HTTP_PUT)==0){
	 if((ptr = get_header(&request,"Content-Length="))!=NULL){
	  read_post_data(&request, atoi(ptr));
	 }else request.post_data=NULL;
	}

	if(c_debug) printf("exit [content-length]\n");

	request.headers_len=n-1;
	doGetPost(&request);
	if(c_debug) printf("exit [dogetpost]\n");

	if(strcmp(&request.connection[0],"keep-alive")==0) ret = CONN_KEEP_ALIVE;

	(void)(r);
	free_request(&request);
	free(buffer);
	free(tmp);

 return ret;
}

void free_request(http_request* r){

	int n = 0;
	while(1){
	 if(r->headers[n]==NULL) break;
	 free(r->headers[n++]);
	}
	if(r->post_data!=NULL) free(r->post_data);
}

void free_response(http_response* r){

	int n = 0;
	while(1){
	 if(r->envp[n]==NULL) break;
	 free(r->envp[n++]);
	}
}

int read_http_responses(){

  	int len;
  	int r;
  	FILE* fd;
	char* file = (char*)malloc(1024);
	char* buffer = (char*)malloc(1024);

	sprintf(file,"%s/conf/404.txt", getenv("CORNELIA_HOME"));
  	if((fd=fopen(file,"r"))!=NULL){
   	  fseek(fd,0L,SEEK_END);
   	  len = ftell(fd);
   	  fseek(fd,0L,SEEK_SET);
   	  bad_request = (char*)malloc(len);
   	  r=fread(bad_request,1,len,fd);
   	  (void)(r);
   	  fclose(fd);
  	}else {
	  printf("Bad file: missing conf/404.txt\n");
	  fprintf(stderr,"Bad file: missing conf/404.txt\n");
	  return -1;
	}

	sprintf(file,"%s/conf/500.txt", getenv("CORNELIA_HOME"));
        if((fd=fopen(file,"r"))!=NULL){
          fseek(fd,0L,SEEK_END);
          len = ftell(fd);
          fseek(fd,0L,SEEK_SET);
          internal_server_error = (char*)malloc(len);
          r=fread(internal_server_error,1,len,fd);
          (void)(r);
          fclose(fd);
        }else{
	  fprintf(stderr,"Bad file: missing conf/500.txt\n");
	  printf("Bad file: missing conf/500.txt\n");
	  return -1;
	}

       sprintf(file,"%s/conf/403.txt", getenv("CORNELIA_HOME"));
       if((fd=fopen(file,"r"))!=NULL){
          fseek(fd,0L,SEEK_END);
          len = ftell(fd);
          fseek(fd,0L,SEEK_SET);
          forbidden = (char*)malloc(len);
          r=fread(forbidden,1,len,fd);
          (void)(r);
          fclose(fd);
        }else{
          fprintf(stderr,"Bad file: missing conf/403.txt\n");
          printf("Bad file: missing conf/403.txt\n");
          return -1;
        }

       sprintf(file,"%s/conf/401.txt", getenv("CORNELIA_HOME"));
       if((fd=fopen(file,"r"))!=NULL){
          fseek(fd,0L,SEEK_END);
          len = ftell(fd);
          fseek(fd,0L,SEEK_SET);
          unauthorized = (char*)malloc(len);
          r=fread(unauthorized,1,len,fd);
          (void)(r);
          fclose(fd);
        }else{
          fprintf(stderr,"Bad file: missing conf/401.txt\n");
          printf("Bad file: missing conf/401.txt\n");
          return -1;
        }


       sprintf(file,"%s/conf/http_options.txt", getenv("CORNELIA_HOME"));
       if((fd=fopen(file,"r"))!=NULL){
          fseek(fd,0L,SEEK_END);
          len = ftell(fd);
          fseek(fd,0L,SEEK_SET);
          http_options = (char*)malloc(len);
          r=fread(http_options,1,len,fd);
          (void)(r);
          fclose(fd);
        }else{
          fprintf(stderr,"Bad file: missing conf/http_options.txt\n");
          printf("Bad file: missing conf/http_options.txt\n");
          return -1;
        }


       sprintf(file,"%s/conf/Access-Control-Allow.txt", getenv("CORNELIA_HOME"));
        ACAOrigin = (char*)malloc(4096);
	memset(ACAOrigin,0,4096);
       if((fd=fopen(file,"r"))!=NULL){
	 while((fgets(buffer,1024,fd))!=NULL){
	   if(strstr(buffer,"#")!=NULL) continue;
	   strcat(ACAOrigin,buffer);
	 }
	 clipend(ACAOrigin);
	 if(ACAOrigin[strlen(ACAOrigin)-1]!='\n') strcat(ACAOrigin,"\n");
  	 fclose(fd);
        }else{
          fprintf(stderr,"Bad file: missing conf/Access-Control-Allow\n");
          printf("Bad file: missing conf/Access-Control-Allow-Origin\n");
          return -1;
        }

	free(buffer);
	free(file);

 return 0;
}

char* encode_url(unsigned char* url, char* url_enc){

	char rfc3986[256] = {0};
	char html5[256] = {0};

	url_encoder_rfc_tables_init(&html5[0], &rfc3986[0], 256);
	url_encode( &html5[0], url, url_enc);

 return url_enc;
}

int setenv(const char *name, const char *value, int overwrite);

void usage(){

	printf("\nCornelia Web Server (c) CrazedoutSoft 2022\n\n");
	printf("usage:\tcornelia_d [OPTION]\n");
	printf("example:cornelia -c myconf.conf -p 8080\n\n");
	printf("-http\tHTTP (Default)\n");
	printf("-ssl\tHTTP/SSL\n");
	printf("-tls\tHTTP/TLS\n");
	printf("-c\t<conf_file>\n");
	printf("-p\t<server_port>\n");
	printf("-ssl\t<server_ssl_port>\n");
	printf("-tsl\t<server_tsl_port>\n");
	printf("-i \tprints config\n");
	printf("-uep\tset up endpoint [-uep:/myendpoint%%{\"my\":\"content\"}%%application/json\n");
	printf("                       [-uep:/myendpoint%%file:myjson.js%%application/json\n");
	printf("\tContent-Type can default to 'application/json' if omitted.\n");
	printf("--help prints this message\n\n");

}

void check_conf(int use_ssl, int use_tls){


	if(serv_conf.port==0){
	  printf("Warning: http port missing - defaulting to 8080\n");
	  serv_conf.port=8080;
	}
	if(use_ssl && serv_conf.ssl_port==0){
	  printf("Warning: ssl port missing - defaulting to 8081\n");
	  serv_conf.ssl_port=8081;
	}
	if(use_tls && serv_conf.tls_port==0){
	  printf("Warning: tls port missing - defaulting to 8082\n");
	  serv_conf.tls_port=8081;
	}
	if(strlen(&serv_conf.www_root[0])==0){
	  printf("Warning: www root missing - defaulting to 'www'\n");
	  strcpy(&serv_conf.www_root[0],"www");
	}
	if(serv_conf.keep_alive_timeout==0){
	  printf("Warning: keep-alive timout missing - defaulting to 500000\n");
	  serv_conf.tls_port=8081;
	}

}


user_endpoint* get_user_endpoint(char* argstr){

  int n = 0;
  char* token;
  char* ptr;
  char* arg = (char*)malloc(strlen(argstr)+1);
  FILE* fd;
  int r=0;
  long fd_len=0;
  strcpy(arg, argstr);

  user_endpoint* uep = (user_endpoint*)malloc(sizeof(user_endpoint));
  memset(uep,0,sizeof(user_endpoint));

  token = strtok(arg, ":");

   while((token = strtok(NULL, "%"))!=NULL){
    if(n==0){
      uep->endpoint = (char*)malloc(strlen(token));
      strcpy(uep->endpoint, token);
    }else if(n==1){
      if((ptr=strstr(token,"file:"))!=NULL){
	if((fd=fopen(ptr+5,"r"))!=NULL){
	  fseek(fd,0,SEEK_END);
	  fd_len=ftell(fd);
	  fseek(fd,0,SEEK_SET);
	  ptr = (char*)malloc(fd_len+1);
	  r=fread(ptr,fd_len,1,fd);
	  (void)(r);
	  fclose(fd);
	  uep->response = (char*)malloc(strlen(ptr));
	  strcpy(uep->response, ptr);
	  free(ptr);
	}else fprintf(stderr,"Bad uep file\n");
      }else {
        uep->response = (char*)malloc(strlen(token));
	strcpy(uep->response, token);
      }
    }else if(n==2){
      uep->content_type = (char*)malloc(strlen(token));
      strcpy(uep->content_type, token);
    }
    n++;
   }

  if(uep->endpoint==NULL || uep->response==NULL){
        printf("User enpoint must have at least 'enpoint' and 'response' defined.\n");
        printf("Format: -uep:/myendpoint\%%{\"name\":\"value\"}\%%application/json\n");
        printf("Format: -uep:/myendpoint\%%file:myjson.js\%%application/json\n");
        printf("Supplied: -uep:%s %s %s\n", uep->endpoint, uep->response, uep->content_type);
        printf("No correct eup defined\n");
  }else{
	printf("Listening on endpoint:%s, reply %s\n", uep->endpoint, uep->content_type!=NULL?uep->content_type:"application/json");
  }

  free(arg);

 return uep;
}


int main(int args, char* argv[]){

	int  user_port = 0;
	int  user_ssl_port=0;
	int  user_tsl_port=0;
	int  dump_c=0;
	char *ptr;
	char* dir = (char*)malloc(1024);
	int use_ssl=0;
	int use_tls=0;

	get_work_dir(dir,1024);

	if(getenv("CORNELIA_HOME")==NULL){
	  printf("env CORNELIA_HOME should be set to cornelia_d workdir\n");
	  printf("export CORNELIA_HOME=<dir of cornelia_d>\n");
	  printf("Asuming: %s - Lets's try it..\n", dir);
	  setenv("CORNELIA_HOME",dir,1);
	}
	free(dir);

	if(args>1){

	 for(int i = 0; i < args; i++){
	  if(strcmp(argv[i],"-c")==0) {
		if(i+1>=args){
		  printf("Bad conf file:<null>\nTry --help\n");
		  return -1;
		}
		strcpy(&conf_file[0], argv[i+1]);
	  }
	  else if(strcmp(argv[i],"-p")==0 && i<args-1 && i+1<args) {
		user_port=atoi((ptr=argv[i+1]));
		if(user_port==0) {
		 printf("Bad port:%s\nTry --help\n", ptr);
		 return -1;
		}
	  }
	  else if(strcmp(argv[i],"-ssl_p")==0 && i<args-1 && i+1<args) {
                user_ssl_port=atoi((ptr=argv[i+1]));
                if(user_ssl_port==0) {
                 printf("Bad ssl port:%s\nTry --help\n", ptr);
                 return -1;
                }
          }
	  else if(strcmp(argv[i],"-tsl_p")==0 && i<args-1 && i+1<args) {
                user_tsl_port=atoi((ptr=argv[i+1]));
                if(user_tsl_port==0) {
                 printf("Bad ssl port:%s\nTry --help\n", ptr);
                 return -1;
                }
          }
	  else if(strcmp(argv[i],"-ssl")==0) use_ssl=1;
	  else if(strcmp(argv[i],"-tls")==0) use_tls=1;
	  else if(strcmp(argv[i],"-i")==0) dump_c=1;
	  else if(strcmp(argv[i],"-d")==0) c_debug=1;
	  else if(strstr(argv[i],"-uep")!=NULL){
	    uep = get_user_endpoint(strstr(argv[i],"-uep"));
	  }
	  else if(strcmp(argv[i],"--help")==0) {
		usage();
		return 0;
	  }
	 }
	}

	if(read_http_responses()==-1) return -1;

	memset(&serv_conf,0,sizeof(server_conf));
	memset(&a_conf,0,sizeof(auth_conf));

	if(init_conf(&conf_file[0], &serv_conf)>-1){
  	  if(user_port>0) serv_conf.port=user_port;
  	  if(user_ssl_port>0) serv_conf.ssl_port=user_ssl_port;
  	  if(user_tsl_port>0) serv_conf.tls_port=user_tsl_port;
	  if(dump_c) {print_server_conf(&serv_conf);}
	  else {
	   check_conf(use_ssl, use_tls);
	   if(use_ssl==0 && use_tls==0){
		init_server();
	   }else if(use_ssl){
		#ifndef NO_SSL
		init_ssl_server(&serv_conf);
		#endif
		#ifdef NO_SSL
		printf("This Cornelia Web Server was compiled without SSL.(make no_ssl)\n");
		#endif
	   }else if(use_tls){
		#ifndef NO_SSL
		init_tls_server(&serv_conf);
		#endif
		#ifdef NO_SSL
		printf("This Cornelia Web Server was compiled without TLS. (make no_ssl)\n");
		#endif
	   }
          }
	}

	free(bad_request);
	free(internal_server_error);
	free(forbidden);
	free(unauthorized);

 return 0;
}
