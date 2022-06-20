#include "../../include/misc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void exec(const char* code){

	FILE *fd;
	char* file = (char*)malloc(1024);
	char* buffer = (char*)malloc(1024);

	fd = fopen("/tmp/jgaz.jg","w");
	fputs(code, fd);
	fclose(fd);

	fd = popen("jgazm/jgazm /tmp/jgaz.jg 2>&1","r");
	printf("<div style='color: white'><pre><code>\n");

	while((fgets(buffer,1024,fd))!=NULL){
	  printf("%s", buffer);
	}
	fclose(fd);
	printf("</code><pre></div>\n");
	free(buffer);

}

int main(int args, char* argv[]){

	char* qs = getenv("CONTENT_LENGTH");
	char* buffer;
	char* decoded;
	int len;

	printf("Content-Type: text/html\n\n");

	if(qs!=NULL){
	  len = atoi(qs);
	  buffer = (char*)malloc(len+1);
	  decoded = (char*)malloc(len+1);
	  memset(buffer,0,len);
	  memset(decoded,0,len);
	  fread(buffer,len,1,stdin);
	  decode(&buffer[5],decoded);
	  exec(decoded);
	  free(buffer);
	  free(decoded);
	}


}
