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
#ifndef CORNY_FTP
#define CORNY_FTP

#include "mkpasswd.h"
#include "misc.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <crypt.h>

#define SOCKET int

#define USER "USER"
#define PASS "PASS"
#define STAT "STAT"
#define NOOP "NOOP"
#define QUIT "QUIT"
#define SYST "SYST"
#define PORT "PORT"
#define LIST "LIST"
#define PWD  "PWD"
#define CWD  "CWD"
#define RETR "RETR"
#define TYPE "TYPE"
#define PASV "PASV"
#define STOR "STOR"
#define QUIT "QUIT"
#define MKD  "MKD"
#define RNFR "RNFR"
#define RNTO "RNTO"
#define DELE "DELE"

#define BINARY 'I'
#define ASCII  'A'

#define MODE_PASV       0
#define MODE_ACTIVE     1

#define LOBYTE(w)       ((BYTE)(w))
#define HIBYTE(w)       ((BYTE)(((WORD)(w)>>8)&0xFF))

#define WORD    unsigned int
#define MAX_ALLOC       65536

#define READ            1
#define WRITE           2

#define ANONYMOUS       "anonymous"

#endif
