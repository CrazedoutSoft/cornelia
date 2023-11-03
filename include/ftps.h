
#ifndef _CORNY_FTPS
#define _CORNY_FTPS
#include <openssl/ssl.h>
#include <openssl/err.h>


SSL_CTX *create_context();
void configure_context(SSL_CTX *ctx, const char* cert, const char* key);
int ssl_write(void* ssl, char* buffer, int len);
int ssl_read(void* ssl, char* buffer, int len);

#endif
