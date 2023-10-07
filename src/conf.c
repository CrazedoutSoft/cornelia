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

#include "../include/webs.h"
#include "../include/misc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_CONF 		"[server_conf]"
#define AUTH_CONF   		"[auth_conf]"
#define CONTENT_TYPE_CONF   	"[content_type_conf]"
#define CGI_BIN_CONF		"[cgi_exec_conf]"
#define VHOSTS_CONF		"[virtual_hosts]"
#define VHOSTS_FILES		"[virtual_files]"

void print_server_conf(server_conf* serv){

	int n = 0;

	printf("[server_conf]\n");
	printf("server.name=%s\n", &serv->server_name[0]);
	printf("server.port=%d\n", serv->port);
	printf("server.ssl.port=%d\n", serv->ssl_port);
	printf("server.tls.port=%d\n", serv->tls_port);
	printf("server.ssl.certpm=%s\n", &serv->cert[0]);
	printf("server.ssl.keypm=%s\n", &serv->cert_key[0]);
	printf("server.tls.certcrt=%s\n", &serv->certcrt[0]);
	printf("server.tls.keycrt=%s\n", &serv->keycrt[0]);
	printf("server.logfile=%s\n", &serv->logfile[0]);
	printf("server.www_root=%s\n", &serv->www_root[0]);
	printf("server.default_page=%s\n", &serv->default_page[0]);
	printf("#cgi-bin value must be relative too www_root and start with /\n");
	printf("#bug in this release do not change cgi-bin value.\n");
	printf("server.cgi-bin=%s\n", &serv->cgi_bin[0]);
	printf("server.allow_dir_listning=%s\n", &serv->allow_dir_listing[0]);
	printf("server.max.keep_alive.requests=%d\n", serv->max_keep_alive_requests);
	printf("#microseconds 1 sec=1000000\n");
	printf("#this value should be as short as possible as long it works. try 250000\n");
	printf("server.keep_alive.timeout=%d\n", serv->keep_alive_timeout);
	printf("server.max_post_data=%d\n", serv->max_post_data);
	printf("[server_conf]\n\n");


	printf("[content_type_conf]\n");
        n=0;
        while(1){
         if(serv->content_types[n]==NULL) break;
           printf("%s=%s\n", &serv->content_types[n]->file_ext[0],
                &serv->content_types[n]->content_type[0]);
           n++;
        }
	printf("[content_type_conf]\n\n");
	printf("[cgi_exec_conf]\n");

        n=0;
        while(1){
          if(serv->exec_c[n]==NULL) break;
          printf("%s=%s\n", &serv->exec_c[n]->ext[0], &serv->exec_c[n]->exec[0]);
          n++;
        }
	printf("[cgi_exec_conf]\n\n");

	printf("[auth_conf]\n");

	n=0;
	char realms[1024];
	memset(&realms[0],0,1024);
	while(1){
	  if(serv->auth[n]==NULL) break;
	  strcat(&realms[0],&serv->auth[n]->realm[0]);
	  strcat(&realms[0],",");
	  n++;
	}
	realms[strlen(&realms[0])-1]='\0';
	printf("auth.realms=%s\n", &realms[0]);

	n=0;
	while(1){
	  if(serv->auth[n]==NULL) break;
	  printf("auth.%s.path=%s\n", &serv->auth[n]->realm[0], &serv->auth[n]->path[0]);
	  printf("auth.%s.users=%s\n", &serv->auth[n]->realm[0], &serv->auth[n]->base64auth[0]);
	  n++;
	}


	printf("[auth_conf]\n\n");


	printf("[virtual_hosts]\n");
        n=0;
        while(1){
          if(serv->v_hosts[n]==NULL) break;
          printf("%s/%s\n", &serv->v_hosts[n]->name[0], &serv->v_hosts[n]->path[0]);
          n++;
        }
	printf("[virtual_hosts]\n\n");

        printf("[virtual_files]\n");
        n=0;
        while(1){
          if(serv->v_files[n]==NULL) break;
          printf("%s %s %s\n", &serv->v_files[n]->name[0], &serv->v_files[n]->path[0],&serv->v_files[n]->file[0]);
          n++;
        }
        printf("[virtual_files]\n\n");

}

