/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "app.h"
#include "numlock.h"
#include "image.h"


extern App* LoginApp;

void CatchSignal(int sig) {
    cerr << APPNAME << ": unexpected signal " << sig << endl;
    LoginApp->StopServer();
    LoginApp->RemoveLock();
    exit(ERR_EXIT);
}


void AlarmSignal(int sig) {
    int pid = LoginApp->GetServerPID();
    if(waitpid(pid, NULL, WNOHANG) == pid) {
        LoginApp->StopServer();
        LoginApp->RemoveLock();
        exit(OK_EXIT);
    }
    signal(sig, AlarmSignal);
    alarm(2);
}


void User1Signal(int sig) {
    signal(sig, User1Signal);
}


App::App(int argc, char** argv) {

    int tmp;
    ServerPID = -1;
    testing = false;

    // Parse command line
    while((tmp = getopt(argc, argv, "vhp:d?")) != EOF) {
        switch (tmp) {
        case 'p':	// Test theme
            testtheme = optarg;
            testing = true;
            if (testtheme == NULL) {
                cerr << "The -p option requires an argument" << endl;
                exit(ERR_EXIT);
            }
            break;
        case 'd':	// Daemon mode
            daemonmode = true;
            break;
        case 'v':	// Version
            std::cout << APPNAME << " version " << VERSION << endl;
            exit(OK_EXIT);
            break;
        case '?':	// Illegal
            cerr << endl;
        case 'h':   // Help
            cerr << "usage:  " << APPNAME << " [option ...]" << endl
            << "options:" << endl
            << "    -d: daemon mode" << endl
            << "    -v: show version" << endl
            << "    -p /path/to/theme/dir: preview theme" << endl;
            exit(OK_EXIT);
            break;
        }
    }

    if (getuid() != 0 && !testing) {
        cerr << APPNAME << ": only root can run this program" << endl;
        exit(ERR_EXIT);
    }

}


void App::Run() {

    DisplayName = DISPLAY;

#ifdef XNEST_DEBUG
    char* p = getenv("DISPLAY");
    if (p && p[0]) {
        DisplayName = p;
        cout << "Using display name " << DisplayName << endl;
    }
#endif

    // Read configuration and theme
    cfg.readConf(CFGFILE);
    string themebase = "";
    string themefile = "";
    string themedir = "";
    themeName = "";
    if (testing) {
        themeName = testtheme;
    } else {
        themebase = string(THEMESDIR) + "/";
        themeName = cfg.getOption("current_theme");
        string::size_type pos;
        if ((pos = themeName.find(",")) != string::npos) {
            // input is a set
            themeName = findValidRandomTheme(themeName);
            if (themeName == "") {
                themeName = "default";
            }
        }
    }

    bool loaded = false;
    while (!loaded) {
        themedir =  themebase + themeName;
        themefile = themedir + THEMESFILE;
        if (!cfg.readConf(themefile)) {
            if (themeName == "default") {
                cerr << APPNAME << ": Failed to open default theme file "
                     << themefile << endl;
                exit(ERR_EXIT);
            } else {
                cerr << APPNAME << ": Invalid theme in config: "
                     << themeName << endl;
                themeName = "default";
            }
        } else {
            loaded = true;
        }
    }

    if (!testing) {
        // Create lock file
        LoginApp->GetLock();

        // Start x-server
        setenv("DISPLAY", DisplayName, 1);
        signal(SIGQUIT, CatchSignal);
        signal(SIGTERM, CatchSignal);
        signal(SIGKILL, CatchSignal);
        signal(SIGINT, CatchSignal);
        signal(SIGHUP, CatchSignal);
        signal(SIGPIPE, CatchSignal);
        signal(SIGUSR1, User1Signal);
        signal(SIGALRM, AlarmSignal);

#ifndef XNEST_DEBUG
        OpenLog();
		
		if (cfg.getOption("daemon") == "yes") {
			daemonmode = true;
		}

        // Daemonize
        if (daemonmode) {
            if (daemon(0, 1) == -1) {
                cerr << APPNAME << ": " << strerror(errno) << endl;
                exit(ERR_EXIT);
            }
        }

        StartServer();
        alarm(2);
#endif

    }

    // Open display
    if((Dpy = XOpenDisplay(DisplayName)) == 0) {
        cerr << APPNAME << ": could not open display '"
             << DisplayName << "'" << endl;
        if (!testing) StopServer();
        exit(ERR_EXIT);
    }

    // Get screen and root window
    Scr = DefaultScreen(Dpy);
    Root = RootWindow(Dpy, Scr);


    // for tests we use a standard window
    if (testing) {
        Window RealRoot = RootWindow(Dpy, Scr);
        Root = XCreateSimpleWindow(Dpy, RealRoot, 0, 0, 640, 480, 0, 0, 0);
        XMapWindow(Dpy, Root);
        XFlush(Dpy);
    } else {
        blankScreen();
    }

    HideCursor();

    // Create panel
    LoginPanel = new Panel(Dpy, Scr, Root, &cfg, themedir);

    // Start looping
    XEvent event;
    int panelclosed = 1;
    int Action;
    bool firstloop = true; // 1st time panel is shown (for automatic username)

    while(1) {
        if(panelclosed) {
            // Init root
            setBackground(themedir);

            // Close all clients
            if (!testing) {
                KillAllClients(False);
                KillAllClients(True);
            }

            // Show panel
            LoginPanel->OpenPanel();
        }

        Action = WAIT;
        LoginPanel->GetInput()->Reset();
        if (firstloop && cfg.getOption("default_user") != "") {
            LoginPanel->GetInput()->SetName(cfg.getOption("default_user") );
            firstloop = false;
        }

        while(Action == WAIT) {
            XNextEvent(Dpy, &event);
            Action = LoginPanel->EventHandler(&event);
        }

        if(Action == FAIL) {
            panelclosed = 0;
            LoginPanel->ClearPanel();
            XBell(Dpy, 100);
        } else {
            // for themes test we just quit
            if (testing) {
                Action = EXIT;
            }
            panelclosed = 1;
            LoginPanel->ClosePanel();

            switch(Action) {
                case LOGIN:
                    Login();
                    break;
                case CONSOLE:
                    Console();
                    break;
                case REBOOT:
                    Reboot();
                    break;
                case HALT:
                    Halt();
                    break;
                case SUSPEND:
                    Suspend();
                    break;
                case EXIT:
                    Exit();
                    break;
            }
        }
    }
}


