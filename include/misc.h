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

#ifndef _CORNY_MISC_
#define _CORNY_MISC_

typedef unsigned char BYTE;

char* clipend(char* str);
char* trimpath(char* str);
int   ends_with(char* string, char* end);
char* get_date_time(char* buffer);
char* clip(char* buffer);
char* toupperc(char* upper, const char* buffer, char stop);
char* get_work_dir(char* cwd, int len);
void  split(const char* buffer, char* path, char* file, char* qs, unsigned int maxqs);
int   startsw(const char* str, const char* str2);
char* trim(char* buffer);
void  url_encoder_rfc_tables_init(char* html5, char* rfc3986, int len);
char* url_encode( char *table, unsigned char *s, char *enc);
int   file_exists(const char* file);
int   copy_file(const char* orig, const char* new);
int   decode(const char *s, char *dec);
char* get_http_param(const char* qs, char* buffer, char* name);

#endif
