jGazm v.1.2.2 Copyright CrazedoutSoft / Fredrik Roos 2016.

!!!The %JAVA_HOME% variable in batch files must be set to point to a JDK 1.7 or above !!!

*******************
Linux: Run setup_linux from jGazm directory. Following files will be generated.

jgazm - jGazm console.
helloworld.jgazm - example file.
gazmedit.properties
*******************

The Editor executes the code in the Textfield OR the user selected code snibbet.


Usage: jgazm {options} [file] %arg1 %arg2 ...
Options:
-stdin				Read input directly from System.in (Single "." + ENTER quits)
-workdir			Set path to jGazm runtime working directory.
-dump				dump script as POJO to stdout.
-dump:<file>			dump script as POJO to <file>.
-clean				Remove all temp files generated.
-export:<dest_jar>              export the works as one java runnable package.
-java				Input file is pure Java. Can not be used with -stdin.
				-dump,-clean,-noimport and -keepsource has no meaning when -java in used.
				will be set by default if file ends width '.java'.
-keepsource			Do not delete generated POJO code.
-noimport			Do not import any packages by default.
-version			Print jgazm version.
-check				Compile only.
-showapi			Print methods of com.crazedout.jgazm.Lang.
-showcp				Print current classpaths.
-help or /?			Prints this message.
