
<div >
<a href="https://github.com/othneildrew/Best-README-Template">
    <img src="www/corny_logo.png" alt="Logo" width="600" >
  </a>
 
</div>

******************************************************
*  Welcome to Cornelia Web Server                    
*  CrazedoutSoft 2022 by Fredrik Roos.           
*  http://crazedout.com/corny/ info@crazedout.com   
*  https://github.com/openssl                      
******************************************************

goto cornelia install directory.<br/>

set env CORNELIA_HOME to cornelia install dir.
(export CORNELIA_HOME=<path to cornelia>)

>make<br/>
>bin/startup.sh (may have to chmod 755 on *.sh files in /bin)
<p/>
If lib errors appears in make process or runtime you may have to copy: <br/>
 openssl/libcrypto.so, openssl/libcrypto.so.3, openssl/libssl.so and openssl/libssl.so.3 <br/>
to /usr/lib<br/>

test if you have php-cgi installed<br/>
>which php-cgi
<p/>
otherwise download and install php-cgi like so:<br/>

>sudo apt install php<br/>
>sudo apt install php-cgi<br/>

<p/>
OpenSSL:<br/>
If you can't build Cornelia with the OpenSSL headers and libs provided you <br/>
must compile your own OpenSLL libs and replace them in dir openssl<br/>
https://github.com/openssl
<br/>
<p/>
jGazm:<br/>
 1) Java JDK 1.7 or above must be installed. Some JRE's may work but no guarantee.<br/>
 2) Run ./setup_linux (you may have to chmod 755 on "setup_linux")<br/>
 3) JAVA_HOME must be set to current JVM 1.7 (or above) in file jgazm. Setup script will try to get it right but may fail.<br/>

 Good Luck / Fredrik. 
