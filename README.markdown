## README file for SLiM ##

Nobuhiro Iwamatsu <iwamatsu@nigauri.org>

### INTRODUCTION
    SLiM (Simple Login Manager) is a graphical login manager for X11.
    It aims to be simple, fast and independent from the various
    desktop environments.
    SLiM is based on latest stable release of Login.app by Per Lidén.

####  New features:

       - External themes and configuration
       - PNG support with alpha transparency for panel
       - PNG / JPEG support for backgrounds
       - XFT / freetype support
       - Double or single (GDM-style) inputbox support
       - CMake build procedure

### INSTALLATION

    see the [INSTALL file](https://github.com/PeteGozz/slim/tree/docsatwork/INSTALL.markdown)
    
### USAGE

    To launch slim, execute the slim binary,
    followed by the -d option if you want it to
    run as a daemon in the background (recommended)
    
    Enter username and password to login.

    The ~/.xinitrc file is executed by default,
    so be sure to have a working
    .xinitrc file in your home.
    
    Special "usernames"
    (commands configurable in the config file)::
      - console: start console login
      - exit: exit SLiM
      - halt: halt the system
      - reboot: reboot the system
    
    Pressing the _F11_ key executes a *user-specified command*.
    The default is to take a screenshot if the 'import'
    program is available. 
    (This is configurable in the /etc/slim.conf file) 
    
### CONFIGURATION
    /usr/etc/slim.conf is the main configuration file.
    Options are explained in the file itself
    
### THEMES
    See [THEMES](THEMES.markdown)

### COPYRIGHT

    SLiM is copyright (c) 2004-06 by Simone Rota, Johannes Winkelmann,
    Nobuhiro Iwamatsu and is available under the GNU General Public
    License.
    See the COPYING file for the complete license.

    Image handling code adapted and extended from xplanet 1.0.1,
    copyright (c) 2002-04 by Hari Nair

    Login.app is copyright (c) 1997, 1998 by Per Liden and is 
    licensed through the GNU General Public License. 
