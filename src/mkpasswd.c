#include "../include/mkpasswd.h"
#include <stdio.h>
#include <crypt.h>

int main(int args, char* argv[]){

	if(args<2) {
	  printf("usage: mkpasswd <password>\n");
	  return 0;
	}

	printf("%s\n", crypt(argv[1], SALT));

 return 0;
}
