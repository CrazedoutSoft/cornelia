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

#define MAX_WWW_ROOT		256
#define MAX_WORK_DIR		1024
#define MAX_DEFAULT_PAGE 	64
#define MAX_CGI_BIN		256
#define MAX_EXEC_DEF		64
#define MAX_LOG_FILE		64
#define MAX_AUTH_REALMS		64
#define MAX_CONTENT_TYPES	64
#define MAX_VIRTUAL_HOSTS	64
#define MAX_VIRTUAL_PATH	256
#define MAX_HTTP_HEADERS	256
#define MAX_ENV_VARS		256
#define MAX_QUERYSTRING	 	2048

void handle_request(SOCKET sockfd, char* clientIP, void* cSSL);

typedef struct virtual_host_t {

	char name[128];
	char path[1024];

} virtual_host;

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
        char  path[2048];
        char  query_string[2048];
        char  httpv[12];
        char  clientIP[16];
        char* headers[MAX_HTTP_HEADERS];
        int   headers_len;
        unsigned char* post_data;
	void* cSSL;
	char connection[65];
	char virtual_path[MAX_VIRTUAL_PATH];

} http_request;

typedef struct http_response_t {

        SOCKET sockfd;
        http_request* request;
        char  content_type[128];
        int   content_length;
        char* envp[MAX_ENV_VARS];

} http_response;


typedef struct server_conf_t {

	unsigned int port;
	unsigned int ssl_port;
	unsigned int tls_port;
	unsigned int max_keep_alive;
	char server_name[64];
        char www_root[256];
	char workdir[MAX_WORK_DIR];
        char default_page[MAX_DEFAULT_PAGE];
	char cgi_bin[MAX_CGI_BIN];
        char execs[MAX_EXEC_DEF];
	char logfile[MAX_LOG_FILE];
	char allow_dir_listing[16];
        auth_conf* auth[MAX_AUTH_REALMS];
	content_type* content_types[MAX_CONTENT_TYPES];
	cgi_exec* exec_c[MAX_EXEC_DEF];
	char cert[1024];
	char cert_key[1024];
	char certcrt[1024];
	char keycrt[1024];
	int  max_keep_alive_requests;
	int  keep_alive_timeout;
	virtual_host* v_hosts[MAX_VIRTUAL_HOSTS];
	unsigned int max_post_data;

} server_conf;

int   readline(const http_request* request, char* buffer, int len);
void  send_bad_request(http_response* response, char* code);
/* Not very pretty but it will do for now...*/
void  send_bad_request2(http_request* request);
void  send_internal_error(http_response* response);
char* get_head(http_response* response, char* head, char* code);
void  free_request(http_request* r);
void  free_response(http_response* r);
void  parse_env(http_response* res);
void  dump_r(http_request* r);
char* get_header(const http_request* request, const char* header);
char* encode_url(unsigned char* url, char* url_enc);
int   socket_read(const http_request* request, char* buffer, int len);
int   socket_write(const http_request* request, const char* buffer, int len);
int   exec_request(SOCKET sockfd, char* clientIP, void* cSSL);

#endif