int App::GetServerPID() {
    return ServerPID;
}

// Hide the cursor
void App::HideCursor() {
	if (cfg.getOption("hidecursor") == "true") {
		XColor		    black;
		char		    cursordata[1];
		Pixmap		    cursorpixmap;
		Cursor		    cursor;
		cursordata[0]=0;
		cursorpixmap=XCreateBitmapFromData(Dpy,Root,cursordata,1,1);
		black.red=0;
		black.green=0;
		black.blue=0;
		cursor=XCreatePixmapCursor(Dpy,cursorpixmap,cursorpixmap,&black,&black,0,0);
		XDefineCursor(Dpy,Root,cursor);
	}
}

void App::Login() {
    struct passwd *pw;
    pid_t pid;

    pw = LoginPanel->GetInput()->GetPasswdStruct();
    if(pw == 0)
        return;

    // Create new process
    pid = fork();
    if(pid == 0) {
        // Login process starts here
        SwitchUser Su(pw, &cfg, DisplayName);
        string session = LoginPanel->getSession();
        string loginCommand = cfg.getOption("login_cmd");
        replaceVariables(loginCommand, SESSION_VAR, session);
        replaceVariables(loginCommand, THEME_VAR, themeName);
        string sessStart = cfg.getOption("sessionstart_cmd");
        if (sessStart != "") {
            replaceVariables(sessStart, USER_VAR, pw->pw_name);
            system(sessStart.c_str());
        }
        Su.Login(loginCommand.c_str());
        exit(OK_EXIT);
    }

#ifndef XNEST_DEBUG
    CloseLog();
#endif

    // Wait until user is logging out (login process terminates)
    pid_t wpid = -1;
    int status;
    while (wpid != pid) {
        wpid = wait(&status);
    }
    if (WIFEXITED(status) && WEXITSTATUS(status)) {
        LoginPanel->Message("Failed to execute login command");
    } else {
         string sessStop = cfg.getOption("sessionstop_cmd");
         if (sessStop != "") {
            replaceVariables(sessStop, USER_VAR, pw->pw_name);
            system(sessStop.c_str());
        }
    }

    // Close all clients
    KillAllClients(False);
    KillAllClients(True);

    // Send HUP signal to clientgroup
    killpg(pid, SIGHUP);

    // Send TERM signal to clientgroup, if error send KILL
    if(killpg(pid, SIGTERM))
    killpg(pid, SIGKILL);

    HideCursor();

#ifndef XNEST_DEBUG
    // Re-activate log file
    OpenLog();
    RestartServer();
#endif


}


