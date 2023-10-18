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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>


char* clipend(char* str){

        for(int i = strlen(str)-1; i>-1; i--){
          if(str[i]!='\n' && str[i]!='\r') {str[i+1]=0;break;}
        }

 return str;
}

char* trimpath(char* str){

        char* buffer = (char*)malloc((int)strlen(str)+1);
        char* path = (char*)malloc((int)strlen(str));
        char old_token[64];
        char* token;

        memset(path,0,strlen(str));
        memset(old_token,0,64);
        strcpy(buffer,str);
        token = strtok(buffer,"/");
        do{
          if(strcmp(token,"..")!=0) {
                strcat(path,old_token);
                strcat(path,"/");
           }
         strcpy(&old_token[0],token);
        }while((token=strtok(NULL,"/"))!=NULL);

 	if(strstr(old_token,"..")==NULL) strcat(path,old_token);
        strcpy(str,path);

        free(buffer);
        free(path);

 return str;
}

int ends_with(char *string, char* end){

  string = strrchr(string, '.');

  if (string != NULL)
    return strcmp(string, end);

  return -1;
}

char* get_date_time(char* buffer){

    time_t current_time;
    char* c_time_string;

    current_time = time(NULL);

    if (current_time == ((time_t)-1)){
	return NULL;
    }

    c_time_string = ctime(&current_time);

    if (c_time_string == NULL){
	return NULL;
    }

    sprintf(buffer,"%s", c_time_string);

 return buffer;
}


int copy_file(const char* orig, const char* newf){


	FILE* fd;
	int len;
	char* buffer;
	int r=-1;

	if((fd=fopen(orig,"r"))!=NULL){
	  fseek(fd,0L,SEEK_END);
	  len=ftell(fd);
	  fseek(fd,0L,SEEK_SET);
	  buffer=(char*)malloc(len);
	  r=fread(buffer,1,len,fd);
	  fclose(fd);
	  if((fd=fopen(newf,"w"))!=NULL){
	    fwrite(buffer,1,len,fd);
	    fclose(fd);
	  } else {
	    free(buffer);
	    return -1;
	  }
	}else return -1;

 return r;
}

int file_exists(const char* file){

    FILE *fd;
    if ((fd = fopen(file, "r")))
    {
        fclose(fd);
        return 1;
    }
    return 0;
}

void url_encoder_rfc_tables_init(char* html5, char* rfc3986, int len){

    int i;

    for (i = 0; i < len; i++){

        rfc3986[i] = isalnum( i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
        html5[i] = isalnum( i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }
}

char *url_encode( char *table, unsigned char *s, char *enc){

    for (; *s; s++){

        if (table[*s]) *enc = table[*s];
        else sprintf( enc, "%%%02X", *s);
        while (*++enc);
    }

    return( enc);
}

char* toupperc(char* upper, const char* buffer, char stop){

        int st=0;
        for(int i = 0; i < (int)strlen(buffer); i++){
         if(buffer[i]==stop) st=1;
         if(!st) {
          if(buffer[i]=='-') upper[i]='_';
          else upper[i]=toupper(buffer[i]);
         }
         else upper[i]=buffer[i];
        }

 return upper;
}

char* clip(char* buffer){

	for(int i = 0; i < (int)strlen(buffer); i++){
	  if(buffer[i]=='\r' || buffer[i]=='\n') {
	    buffer[i]='\0';
	    break;
	  }
	}

 return buffer;
}

char* get_work_dir(char* cwd, int len){
   return getcwd(cwd, len);
}

void split(const char* buff, char* path, char* file, char* qs, unsigned int maxqs){

	char* buffer = (char*)malloc(strlen(buff)+1);
	strcpy(buffer, buff);

	memset(qs,0,maxqs);
        for(int i = strlen(buffer); i>-1; i--){
          if(buffer[i]=='?') {
            if(strlen(&buffer[i+1])<maxqs){
	      strcpy(qs, &buffer[i+1]);
	    }else{
	      memcpy(qs,&buffer[i+1],maxqs);
	    }
            buffer[i]='\0';
          }
        }

        for(int i = strlen(buffer); i>-1; i--){
          if(buffer[i]=='/') {
            strcpy(&file[0], &buffer[i+1]);
            buffer[i]='\0';
            strcpy(&path[0], &buffer[0]);
            strcat(&path[0],"/");
            break;
          }
        }

	free(buffer);
}

int startsw(const char* str, const char* str2){
        int n=0;
        for(int i = 0; i < (int)strlen(str2); i++){
         if(str[i]!=str2[i]) n++;
        }

 return n;
}

char* trim(char* buffer){
	int mark=0;
	int n=0;
	char* tmp = (char*)malloc(strlen(buffer));
	for(int i = 0; i < (int)strlen(buffer); i++){
	  if(buffer[i]!=' '){
	    mark=1;
	  }
	 if(mark){
	  tmp[n++]=buffer[i];
	 }
	}
	tmp[n]='\0';

	memcpy(buffer,tmp,n);
	free(tmp);

  return buffer;
}

int ishex(int x){
        return  (x >= '0' && x <= '9')  ||
                (x >= 'a' && x <= 'f')  ||
                (x >= 'A' && x <= 'F');
}

int decode(const char *s, char *dec){
        char *o;
        const char *end = s + strlen(s);
        int c;

        for (o = dec; s <= end; o++) {
                c = *s++;
                if (c == '+') c = ' ';
                else if (c == '%' && (  !ishex(*s++)    ||
                                        !ishex(*s++)    ||
                                        !sscanf(s - 2, "%2x", &c)))
                        return -1;

                if (dec) *o = c;
        }
        return o - dec;
}

char* get_http_param(const char* qs, char* buffer, char* name){

        char* ptr;
        char* tok;
        char* tmp = (char*)malloc(strlen(qs)+1);

        strcpy(tmp,qs);
        ptr = strstr(tmp,name);
        if(ptr!=NULL){
          tok = strtok(ptr,"=");
          if(tok!=NULL) {
           tok=strtok(NULL,"&");
           if(tok!=NULL) {
	    strcpy(buffer,tok);
	    free(tmp);
	    return buffer;
	   }
          }
        }
        free(tmp);

  return NULL;
}