void read_server_conf(FILE* fd, server_conf* serv){

    char* buffer = (char*)malloc(1024);
    char* chome = (char*)malloc(1024);
    char* ptr;

    strcpy(chome, getenv("CORNELIA_HOME"));

    while((fgets(buffer,1024,fd))!=NULL){

	if(buffer[0]=='#') continue;
	if(strcmp(clip(buffer),SERVER_CONF)==0) break;

	if((ptr=strstr(buffer,"server.www_root="))!=NULL){
	 strcpy(&serv->www_root[0], ptr+16);
	}
	else if((ptr=strstr(buffer,"server.default_page="))!=NULL){
	 strcpy(&serv->default_page[0], ptr+20);
	}
	else if((ptr=strstr(buffer,"server.cgi-bin="))!=NULL){
	 strcpy(&serv->cgi_bin[0], ptr+15);
	}
	else if((ptr=strstr(buffer,"server.port="))!=NULL){
	 serv->port=atoi(ptr+12);
	}
	else if((ptr=strstr(buffer,"server.max.keep_alive.requests="))!=NULL){
	 serv->max_keep_alive_requests=atoi(ptr+31);
	}
	else if((ptr=strstr(buffer,"server.keep_alive.timeout="))!=NULL){
	 serv->keep_alive_timeout=atoi(ptr+26);
	}
	else if((ptr=strstr(buffer,"server.ssl.port="))!=NULL){
	 serv->ssl_port=atoi(ptr+16);
	}
	else if((ptr=strstr(buffer,"server.tls.port="))!=NULL){
	 serv->tls_port=atoi(ptr+16);
	}
	else if((ptr=strstr(buffer,"server.logfile="))!=NULL){
	 strcpy(&serv->logfile[0],ptr+15);
	}
	else if((ptr=strstr(buffer,"server.allow_dir_listning="))!=NULL){
	 strcpy(&serv->allow_dir_listing[0],ptr+26);
	}
	else if((ptr=strstr(buffer,"server.ssl.certpm="))!=NULL){
	 sprintf(&serv->cert[0],"%s/%s", chome, ptr+18);
	}
	else if((ptr=strstr(buffer,"server.ssl.keypm="))!=NULL){
	 sprintf(&serv->cert_key[0],"%s/%s", chome, ptr+17);
	}
	else if((ptr=strstr(buffer, "server.tls.certcrt="))!=NULL){
	 sprintf(&serv->certcrt[0],"%s/%s", chome, ptr+19);
	}
	else if((ptr=strstr(buffer, "server.tls.keycrt="))!=NULL){
	 sprintf(&serv->keycrt[0],"%s/%s", chome, ptr+18);
	}
	else if((ptr=strstr(buffer, "server.name="))!=NULL){
	 sprintf(&serv->server_name[0],"%s", ptr+12);
	}
	else if((ptr=strstr(buffer, "server.max_post_data="))!=NULL){
	 serv->max_post_data=atoi(ptr+21);
	}
    }

    strcpy(&serv->workdir[0], chome);

    free(chome);
    free(buffer);

}

void read_auth_conf(FILE* fd, server_conf *serv){

    char* buffer = (char*)malloc(1024);
    char* ptr,*tptr;
    char* realms = (char*)malloc(256);
    auth_conf* r_arr[256];
    int   r_arr_n = 0;
    int   n = 0;
    char  *tmp = (char*)malloc(256);

    while(fgets(buffer,1024,fd)!=NULL){

	if(buffer[0]=='#' || strlen(buffer)==0) continue;

        if(strcmp(clip(buffer),AUTH_CONF)==0) break;
	if(strlen(buffer)==0) continue;
	else if((ptr=strstr(buffer, "auth.realms="))!=NULL){
	 strcpy(realms,ptr+12);
	 tptr=strtok(realms,",");
	 if(tptr!=NULL){
	   r_arr[r_arr_n]=(auth_conf*)malloc(sizeof(auth_conf));
	   memset(r_arr[r_arr_n],0,sizeof(auth_conf));
 	   strcpy(&r_arr[r_arr_n++]->realm[0],tptr);
	  while((tptr=strtok(NULL,","))){
	    r_arr[r_arr_n]=(auth_conf*)malloc(sizeof(auth_conf));
	    memset(r_arr[r_arr_n],0,sizeof(auth_conf));
 	    strcpy(&r_arr[r_arr_n++]->realm[0],tptr);
	  }
	 }
	 r_arr[r_arr_n]=NULL;
	}

	n=0;
    	while(1){
        if(r_arr[n]==NULL) break;

	 sprintf(tmp,"auth.%s.path=", r_arr[n]->realm);
	 if((ptr=strstr(buffer,tmp))!=NULL){
	  strcpy(&r_arr[n]->path[0], ptr+strlen(tmp));
	 }

	 sprintf(tmp,"auth.%s.users=", r_arr[n]->realm);
	 if((ptr=strstr(buffer,tmp))!=NULL){
	  strcpy(&r_arr[n]->base64auth[0], clip(ptr+strlen(tmp)));
	 }
	n++;
        }
       }

     n=0;
     while(1){
	if(r_arr[n]==NULL) break;
	serv->auth[n]=(auth_conf*)malloc(sizeof(auth_conf));
	memset(serv->auth[n],0,sizeof(auth_conf));
	memcpy(serv->auth[n], r_arr[n], sizeof(auth_conf));
	free(r_arr[n++]);
     }
     serv->auth[n]=NULL;

    free(tmp);
    free(realms);
    free(buffer);
}

