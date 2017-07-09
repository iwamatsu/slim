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
     
   
   Optionally may require:
   
   - libpam0g  (required really)
   - libck-connector0
   - libdbus-1-3

   (These should be auto-detected and reported at cmake time.)

External Applications
----------------------

For Operations::
  
    + one Window Manager or Desktop Environment to login to.
      - _F1_ Allows choice between installed environments.
    + "console" Requires an  Xterm
    +  _F11_  as screenshot
       - Requires "import" e.g as found in the imagemagick suite.
       - Alt: slim has also well tested with "scrot"

---------------------
  Build and Install  
---------------------

1. Build 
---------

   Edit the CMakefile.txt to adjust libraries and paths to your OS (if needed).
   Note for example:  CMAKE_INSTALL_PREFIX "/usr/local"

2. Default Build 
-----------------
::

    $ mkdir build
    $ cd build
    $ cmake ../

Notes:

The cmake system will *auto include and enable* support for libraries found.
See: output of a default   cmake ../ 

e.g. To Disallow consolkit  *(N.B. and by extension dbus)*::

    $ cmake ../  -DUSE_PAM=yes -DUSE_CONSOLEKIT=no

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
Some notes hints and paths from the wild.

::
    $ make clean 
    $ cmake --help

(in the build directory) 
 
Remove the "build" directory tree (only) to start over.

Path Manifest
-------------
Here is a listing of most of typical installed paths
as output by a run of " make install " ::
   
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



