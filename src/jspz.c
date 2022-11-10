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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>

char imports[1024];
char head[2048];
char globals[4096];

FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);

char request_code[] = "JspzHttpRequest request = new JspzHttpRequest();\r\n\r\nclass JspzHttpRequest {\r\n\r\n  String getParameter(String param){\r\n    String query = System.getenv(\"QUERY_STRING\")!=null?System.getenv(\"QUERY_STRING\"):\"\";\r\n    java.util.StringTokenizer t = new java.util.StringTokenizer(query,\"&\");\r\n    while(t.hasMoreTokens()){\r\n      String tok = t.nextToken();\r\n      int i = tok.indexOf(\"=\");\r\n      String key = tok.substring(0,i).trim();\r\n      String val = tok.substring(i+1).trim();\r\n      if(key.equals(param)) return val;\r\n    }\r\n   return null;\r\n }\r\n\r\n  String getContentType(){\r\n    return System.getenv(\"CONTENT_TYPE\")!=null?System.getenv(\"CONTENT_TYPE\"):\"\";\r\n  }\r\n\r\n  String getQueryString(){\r\n    return System.getenv(\"QUERY_STRING\")!=null?System.getenv(\"QUERY_STRING\"):\"\";\r\n  }\r\n\r\n  String getRequestMethod(){\r\n    return System.getenv(\"REQUEST_METHOD\")!=null?System.getenv(\"REQUEST_METHOD\"):\"\";\r\n  }\r\n\r\n  int getContentLength(){\r\n    return Integer.parseInt(System.getenv(\"CONTENT_LENGTH\")!=null?System.getenv(\"CONTENT_LENGTH\"):\"0\");\r\n  }\r\n\r\n  byte[] getPostData(){\r\n\r\n   byte[] buffer = null;\r\n   try{\r\n     int n=getContentLength();\r\n     if(n==0) return null;\r\n     buffer = new byte[n];\r\n     java.io.DataInputStream din = new java.io.DataInputStream(System.in);\r\n     din.read(buffer);\r\n     din.close();\r\n    }catch(Exception ex){\r\n      ex.printStackTrace();\r\n    }\r\n    return buffer;\r\n  }\r\n}\r\n\r\n";

int recompile(const char *jspz_file, const char* class_file) {

    struct stat attr1, attr2;
    stat(jspz_file, &attr1);
    stat(class_file, &attr2);

    return attr1.st_mtime > attr2.st_mtime;
}

char *trim(char *s) {
    char *ptr;
    if (!s)
        return NULL;
    if (!*s)
        return s;
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}

void parse_page_content_type(FILE* fd, const char* buffer){

	int open=0;
	char ct[64];
	int n = 0;
	char* tmp = (char*)malloc(128);

	memset(ct,0,64);
	for(int i=0; i < (int)strlen(buffer); i++){
	  if(buffer[i]=='"') {
	   open=!open;
	   continue;
	  }
	  if(open) ct[n++]=buffer[i];
	}
	ct[n]='\0';

	sprintf(tmp,"Content-Type: %s\r\n\r\n", ct);
	printf("%s\n", tmp);
	fprintf(fd,"%s", tmp);
	free(tmp);

}

void parse_page_globals(FILE* fd, char* buffer){

	char curr[2];
	char* tmp = (char*)malloc(1024);
	memset(tmp,0,1024);
	int n = 0;
	char prev=0;
	while((fread(curr,1,1,fd))>0){
	  if(prev=='%' && curr[0]=='>') break;
	  tmp[n++]=curr[0];
	  prev=curr[0];
	}
	tmp[n-1]='\0';
	strcat(globals,tmp);
	strcat(globals,"\n");

}

void parse_page_import(char* buffer){

	int open=0;
	int n = 0;
	char* ptr;
	char* tmp = (char*)malloc(1024);

	for(int i = 0; i < (int)strlen(buffer); i++){
	  if(buffer[i]=='\"') {
	    open = !open;
	    continue;
	  }
	  if(open) tmp[n++]=buffer[i];
	}

	if((ptr=strtok(tmp,","))!=NULL){
	  strcat(imports,"import ");
	  strcat(imports, tmp);
	  strcat(imports,";\n");
	  while((ptr=strtok(NULL,","))!=NULL){
	    strcat(imports,"import ");
	    strcat(imports, tmp);
	    strcat(imports,";\n");
	  }
	}
	free(tmp);

}

