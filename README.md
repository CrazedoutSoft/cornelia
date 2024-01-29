
<div >
<a href="https://github.com/othneildrew/Best-README-Template">
    <img src="www/corn2.png" alt="Logo" width="600" >
  </a>
 
</div>

******************************************************
*  Welcome to Cornelia Web Server                    
*  CrazedoutSoft 2022 by Fredrik Roos.           
*  http://crazedout.com info@crazedout.com   
*  https://github.com/openssl                      
******************************************************

<b>Quick start:</b><br>
><b>git clone https://github.com/CrazedoutSoft/cornelia.git</b><br>
><b>cd cornelia</b><br>
><b>./build_all no-ssl-no-warn</b>  [<i>no_ssl-no-warn</i> excludes problems with ssl/tls and compiler warnings]<br>
><b>export CORNELIA_HOME=[current_dir]</b><br>
><b>bin/startup.sh</b><br>
><b>Result in prompt:</b><br>
><b>Cornelia listening on 8080 [HTTP]</b><br>
><a href="http://localhost:8080">http://localhost:8080</a>
<br/>
><a href="https://crazedout.com/corny.mp4">Installation video</a>
<hr>
<b>RECOMMENDED: FULL INSTRUCTIONS:</b>
<p>

goto cornelia install directory.<br/>

set env CORNELIA_HOME to cornelia install dir.
(export CORNELIA_HOME=<path to cornelia>)

If you start bin/startup.sh from cornelia directory CORNELIA_HOME is set to pwd.

Default HTTP port is 8080, SSL 8081, TLS 8082. Edit this in conf/corny.conf

>make (or 'make no_ssl' for http only)<br/>
>bin/startup.sh (may have to chmod 755 on *.sh files in /bin) starts http server.<br/>
>bin/startup.sh http starts http server only.<br/>
>bin/startup.sh ssl starts ssl server only.<br/>
>bin/startup.sh tls starts tls server only.<br/>
>bin/startup.sh all starts http,ssl and tls server.<br/>

The same applies to bin/shutdown.<p/>

If you want to be able to start Cornelia without the <i>bin/startup.sh</i> script<br/>
and the openssl libs are not in /usr/lib, then LD_LIBRARY_PATH need to be set.<p/>

>export LD_LIBRARY_PATH=$CORNELIA_HOME/openssl<p/>

If lib errors appears in make process or runtime you may have to copy: <br/>
 openssl/libcrypto.so, openssl/libcrypto.so.3, openssl/libssl.so and openssl/libssl.so.3 <br/>
to /usr/lib<p/>

<p>
test if you have php-cgi installed<br/>
>which php-cgi
<p/>
otherwise download and install php-cgi like so (Ubuntu):<br/>

>sudo apt install php<br/>
>sudo apt install php-cgi<p/>

Root directory by default is set to $CORNELIA_HOME/www. In "www" there are a numbers of test files for various stuff.</br>
Delete these files at your own discretion. Please leave "www/res" as the icons for dir listing resides there - by default.<br/> 
If you change the root directory, Cornelia will still look for these icons in /res so a sym link or actual move is recommended.<p/>
<p/>

<b>Warning: Since Cornelias primary purpose is for development the header 'Access-Control-Allow-Origin' is set to '*' by default.<br>
This is dangerous in a open context. Change this in conf/Access-Control-Allow.txt<p/>
For the http <i>OPTIONS</i> request the response is set in conf/http_options.
</b>
<p />

<b>User enpoints</b><br>
Set up virtual endpoints:<br>
In conf/corny:<br>
[virtual_files]<br>
#endpoint path file<br>
/myendpoint / my_repsponse_file.pl<br>
[virtual_files]<p>

><b>Set up custum endpoints on cmd line:</b><br>
>bin/cornelia_d -uep:/myendpoint%file:myjson.js%application/json<br>
>bin/cornelia_d -uep:/myendpoint%{"my","answer"}%application/json<br>
>If <i>Content-Type</i> is omitted - application/json is default.<br>
><b>http://localhost:8080/myendpoint</b> will respond with supplied response (or from file) with <i>Content-Type</i> or application/json if omitted.
<p>

OpenSSL:<br/>
If you can't build Cornelia with the OpenSSL headers and libs provided you <br/>
must compile your own OpenSLL libs and replace them in dir openssl<br/>
https://github.com/openssl
<p>
<b> ! You should of course replace the default cert/keys with your own using cert/make_ssl_cert and/or cert/make_tls_cert !</b>
<p>
Cornelia handles all default CGI such as plain c binaries, shell script, perl and so on.<br/>
PHP is handled by php-cgi and *.jgazm by jGazm.<br>
Cornelia also includes a home brewed simple version of JSP <a href="JSPZ.md">JSPZ.md</a>.
<p/>
<p/>
jGazm:<br/>
 1) Java JDK 1.7 or above must be installed. Some JRE's may work but no guarantee.<br/>
 2) Run ./setup_linux (you may have to chmod 755 on "setup_linux")<p/>
 3) For 'Online jGazm' to work $CORNELIA_HOME/jgazm needs to be in PATH.<p/>

>cd jgazm<br/>
>./setup_linux</p>

 3) JAVA_HOME must be set to current JVM 1.7 (or above) in file jgazm. Setup script will try to get it right but may fail.<p/>


 Good Luck / Fredrik. 
