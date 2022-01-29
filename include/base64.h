
#ifndef _BASE64_CORNY_
#define _BASE64_CORNY_


char *base64_encode(const unsigned char *data,
                    int input_length,
                    int *output_length);


unsigned char *base64_decode(const unsigned char *data,
                             int input_length,
                             int *output_length);

#endif