void App::Reboot() {
    // Stop alarm clock
    alarm(0);

    // Write message
    LoginPanel->Message((char*)cfg.getOption("reboot_msg").c_str());
    sleep(3);

    // Stop server and reboot
    StopServer();
    RemoveLock();
    system(cfg.getOption("reboot_cmd").c_str());
    exit(OK_EXIT);
}


void App::Halt() {
    // Stop alarm clock
    alarm(0);

    // Write message
    LoginPanel->Message((char*)cfg.getOption("shutdown_msg").c_str());
    sleep(3);

    // Stop server and halt
    StopServer();
    RemoveLock();
    system(cfg.getOption("halt_cmd").c_str());
    exit(OK_EXIT);
}

void App::Suspend() {
    sleep(1);
    system(cfg.getOption("suspend_cmd").c_str());
}


void App::Console() {
    int posx = 40;
    int posy = 40;
    int fontx = 9;
    int fonty = 15;
    int width = (XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posx * 2)) / fontx;
    int height = (XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posy * 2)) / fonty;

    // Execute console
    const char* cmd = cfg.getOption("console_cmd").c_str();
    char *tmp = new char[strlen(cmd) + 60];
    sprintf(tmp, cmd, width, height, posx, posy, fontx, fonty);
    system(tmp);
    delete [] tmp;
}


void App::Exit() {
    if (testing) {
        char* testmsg = "This is a test message :-)";
        LoginPanel->Message(testmsg);
        sleep(3);
    } else {
        delete LoginPanel;
        StopServer();
        RemoveLock();
    }
    exit(OK_EXIT);
}


int CatchErrors(Display *dpy, XErrorEvent *ev) {
    return 0;
}

void App::RestartServer() {
        StopServer(); 
        RemoveLock();
        Run();
} 

void App::KillAllClients(Bool top) {
    Window dummywindow;
    Window *children;
    unsigned int nchildren;
    unsigned int i;
    XWindowAttributes attr;

    XSync(Dpy, 0);
    XSetErrorHandler(CatchErrors);

    nchildren = 0;
    XQueryTree(Dpy, Root, &dummywindow, &dummywindow, &children, &nchildren);
    if(!top) {
        for(i=0; i<nchildren; i++) {
            if(XGetWindowAttributes(Dpy, children[i], &attr) && (attr.map_state == IsViewable))
                children[i] = XmuClientWindow(Dpy, children[i]);
            else
                children[i] = 0;
        }
    }

    for(i=0; i<nchildren; i++) {
        if(children[i])
            XKillClient(Dpy, children[i]);
    }
    XFree((char *)children);

    XSync(Dpy, 0);
    XSetErrorHandler(NULL);
}


int App::ServerTimeout(int timeout, char* text) {
    int	i = 0;
    int pidfound = -1;
    static char	*lasttext;

    for(;;) {
        pidfound = waitpid(ServerPID, NULL, WNOHANG);
        if(pidfound == ServerPID)
            break;
        if(timeout) {
            if(i == 0 && text != lasttext)
                cerr << endl << APPNAME << ": waiting for " << text;
            else
                cerr << ".";
        }
        if(timeout)
            sleep(1);
        if(++i > timeout)
            break;
    }

    if(i > 0)
        cerr << endl;
    lasttext = text;

    return (ServerPID != pidfound);
}


int App::WaitForServer() {
    int	ncycles	 = 120;
    int	cycles;

    for(cycles = 0; cycles < ncycles; cycles++) {
        if((Dpy = XOpenDisplay(DisplayName))) {
            return 1;
        } else {
            if(!ServerTimeout(1, "X server to begin accepting connections"))
                break;
        }
    }

    cerr << "Giving up." << endl;

    return 0;
}