void create_head(const char* file_name){

	char* tmp = (char*)malloc(2048);
	memset(tmp,0,128);
	sprintf(tmp,"%s\n%s\n", imports, "import static java.lang.System.*;");
	strcat(head,tmp);
	sprintf(tmp,"public class %s {\n", file_name);
	strcat(head,tmp);
	sprintf(tmp,"JspzHttpResquest request = new JspzHttpRequest();\n");
	sprintf(tmp,"  private String getEnv(String env){return getenv(env)!=null?getenv(env):\"\";}\n");
	strcat(head,tmp);
	sprintf(tmp,"  public static void main(String argv[]) throws Exception {\n");
	strcat(head,tmp);
	sprintf(tmp,"    new %s();\n", file_name);
	strcat(head,tmp);
	sprintf(tmp,"  }\n");
	strcat(head,tmp);
	sprintf(tmp,"  public %s() throws Exception {\n", file_name);
	strcat(head,tmp);

}

void parse_directives(FILE* fd){

	char* buffer = (char*)malloc(1024);
	while((fgets(buffer,1024,fd))!=NULL){

	  if(strstr(buffer,"<%@ ")!=NULL){
	    if(strstr(buffer," page import")!=NULL){
	      parse_page_import(buffer);
	    }
	    else if(strstr(buffer," page contentType")!=NULL){
	      parse_page_content_type(fd,buffer);
	    }
	  }
	  else if(strstr(buffer,"<%!")!=NULL){
	    parse_page_globals(fd, buffer);
	  }
	}
	fseek(fd,0L,SEEK_SET);
	free(buffer);
}


int file_exists(const char* file){

	FILE *fd;
	if((fd=fopen(file,"rb"))!=NULL){
	 fclose(fd);
	 return 1;
	}

 return 0;
}

char* make_file_name(const char* in_file, char* file_name){


	for(int i = 0; i < (int)strlen(in_file); i++){
	  if(in_file[i]=='.' || in_file[i]=='-' || in_file[i]=='/') file_name[i]='_';
	  else file_name[i]=in_file[i];
	}

 return file_name;
}

void clip_file_name(const char* in_file, char* in_file_name, char* in_file_path){

	int i = (int)strlen(in_file);
	char* tmp = (char*)malloc(256);

	if(strstr(in_file,"/")!=NULL){
  	  for(i=(int)strlen(in_file)-1;i>0;i--){
	    if(in_file[i]=='/') break;
	  }
	  sprintf(in_file_name,"%s", &in_file[i+1]);
	  strcpy(tmp,in_file);
	  tmp[i]='\0';
	  sprintf(in_file_path,"%s", tmp);
	}else{
	  sprintf(in_file_name, "%s", in_file);
	}

	free(tmp);
}


void handle_code(FILE* out){

}

char* make_plain(const char* orig, char* dest){

	int n = 0;
	for(int i = 0; i < (int)strlen(orig); i++){

	  if(orig[i]=='\n'){
	    dest[n++]='\\';
  	    dest[n] = 'n';
	  }else{
	    dest[n] = orig[i];
	  }
	  n++;
	}

 return dest;
}

void handle_chars(FILE* out, char* buffer, int mode){

	char *tmp = (char*)malloc(4096);

	memset(tmp,0,1024);
	if(buffer[0]!='@' && buffer[0]!='!'){
	  if(mode==1){
	    if(buffer[0]=='='){
	      fprintf(out,"out.print(%s);\n",&buffer[1]);
	    }
	    else fprintf(out, "%s", buffer);
	  }else if(mode==0){
	    if(!(strlen(buffer)==1 && buffer[0]=='\n')){
	      fprintf(out,"out.print(\"%s\");\n", make_plain(buffer,tmp));
	    }
	  }
	}
	free(tmp);
}

void parse_jspz(FILE* in, FILE* out){

	char curr[2];
	char prev=0;
	int  open = 0;
	char buffer[65536];
	int n = 0;

	memset(buffer,0,65536);
	while((fread(curr,1,1,in))>0){

	  if(curr[0]=='%' && prev == '<'){
	    buffer[n]='\0';
	    handle_chars(out,buffer,open);
	    open = 1;
	    prev=0;
	    n=0;
	    memset(buffer,0,65536);
	    continue;
	  }
	  else if(prev=='%' && curr[0]=='>'){
	    buffer[n]='\0';
	    handle_chars(out,buffer,open);
	    open=0;
	    prev=0;
	    n=0;
	    memset(buffer,0,65536);
	    continue;
	  }

	  if(prev!=0) {
	    buffer[n++] = prev;
	    buffer[n]='\0';
	  }
	  prev = curr[0];
	}

	buffer[n]='\0';
	handle_chars(out,buffer,open);

}

