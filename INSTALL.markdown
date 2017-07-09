** INSTALL file for SLiM ##

### 0. Prerequisites: 
   
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

   A least one Window Manager or Desktop Environment to login to.
   _F1_ Allows choice between installed environments.

   To exploit Login Special Username facilities

    - "console"
      Requires an  Xterm

    - _F11_  as screenshot
      Requires "import" e.g as found in the imagemagick suite.
      or it also tested with "scrot"

### Build and Install

1. to build 
Edit the Makefile to adjust libraries and paths to your OS (if needed)
   e.g.  CMAKE_INSTALL_PREFIX "/usr/local"

Default Build::
    
	$ mkdir build
    $ cd build
    $ cmake ../

The cmake system will _auto include and enable_ support for libs its finds.
see the output of a default cmake.

e.g. Disallow consolkit  (and by extension dbus)::

    $cmake ../  -DUSE_PAM=yes -DUSE_CONSOLEKIT=no

###Simple test

	./slim -v
	
### Install

See:  CMAKE_INSTALL_PREFIX "/usr/local"

    $sudo make install


