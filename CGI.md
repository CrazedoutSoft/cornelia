<html>
<body>
Default .cgi exec.<p/>

[cgi_exec_conf]<br/>
.cgi=[shell]<br/>
.pl=[shell]<br/>
.py=[shell]<br/>
.php=/usr/bin/php-cgi<br/>
.jgazm=jgazm/jgazm<br/>
.jsp=bin/jspz
.jspz=bin/jspz
[cgi_exec_conf]<br/>


The ENV varibles that are passed to CGI are at this time:

<ul>
<li>CONTENT_LENGTH (POST context)</li>
<li>CONTENT_TYPE</li>
<li>QUERY_STRING</li>
<li>REQUEST_METHOD</li>
<li>REQUEST_URI</li>
<li>REDIRECT_STATUS</li>
<li>SCRIPT_FILENAME</li>
<li>SCRIPT_NAME</li>
<li>SERVER_NAME</li>
</ul>

Incoming headers are also set as ENV as <i>HTTP_[header_name]=[header_value]</i>.
Example: HTTP_COOKIE=melike:cookies

</body>
</html>
