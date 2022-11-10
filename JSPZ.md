<h2>JSPZ a simple JSP engine</h2>

Cornelia Web Server ships with a simple Java JSP engine.<br/>
There is no Java container or Servlets. The jspz engine just parses the jsp 'code' into a POJO, compiles and executes.<br/>
If the source file is older than the class file generated no compilation in done.<p/>

The jspz engine uses "java" and "javac" for its operations. If you with to use other Java distros set theese env varibles:

<p/>

>export JSPZ_JAVA=my_custom_java<br/>
>export JSPZ_JAVAC=my_custom_javac<br/>

The jspz engine uses ./JSPZWD as default working directory. Change this by setting:
<p/>

>export JSPZ_WORKDIR=myworkdir (user jars goes here)<br/>

Supported JSP directives are:<p/>

&lt;%@ page import %&gt;
<p/>
&lt;%@ page contentType %&gt;
<p/>
&lt;%=java statement %&gt;
<p/>
&lt;%
  .. java code
%&gt;
</p>
&lt;%!
  .. 'global' java code such as methods.
%&gt;
<p/>

Jspz has an static import for java.lang.System.*
<p/>

The JSP action tag (&lt;jsp:useBean...&gt;) is NOT supported at this time.<br/>
<p/>
Java files generated has an inner class JszpHttpRequest wich has theese methods:<b/>

<ul>
<li>String getParameter(String param)</li>
<li>String getContentType()</li>
<li>String getQueryString()</li>
<li>String getRequestMethod()</li>
<li>int getContentLength()</li>
<li>byte[] getPostData()</li>
</ul>
<pre>

 <html>
  <body>
   <%=request.getParameter("name")%>
  </body>
 </html>

</pre>
<p/>
The jspz engine is located at bin/jspz and can be run out side the server with jspz file as argument (bin/jspz myjspz.jspz).<br/>
File extension should be *.jspz or *.jsp
</p>


