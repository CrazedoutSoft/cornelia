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
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <fcntl.h>

#define SA struct sockaddr

int open_socket_by_ip(const char* ip, int port);


int list_ip(int port) {

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
	    if(strcmp(&addressBuffer[0],"127.0.0.1")==0) continue;
	    if(open_socket_by_ip(&addressBuffer[0], port)>-1){
	      printf("%s is local\n", &addressBuffer[0]);
	    }

        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

int create_socket(int port){

    int s;
    struct sockaddr_in addr;
    unsigned int len = sizeof(addr);
    struct sockaddr_in* pV4Addr;
    struct sockaddr_in cli;
    struct in_addr ipAddr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    printf("accepting\n");
    char cip[20];
     int client = accept(s, (struct sockaddr*)&cli, &len);
            memset(&cip[0],0,16);
            pV4Addr = (struct sockaddr_in*)&cli;
            ipAddr = pV4Addr->sin_addr;
            inet_ntop(AF_INET, &ipAddr, &cip[0], INET_ADDRSTRLEN );
     	    printf("%s\n", &cip[0]);

    return s;
}

int open_socket_by_ip(const char* ip, int port){

        int sockfd;
        struct sockaddr_in servaddr, cli;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1){
         return -1;
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(ip);
        servaddr.sin_port = htons(port);

	fcntl(sockfd, F_SETFL, O_NONBLOCK);
        if(connect(sockfd, (SA*)&servaddr,sizeof(servaddr))>-1){
         return sockfd;
        }else{
         return -1;
        }

}

int get_remote_ip(){


}

int main(int args, char* argv[]){

	int pid=fork();
	if(pid==0)
	create_socket(8821);
	else
	list_ip(8821);

 return 0;
}


