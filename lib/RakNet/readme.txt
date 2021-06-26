RakNet 2.52
Copyright 2002-2005 Kevin Jenkins (rakkar@rakkarsoft.com).
This API and the code herein created by and wholly and privately owned by Kevin Jenkins except where specifically indicated otherwise.
Licensed under the "RakNet" brand by Jenkins Software and subject to the terms of the relevant licensing agreement available at http://www.rakkarsoft.com

See Help/revisionlog.html for what has changed.

------------------------------------------
Windows users (Visual Studio 2005)
-----------------------------------------
Load RakNet.sln and convert.
If it doesn't work, see Help/compilersetup.html or the training video at http://www.rakkarsoft.com/raknet/manual/helloworldvideo.html on how to setup the project.

-----------------------------------------
Windows users (.NET 2003)
-----------------------------------------
Load RakNet.sln

-----------------------------------------
Windows users (VC6)
-----------------------------------------
Load RakNet.dsw .  I've gotten mixed reports on if it works or not - this is an auto-generated file.
If it doesn't work, see Help/compilersetup.html or the training video at http://www.rakkarsoft.com/raknet/manual/helloworldvideo.html on how to setup the project.

-----------------------------------------
CYGWIN users
-----------------------------------------
Copy Include, Source, and whatever you want to run in the home directory.  Then type
g++ ../../lib/w32api/libws2_32.a *.cpp
You can run a.exe
You might have to copy *.dll from cygwin\bin as well.

-----------------------------------------
Linux users
-----------------------------------------
Linux build files are community contributed.  I do not support them.  As far as I'm concerned "gcc -c *.cpp" is all you ever need.  If the files are out of date and you know how to fix them,
email me fixes at rakkar@rakkarsoft.com and I will include the fixes in the next version.

- Kevin

What follows is a user note:

These new makefiles are based on the old ones by NathanH, i almost rewrote everything he done but i left some basic stuff, before compile, make sure to check the file makefile.defs, change the INCLUDE_DIR= to your include directory(generally /usr/local/include or /usr/include) and LIBS_DIR to your library directory( usually /usr/lib or /usr/local/lib ).

1-To compile raknet just type "make" on the root directory
2-To install it, log as root ( su ) and type "make install", after install it, check your /etc/ld.so.config(in debian) for the lib directories, if your LIBS_DIR directory isnt there, include it, and run ldconfig after the install process.
3-To compile the linux samples go to the linux sample directories and run make
4-To compile a program using raknet just use -I/usr/local/include(or your INCLUDE_DIR) and -lraknet for linking

by romulo fernandes(abra185@gmail.com) feel
free to distribute it or even modify it. 
Theres no compilation for macOSX present cause i dont have a mac system, if you want to use those, use NathanH makefiles. Thanks.

if you get errors related to speex do one of the following:
1. just download the devel package for speex from your distributions package repository and install that 
2. you could download speex from the speex homepage (google for it) and compile and install that
3. if you don't plan to use the voice part of raknet you can disable it by removing in Makefile the voicestatic and voiceshared from that line:
all: static shared voicestatic voiceshared

the speex distributed with raknet in the zip archive is actually pretty useless for linux users, cause you would need to run configure to get some of the headers built, but configure seams to be broken because it has been on a windows filesystem, where all the flags like "executable" get removed.

updated by Simon Schmeisser (mail_to_wrt@gmx.de)

-----------------------------------------
DevCPP Users
-----------------------------------------
Load RakNet.dev

-----------------------------------------
CodeBlocks Users
-----------------------------------------
Load RakNet.cbp

------------------------------------------
DigitalMars Users
-----------------------------------------
A user contributed file DMCMakefile.win is included in the distribution.  I have no idea if it works or not.  If it doesn't, you can email me updates at rakkar@rakkarsoft.com

-----------------------------------------
Mac Users
-----------------------------------------
From http://www.rakkarsoft.com/raknet/forum/index.php?topic=746.0;topicseen
Open a Terminal window and type: cd ~/Desktop/RakNet/Source

Give the following command:

Code:
gcc -c -I ../Include -isysroot /Developer/SDKs/MacOSX10.3.9.sdk/ -arch ppc *.cpp

The sources should build cleanly. This gives you a bunch of PowerPC binaries, compiled against the 10.3.9 SDK which is a good thing.

Give the following command:

Code:
libtool -static -o raknetppc.a *.o

This will stitch together a static library for the PowerPC architecture. There may be warnings that some .o files do not have any symbols. If you want to be prudent, remove the named files (the .o files, not the .cpp files!) and re-run the libtool command.

Now, we build the source files for Intel:

Code:
gcc -c -I ../Include -isysroot /Developer/SDKs/MacOSX10.4u.sdk/ -arch i386 *.cpp

..and stitch it into a i386 library:

Code:
libtool -static -o rakneti386.a *.o

Now, type:

Code:
ls *.a

which should list the two .a files. Now, we make them into a universal binary:

Code:
lipo -create *.a -o libraknet.a

You now have a file named libraknet.a. This is the RakNet library, built to run on both PowerPC and Intel Macs. Enjoy! ;-)


-----------------------------------------
Other Platforms
-----------------------------------------
The define _COMPATIBILITY_1 while adding Compatibility1Includes.h may work for you.

-----------------------------------------
Package notes
-----------------------------------------
The Source directory contain all files required for the core of Raknet and is used if you want to use the source in your program or create your own dll
The Samples directory contains code samples and one game using an older version of Raknet.  The code samples each demonstrate one feature of Raknet.  The game samples cover several features.
The lib directory contains libs for debug and release versions of RakNet and RakVoice
The Help directory contains index.html, which is full help documentation in HTML format
There is a make file for linux users in the root directory.  Windows users can use projects under Samples\Project Samples

For support please visit
http://www.rakkarsoft.com