# ondir_platform
ON Director Platform Ver 2.0 based

Requirements
-------------
1. libncurses
   libedit
   liburcu
   libcrypto
   libfutex

2. ON director platform code can be checked out from http://github.com with the below command:
   git clone https://github.com/Open-SFC/ondir-platform.git

   This will create ondir-platform repositoy in your currnet directory.

   Move to ".../ondir-paltform" directory.

3. All the opensource source code is placed at ofcdriver/ext directory.

4. While compilation you may observe issues with ant-tool. Install the ant-tool on the development machine.
   apt-get install ant

5. Check softlink which java ->/usr/bin/java --> /etc/alternatives/java --> /usr/lib/jvm/java-6-sun/bin/java
   if not install sun java6 for ubuntu from the Internet, and update the java soft-link as mentioned above.

6. All the documentation related files are located at ofcdriver/doc..

Compilation Procedure 
-----------------------
1. cd ofcdriver

2. Update JAVA_HOME,PATH,CLASSPATH in the respective shell script mentioned below.

3. For P4080, Upate the SDK_INSTALL_PATH in p4080-sdk-1.4-env.sh This source has used QorIQ SDK v1.4 
   Browse through the website for the SDK1.4 http://linux.freescale.net/labDownload2/viewDownloads.php
   source p4080-sdk-1.4-env.sh

4. For X86 32 bit, run source x86-env.sh
       X86 64 bit, run source x86-64bit-env.sh

5. make 

Installation
-------------
1. make imageinstall 
   which will install the Ondirector binaries and shared object files in the "/usr/local/ondir_platform/1.0" directory

1. make hdrinstall 
   which will install the Ondirector header files that are required for the applications in the "/usr/local/ondir_platform/1.0/include" directory.

3. cd /usr/local/ondir_platform/2.0 
   sudo ./ond-of-start.sh

NOTE: controller is running on 6653 by default, user can change the port number to his desired one by modifying the line as follows in ondirector-start.sh and restart the controller
./controller 0 8 -f &  ==> ./controller 0 8 6633 -f &

Binary files
-------------
1. All the binary files are placed in /ondirector/bin directory
   cd /usr/local/ondir_platform/2.0/bin
    controller  ofcli  ofucmd  ofldsvd

2. controller, ofldsvd  and ofucmd are started as part of the start script.

3. To configure controller through CLI, run ofcli.
   Refer ofp-configure.txt file in 2.0/doc directory.

Application Shared Libraries
-----------------------------
1. All application libraries are placed in /usr/local/ondir_platform/2.0/lib/apps
   cd /usr/local/ondir_platform/2.0/lib/apps

LDSV - Default configuration
-----------------------------
1. LDSV will load the default configuration from /ondirector/conf/ofpsdn directory, when the controller is started initially.
2. When the user issued save command in CLI, the configuration is updated and stored in the /usr/local/ondir_platform/2.0/conf/<VERSION_NUM>/ofpsdn.
3. The updated configuration will be loaded when the controller is restarted.