void read_content_types(FILE* fd, server_conf *serv){

    char* buffer = (char*)malloc(1024);
    char* ptr;
    int   n=0;

    while(fgets(buffer,1024,fd)!=NULL){

	if(strcmp(clip(buffer),CONTENT_TYPE_CONF)==0) break;
	if(buffer[0]=='#') continue;

	if(buffer[0]=='.'){
	 ptr=strtok(buffer,"=");
	 if(ptr!=NULL){
	  serv->content_types[n]=(content_type*)malloc(sizeof(content_type));
	  memset(serv->content_types[n],0,sizeof(content_type));
	  strcpy(&serv->content_types[n]->file_ext[0], ptr);
	  ptr=strtok(NULL,"=");
	  if(ptr!=NULL){
	    strcpy(&serv->content_types[n]->content_type[0], ptr);
	  }
	  n++;
	 }
	}
    }

    free(buffer);
}

void read_cgi_bin(FILE* fd, server_conf* serv){

    char* buffer = (char*)malloc(1024);
    char* ptr;
    int   n=0;

   while(fgets(buffer,1024,fd)!=NULL){

        if(strcmp(clip(buffer),CGI_BIN_CONF)==0) break;
	if(buffer[0]=='#') continue;

	if(buffer[0]=='.'){
	 ptr=strtok(buffer,"=");
	 if(ptr!=NULL){
	   serv->exec_c[n]=(cgi_exec*)malloc(sizeof(cgi_exec));
	   memset(serv->exec_c[n],0,sizeof(cgi_exec));
	   strcpy(&serv->exec_c[n]->ext[0],ptr);
	   ptr=strtok(NULL,"=");
	    if(ptr!=NULL){
	      strcpy(&serv->exec_c[n]->exec[0],ptr);
	    }
	  n++;
	 }
	 serv->exec_c[n]=NULL;
	}
   }

  free(buffer);

}

void read_vhosts(FILE* fd, server_conf* serv){

	int n = 0;
	char* buffer = (char*)malloc(1024);
	char* mem = (char*)malloc(1024);
	char* tmp = (char*)malloc(256);
	char* ptr;

   	while(fgets(buffer,1024,fd)!=NULL){

	  if(strcmp(clip(buffer),VHOSTS_CONF)==0) break;
	  if(buffer[0]=='#') continue;
	  if(strlen(buffer)==0) continue;

	  strcpy(mem, buffer);
  	  serv->v_hosts[n] = (virtual_host*)malloc(sizeof(virtual_host));
	  memset(serv->v_hosts[n],0,sizeof(virtual_host));
	   if((ptr=strtok(mem,"/"))!=NULL){
	     strcpy(&serv->v_hosts[n]->name[0],ptr);
	     if((ptr=strtok(NULL,"/"))!=NULL){
	     	strcpy(&serv->v_hosts[n]->path[0],ptr);
	     }
	   }
	n++;
	}
	serv->v_hosts[n]=NULL;

	free(mem);
	free(buffer);
	free(tmp);
}

void read_vfiles(FILE* fd, server_conf* serv){

        int n = 0;
        char* buffer = (char*)malloc(1024);
        char* mem = (char*)malloc(1024);
        char* tmp = (char*)malloc(256);
        char* ptr;

        while(fgets(buffer,1024,fd)!=NULL){

          if(strcmp(clip(buffer),VHOSTS_FILES)==0) break;
          if(buffer[0]=='#') continue;
          if(strlen(buffer)==0) continue;

          strcpy(mem, buffer);
          serv->v_files[n] = (virtual_files*)malloc(sizeof(virtual_files));
          memset(serv->v_files[n],0,sizeof(virtual_files));
           if((ptr=strtok(mem," "))!=NULL){
             strcpy(&serv->v_files[n]->name[0],ptr);
             if((ptr=strtok(NULL," "))!=NULL){
                strcpy(&serv->v_files[n]->path[0],ptr);
             }
             if((ptr=strtok(NULL," "))!=NULL){
                strcpy(&serv->v_files[n]->file[0],ptr);
             }
           }
        n++;
        }
        serv->v_files[n]=NULL;

        free(mem);
        free(buffer);
        free(tmp);
}

int init_conf(const char* conf_file, server_conf *serv){

    char* buffer = (char*)malloc(1024);
    FILE* fd;
    if((fd=fopen(conf_file,"r"))!=NULL){
	while((fgets(buffer,1024,fd))!=NULL){
	  if(strcmp(clip(buffer),SERVER_CONF)==0) read_server_conf(fd,serv);
	  else if(strcmp(clip(buffer),AUTH_CONF)==0) read_auth_conf(fd,serv);
	  else if(strcmp(clip(buffer),CONTENT_TYPE_CONF)==0) read_content_types(fd,serv);
	  else if(strcmp(clip(buffer),CGI_BIN_CONF)==0) read_cgi_bin(fd,serv);
	  else if(strcmp(clip(buffer),VHOSTS_CONF)==0) read_vhosts(fd,serv);
	  else if(strcmp(clip(buffer),VHOSTS_FILES)==0) read_vfiles(fd,serv);
	}
     fclose(fd);
    }else{
     fprintf(stderr, "Bad conf file:%s\n", conf_file);
     free(buffer);
     return -1;
    }

    free(buffer);

 return 0;
}
