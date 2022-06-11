
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getenv(const char* name);

int main(int args, char* argv[]){


   printf("Set-Cookie: crazedout=soft\n");
   printf("Content-Type: text/html\r\n\r\n");

   printf("<h2>Hello from C CGI -  where the rubber hits the road</h2>");


 return 0;
}
