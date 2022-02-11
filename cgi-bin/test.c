
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getenv(const char* name);

int main(int args, char* argv[]){


   printf("Set-Cookie: fredrik=roos\n");
   printf("Content-Type: text/html\r\n\r\n");

   printf("<h2>Hello from C CGI</h2>");


 return 0;
}
