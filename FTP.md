<html>
<body>

<h2>Simple FTP/FTPS Server</h2>
A simple FTP server is provided in the Cornalia package.</br>
Users are defined in conf/ftp.passwd and has READ and WRITE access.<br/>
<i>anonymous</i> can be set to accept but has only READ access.
<p>
<b>WARNING Cornelia FTP Server is probably highy insecure at this stage. Do not use in a public context</b><p>
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
#define XPWD "PWD"
#define CWD  "CWD"
#define XCWD "XCWD"
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
<br>
<b>FTPS</b><br>

>Use implicit FTP over TLS.

>Read more about ftps etc here: <a href="https://www.sharetru.com/blog/explicit-ftps-vs-implicit-ftps-what-you-need-to-know">Implicit / Explicit TLS</a>

>bin/ftp_cornelia -bind [my_non_local_ip] -port 8821 -anonymous_allowed -tls

>tls turn on FTPS mode.

<p>

>For now the key and cert is the same as default Cornelia Web Server (conf/cert)

<p>

> Files: Spaces in filenames will be replaced by '_' (underscore).

<p>
Cornelia FTP needs a IP for PASV connection. Otherwise the PASV IP could be 127.0.0.1 which<br/>
non local users can't connect to.<br>
Non local IP can be uptained by using <i>bin/findip</i>.<br>
Cornelia FTP will try to get this right on start-up but may fail depending on network circumstances.<br>

</body>
<html>
