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

#ifndef _WEBS_CONF_
#define _WEBS_CONF_

#define ORG_SERVER_NAME "Cornelia"
#define SOCKET unsigned int

void handle_request(SOCKET sockfd, char* clientIP, void* cSSL);

typedef struct auth_t {

        char realm[128];
        char path[1024];
        char base64auth[256];

} auth_conf;

typedef struct cgi_exec_t {
	char ext[16];
	char exec[128];
} cgi_exec;

typedef struct content_type_t {

        char file_ext[16];
        char content_type[64];

} content_type;

typedef struct http_request_t {

        SOCKET sockfd;
        char  method[5];
        char  file[256];
        char  path[1024];
        char  query_string[1024];
        char  httpv[12];
        char  clientIP[16];
        char* headers[256];
        int   headers_len;
        unsigned char* post_data;
	void* cSSL;
	char connection[65];

} http_request;

typedef struct http_response_t {

        SOCKET sockfd;
        http_request* request;
        char  content_type[128];
        int   content_length;
        char* envp[256];

} http_response;


typedef struct server_conf_t {

	unsigned int port;
	unsigned int ssl_port;
	unsigned int tls_port;
	unsigned int max_keep_alive;
	char server_name[64];
        char www_root[256];
	char workdir[1024];
        char default_page[64];
	char cgi_bin[256];
        char execs[128];
	char logfile[256];
	char allow_dir_listing[16];
        auth_conf* auth[64];
	content_type* content_types[64];
	cgi_exec* exec_c[64];
	char cert[1024];
	char cert_key[1024];
	char certcrt[1024];
	char keycrt[1024];
	int  max_keep_alive_requests;
	int  keep_alive_timeout;

} server_conf;

#endif
