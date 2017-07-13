---------------------------
   INSTALL file for SLiM 
---------------------------

Prerequisites:
--------------
   
   - cmake
   - X.org or XFree86
   - libxmu
   - libpng
   - libjpeg
     
   
   Optionally may also require:
   
   - libpam0g  (required really)
   - libck-connector0
   - libdbus-1-3

   (These should be auto-detected cached and reported at _initial_ cmake time.)

External Applications
----------------------

For Operations::
  
    - one Window Manager or Desktop Environment to login to.
      + _F1_ Allows choice between installed environments.
    - "console" Requires an  Xterm
    -  _F11_  as screenshot
       + Requires "import" e.g as found in the imagemagick suite.
       + Alt: slim has also well tested with "scrot"

---------------------
  Build and Install
---------------------

1. Build 
---------

   Optionally edit the CMakefile.txt to adjust libraries and paths 
   to your Operating System (if needed).
   For example:  CMAKE_INSTALL_PREFIX "/usr/local"

2. Default Build 
-----------------
::

    $ mkdir build
    $ cd build
    $ cmake ../

Notes:
The above should be a reasonable first test.
This incantation should get you a full build.
With ConsoleKit and Desktop Bus support.

    $ cmake ../ -DUSE_PAM=yes -DUSE_CONSOLEKIT=yes   

To Disallow consolkit  *(N.B. and by extension dbus)*
and NOT build shared libraries::

    $ cmake ../ -DUSE_PAM=yes -DUSE_CONSOLEKIT=no -DBUILD_SHARED_LIBS=no
    
After you have configured your make files run make::

    $ make
    $ ls 

3. Simple test
--------------

   Test the freshly made executable::
   
		$ ./slim -v
	
4. Installation 
----------------

The GNU Makefile produced by cmake has some interesting targets.
Also Consider current CMake settings:  CMAKE_INSTALL_PREFIX "/usr/local"
** $ sudo make install/local  **  (may be reassuring.)

Then there is the classic::
  
  $ sudo make install

Test the new system theme *from a running X session*::
  
  $ slim -p /usr/local/share/slim/themes/default

5. Other 
---------
Some notes hints and paths from the wild::

     $ make clean 
     $ cmake --help
     $ make local/install
	 
(all run from the build directory) 
 
Remove the "build" directory tree (only) to start over.

Path Manifest
-------------
Here is a listing of typical installed paths
as output by an "updating" run of " make install " 
Notice only rebuilt files are installed ::
   
    Install the project...
    /usr/bin/cmake -P cmake_install.cmake
    -- Install configuration: ""
    -- Up-to-date: /usr/local/bin/slim
    -- Up-to-date: /usr/local/bin/slimlock
    -- Up-to-date: /usr/local/lib/libslim.so.1.3.6
    -- Up-to-date: /usr/local/lib/libslim.so
    -- Up-to-date: /usr/local/share/man/man1/slim.1
    -- Up-to-date: /usr/local/share/man/man1/slimlock.1
    -- Up-to-date: /etc/slim.conf
    -- Up-to-date: /lib/systemd/system/slim.service
    -- Installing: /usr/local/share/slim/themes/default/slim.theme
    -- Installing: /usr/local/share/slim/themes/default/panel.png
    -- Installing: /usr/local/share/slim/themes/default/background.jpg


Linked Libraries
................
 This is a typical linkage string passed to the compiler
 for the final slim (elf) executable.

""
-lfontconfig -lpam -ldbus-1 -lck-connector -lm -lrt -lcrypt -lX11 -lXft -lXrender -lXrandr -lXmu -lfreetype -ljpeg -lpng -lz libslim.so.1.3.6 -ljpeg -lpng -lz
""

$ ldd slim

$ file slim

 may approximate::

     ELF 32-bit LSB shared object, Intel 80386, version 1 (SYSV),
     dynamically linked, interpreter /lib/ld-linux.so.2,
     for GNU/Linux 2.6.32,
     BuildID[sha1]=b44698a3baf559d0a79e517221c0ad6cea2b5504, not stripped