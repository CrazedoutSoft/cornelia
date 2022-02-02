<html>
<body>

<h2>Simple FTP Server</h2>
A simple FTP server is provided in the Cornalia package.</br>
Users are defined in conf/ftp.passwd and has READ and WRITE access.<br/>
<i>anonymous</i> can be set to accept but has only READ access.<p/>
<b>Following werbs are implemented so far</b>.<br/>

<pre>
<code>
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
</code>
</pre>

>bin/ftp_cornelia

</body>
<html>