int App::StartServer() {
    ServerPID = vfork();

    static const int MAX_XSERVER_ARGS = 256;
    static char* server[MAX_XSERVER_ARGS+2] = { NULL };
    server[0] = (char *)cfg.getOption("default_xserver").c_str();
    string argOption = cfg.getOption("xserver_arguments");
    char* args = new char[argOption.length()+2]; // NULL plus vt
    strcpy(args, argOption.c_str());

    int argc = 1;
    int pos = 0;
    bool hasVtSet = false;
    while (args[pos] != '\0') {
        if (args[pos] == ' ' || args[pos] == '\t') {
            *(args+pos) = '\0';
            server[argc++] = args+pos+1;
        } else if (pos == 0) {
            server[argc++] = args+pos;
        }
        if (server[argc-1][0] == 'v' && server[argc-1][1] == 't') {
            bool ok = false;
            Cfg::string2int(server[argc-1]+2, &ok);
            if (ok) {
                hasVtSet = true;
            }
        }
        ++pos;

        if (argc+1 >= MAX_XSERVER_ARGS) {
            // ignore _all_ arguments to make sure the server starts at
            // all
            argc = 1;
            break;
        }
    }
    if (!hasVtSet && daemonmode) {
        server[argc++] = "vt07";
    }
    server[argc] = NULL;

    switch(ServerPID) {
    case 0:
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);
        setpgid(0,getpid());


        execvp(server[0], server);
        cerr << APPNAME << ": X server could not be started" << endl;
        exit(ERR_EXIT);
        break;

    case -1:
        break;

    default:
        errno = 0;
        if(!ServerTimeout(0, "")) {
            ServerPID = -1;
            break;
        }
        alarm(15);
        pause();
        alarm(0);

        // Wait for server to start up
        if(WaitForServer() == 0) {
            cerr << APPNAME << ": unable to connect to X server" << endl;
            StopServer();
            ServerPID = -1;
            exit(ERR_EXIT);
        }
        break;
    }

    string numlock = cfg.getOption("numlock");
    if (numlock == "on") {
        NumLock::setOn();
    } else if (numlock == "off") {
        NumLock::setOff();
	}
    
	delete args;

    return ServerPID;
}


jmp_buf CloseEnv;
int IgnoreXIO(Display *d) {
    cerr << APPNAME << ": connection to X server lost." << endl;
    longjmp(CloseEnv, 1);
}


void App::StopServer() {
    // Stop alars clock and ignore signals
    alarm(0);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_DFL);
    signal(SIGKILL, SIG_DFL);
    signal(SIGALRM, SIG_DFL);

    // Catch X error
    XSetIOErrorHandler(IgnoreXIO);
    if(!setjmp(CloseEnv))
        XCloseDisplay(Dpy);

    // Send HUP to process group
    errno = 0;
    if((killpg(getpid(), SIGHUP) != 0) && (errno != ESRCH))
        cerr << APPNAME << ": can't send HUP to process group " << getpid() << endl;

    // Send TERM to server
    if(ServerPID < 0)
        return;
    errno = 0;
    if(killpg(ServerPID, SIGTERM) < 0) {
        if(errno == EPERM) {
            cerr << APPNAME << ": can't kill X server" << endl;
            exit(ERR_EXIT);
        }
        if(errno == ESRCH)
            return;
    }

    // Wait for server to shut down
    if(!ServerTimeout(10, "X server to shut down")) {
        cerr << endl;
        return;
    }

    cerr << endl << APPNAME << ":  X server slow to shut down, sending KILL signal." << endl;

    // Send KILL to server
    errno = 0;
    if(killpg(ServerPID, SIGKILL) < 0) {
        if(errno == ESRCH)
            return;
    }

    // Wait for server to die
    if(ServerTimeout(3, "server to die")) {
        cerr << endl << APPNAME << ": can't kill server" << endl;
        exit(ERR_EXIT);
    }
    cerr << endl;
}


void App::blankScreen()
{
    GC gc = XCreateGC(Dpy, Root, 0, 0);
    XSetForeground(Dpy, gc, BlackPixel(Dpy, Scr));
    XFillRectangle(Dpy, Root, gc, 0, 0,
                   XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)),
                   XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    XFlush(Dpy);
    XFreeGC(Dpy, gc);

}

