
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

goto cornelia install directory.<br/>

set env CORNELIA_HOME to cornelia install dir.
(export CORNELIA_HOME=<path to cornelia>)

If you start bin/startup.sh from cornelia directory CORNELIA_HOME is set to pwd.

>make (or 'make nossl' for http only)<br/>
>bin/startup.sh (may have to chmod 755 on *.sh files in /bin) starts http,ssl and tls.<br/>
>bin/startup.sh http starts http server only<br/>
>bin/startup.sh ssl starts ssl server only<br/>
>bin/startup.sh tls starts tls server only<br/>

The same applies to bin/shutdown.<p/>

If you want to be able to start Cornelia without the <i>bin/startup.sh</i> script<br/>
and the openssl libs are not in /usr/lib, then LD_LIBRARY_PATH need to be set.<p/>

>export LD_LIBRARY_PATH=$CORNELIA_HOME/openssl<p/>

If lib errors appears in make process or runtime you may have to copy: <br/>
 openssl/libcrypto.so, openssl/libcrypto.so.3, openssl/libssl.so and openssl/libssl.so.3 <br/>
to /usr/lib<br/>

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

OpenSSL:<br/>
If you can't build Cornelia with the OpenSSL headers and libs provided you <br/>
must compile your own OpenSLL libs and replace them in dir openssl<br/>
https://github.com/openssl
<br/>
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