void find_jars(const char* workdir, char* cp, int lencp){

  DIR *dp;
  struct dirent *ep;

  dp=opendir(workdir);
  if(dp!=NULL){
   while((ep = readdir(dp))){
    if(strstr(ep->d_name,".jar")>0 || strstr(ep->d_name,".zip")>0) {
	strcat(cp,workdir);
	strcat(cp,"/");
	strcat(cp,ep->d_name);
	strcat(cp,":");
    }
   }
   (void)closedir(dp);
  }
}

void read_jspz_include(char* buffer, int len){

	int l = (int)strlen(request_code);
	if(l<len){
	  strcpy(buffer, request_code);
	}

}

int main(int args, char* argv[]){

	FILE* fd;
	FILE* fi;
	FILE* ci;

	char buffer[1024];
	char in_file[1024];
	char in_file_path[1024];
	char in_file_name[128];
	char file_name[128];
	char java_file[1024];
	char class_file[1024];
	char jspz_import[8192];
	char work_dir[256];
	char exec[3072];
	char java[64];
	char javac[64];
	char classpath[1024];
	int  frecomp = 0;

	strcpy(in_file,argv[args-1]);

	if(!file_exists(in_file)){
	  printf("Bad file: %s\n", in_file);
	  return -1;
	}

	memset(imports,0,1024);
	memset(head,0,2048);
	memset(globals,0,4096);
	memset(in_file_path,0,1024);
	memset(classpath,0,1024);

	char* wptr = getenv("JSPZ_WORKDIR");
	if(wptr==NULL){
	 strcpy(work_dir,"JSPZWD");
	}else{
	  strcpy(work_dir, wptr);
	}

	memset(java,0,64);
	memset(javac,0,64);
	if((wptr=getenv("JSPZ_JAVA"))!=NULL){
	  strcpy(java,wptr);
	}else{
	  strcpy(java,"java");
	}

	if((wptr=getenv("JSPZ_JAVAC"))!=NULL){
	  strcpy(javac,wptr);
	}else{
	  strcpy(javac,"javac");
	}

	mkdir(work_dir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	make_file_name(in_file, file_name);
	clip_file_name(in_file, in_file_name, in_file_path);

	for(int i = 1; i < args; i++){
	  if(strcmp(argv[1],"-r")==0 && i+1<args-1) frecomp=1;
	}

	sprintf(java_file,"%s/%s.java", work_dir, file_name);
	sprintf(class_file,"%s/%s.class", work_dir, file_name);

	find_jars(&work_dir[0],&classpath[0],1024);

	if((recompile(in_file,class_file) || frecomp) || !file_exists(java_file)){
	  if((fd=fopen(in_file,"r"))!=NULL){
	    if((fi=fopen(java_file,"w"))!=NULL){
	      parse_directives(fd);
	      create_head(file_name);
	      fprintf(fi,"%s\n", head);
	      parse_jspz(fd,fi);
	      read_jspz_include(jspz_import, 8192);
	      fprintf(fi,"  }\n%s\n\n%s\n\n}\n", globals, jspz_import);
	      fclose(fi);
	    }
	   fclose(fd);
	  }else printf("Error: can't make file:%s\r\n\r\n", java_file);

	  sprintf(exec,"%s -classpath %s%s -d %s %s 2>&1", javac, classpath, work_dir, work_dir, java_file);

	  if((ci=popen(exec,"r"))!=NULL){
	    while((fgets(buffer,1024,ci))!=NULL){
		printf("%s", buffer);
	    }
	    pclose(ci);
	  }
	// End recompile,,,
	}

	sprintf(exec,"%s -cp %s%s %s 2>&1", java, classpath, work_dir, file_name);
	if((ci = popen(exec,"r"))!=NULL){
	  while((fgets(buffer,1024,ci))!=NULL){
	    printf("%s", buffer);
	  }
	 pclose(ci);
	}else printf("Bad java:%s\r\n\r\n", exec);



 return 0;
}
