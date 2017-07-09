---------------------------
## INSTALL file for SLiM ##
---------------------------

### 0. Prerequisites:
---------------------
   
   - cmake
   - X.org or XFree86
   - libxmu
   - libpng
   - libjpeg
     
   
   Optionally may require:
   
   - libpam0g  (required really)
   - libck-connector0
   - libdbus-1-3

   (These should be autodetected and reported at cmake time.)

#### External Applications
--------------------------

A least one Window Manager or Desktop Environment to login to.
_F1_ Allows choice between installed environments.

To exploit Login Special Username facilities::

    - "console"
      Requires an  Xterm

    - _F11_  as screenshot
      Requires "import" e.g as found in the imagemagick suite.
      slim has also well tested with "scrot"

---------------------
### Build and Install
---------------------

1. Build: 

   Edit the CMakefile.txt to adjust libraries and paths to your OS (if needed).
   Note for example:  CMAKE_INSTALL_PREFIX "/usr/local"

2. Default Build::
    
    $ mkdir build
    $ cd build
    $ cmake ../

Notes:

The cmake system will *auto include and enable* support for libraries found.
See: output of a default   cmake ../ 

e.g. To Disallow consolkit  *(N.B. and by extension dbus)*::

    $ cmake ../  -DUSE_PAM=yes -DUSE_CONSOLEKIT=no

3. Simple test

   Test the freshly made executeable::
	./slim -v
	
4. Install
   
   Consider current settings:  CMAKE_INSTALL_PREFIX "/usr/local"

   ::
      $ sudo make install
      

      
Others
------
::
    $ make clean 
    $ cmake --help

(in the build directory) 
 
Remove the "build" directory tree (only) to start over.


- the installed paths to slim.conf and slim.theme.
- you may need to hand uninstall these. 



