<html>
<body>

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

</body>
</html>