void App::setBackground(const string& themedir) {
    string filename;
    filename = themedir + "/background.png";
    Image *image = new Image;
    bool loaded = image->Read(filename.c_str());
    if (!loaded){ // try jpeg if png failed
        filename = "";
        filename = themedir + "/background.jpg";
        loaded = image->Read(filename.c_str());
    }
    if (loaded) {
        string bgstyle = cfg.getOption("background_style");
        if (bgstyle == "stretch") {
            image->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        } else if (bgstyle == "tile") {
            image->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        } else if (bgstyle == "center") {
    	    string hexvalue = cfg.getOption("background_color");
            hexvalue = hexvalue.substr(1,6);
    	    image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
        			    hexvalue.c_str());
        } else { // plain color or error
    	    string hexvalue = cfg.getOption("background_color");
            hexvalue = hexvalue.substr(1,6);
    	    image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
        			    hexvalue.c_str());
        }
        Pixmap p = image->createPixmap(Dpy, Scr, Root);
        XSetWindowBackgroundPixmap(Dpy, Root, p);
    }
    XClearWindow(Dpy, Root);

    XFlush(Dpy);
}

// Check if there is a lockfile and a corresponding process
void App::GetLock() {
    std::ifstream lockfile(cfg.getOption("lockfile").c_str());
    if (!lockfile) {
        // no lockfile present, create one
        std::ofstream lockfile(cfg.getOption("lockfile").c_str(), ios_base::out);
        if (!lockfile) {
            cerr << APPNAME << ": Could not create lock file: " << cfg.getOption("lockfile").c_str() << std::endl;
            exit(ERR_EXIT);
        }
        lockfile << getpid() << std::endl;
        lockfile.close();
    } else {
        // lockfile present, read pid from it
        int pid = 0;
        lockfile >> pid;
        lockfile.close();
        if (pid > 0) {
            // see if process with this pid exists
            int ret = kill(pid, 0);
            if (ret == 0 || (ret == -1 && errno == EPERM) ) {
                cerr << APPNAME << ": Another instance of the program is already running with PID " << pid << std::endl;
                exit(0);
            } else {
                cerr << APPNAME << ": Stale lockfile found, removing it" << std::endl;
                std::ofstream lockfile(cfg.getOption("lockfile").c_str(), ios_base::out);
                if (!lockfile) {
                    cerr << APPNAME << ": Could not create new lock file: " << cfg.getOption("lockfile") << std::endl;
                    exit(ERR_EXIT);
                }
                lockfile << getpid() << std::endl;
                lockfile.close();
            }
        }
    }
}

// Remove lockfile and close logs
void App::RemoveLock() {
    remove(cfg.getOption("lockfile").c_str());
}

// Redirect stdout and stderr to log file
void App::OpenLog() {
    FILE *log = fopen (cfg.getOption("logfile").c_str(),"a");
    if (!log) {
        cerr <<  APPNAME << ": Could not accesss log file: " << cfg.getOption("logfile") << endl;
        RemoveLock();
        exit(ERR_EXIT);
    }
    fclose(log);
    freopen (cfg.getOption("logfile").c_str(),"a",stdout);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    freopen (cfg.getOption("logfile").c_str(),"a",stderr);
    setvbuf(stderr, NULL, _IONBF, BUFSIZ);
}


// Relases stdout/err
void App::CloseLog(){
    fclose(stderr);
    fclose(stdout);
}

string App::findValidRandomTheme(const string& set)
{
    // extract random theme from theme set; return empty string on error
    string name = set;
    struct stat buf;

    if (name[name.length()-1] == ',') {
        name = name.substr(0, name.length() - 1);
    }

    srandom(getpid()+time(NULL));

    vector<string> themes;
    string themefile;
    Cfg::split(themes, name, ',');
    do {
        int sel = random() % themes.size();

        name = Cfg::Trim(themes[sel]);
        themefile = string(THEMESDIR) +"/" + name + THEMESFILE;
        if (stat(themefile.c_str(), &buf) != 0) {
            themes.erase(find(themes.begin(), themes.end(), name));
            cerr << APPNAME << ": Invalid theme in config: "
                 << name << endl;
            name = "";
        }
    } while (name == "" && themes.size());
    return name;
}


void App::replaceVariables(string& input,
			   const string& var,
			   const string& value)
{
    string::size_type pos = 0;
    int len = var.size();
    while ((pos = input.find(var, pos)) != string::npos) {
        input = input.substr(0, pos) + value + input.substr(pos+len);
    }
}
