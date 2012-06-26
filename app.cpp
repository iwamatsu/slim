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
#include <stdint.h>
#include <cstring>
#include <cstdio>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "app.h"
#include "numlock.h"
#include "util.h"


#ifdef HAVE_SHADOW
#include <shadow.h>
#endif

using namespace std;

#ifdef USE_PAM
#include <string>

int conv(int num_msg, const struct pam_message **msg,
         struct pam_response **resp, void *appdata_ptr){
    *resp = (struct pam_response *) calloc(num_msg, sizeof(struct pam_response));
    Panel* panel = *static_cast<Panel**>(appdata_ptr);
    int result = PAM_SUCCESS;
    for (int i=0; i<num_msg; i++){
        (*resp)[i].resp=0;
        (*resp)[i].resp_retcode=0;
        switch(msg[i]->msg_style){
            case PAM_PROMPT_ECHO_ON:
                // We assume PAM is asking for the username
                panel->EventHandler(Panel::Get_Name);
                switch(panel->getAction()){
                    case Panel::Suspend:
                    case Panel::Halt:
                    case Panel::Reboot:
                        (*resp)[i].resp=strdup("root");
                        break;

                    case Panel::Console:
                    case Panel::Exit:
                    case Panel::Login:
                        (*resp)[i].resp=strdup(panel->GetName().c_str());
                        break;
                }
                break;

            case PAM_PROMPT_ECHO_OFF:
                // We assume PAM is asking for the password
                switch(panel->getAction()){
                    case Panel::Console:
                    case Panel::Exit:
                        // We should leave now!
                        result=PAM_CONV_ERR;
                        break;

                    default:
                        panel->EventHandler(Panel::Get_Passwd);
                        (*resp)[i].resp=strdup(panel->GetPasswd().c_str());
                        break;
                }
                break;

            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
                // We simply write these to the log
                // TODO: Maybe we should simply ignore them
                logStream << APPNAME << ": " << msg[i]->msg << endl;
                break;
        }
        if (result!=PAM_SUCCESS) break;
    }
    if (result!=PAM_SUCCESS){
        for (int i=0; i<num_msg; i++){
            if ((*resp)[i].resp==0) continue;
            free((*resp)[i].resp);
            (*resp)[i].resp=0;
        };
        free(*resp);
        *resp=0;
    };
    return result;
}
#endif

extern App* LoginApp;

int xioerror(Display *disp) {
	LoginApp->RestartServer();
    return 0;
}

void CatchSignal(int sig) {
    logStream << APPNAME << ": unexpected signal " << sig << endl;

    if (LoginApp->isServerStarted())
        LoginApp->StopServer();

    LoginApp->RemoveLock();
    exit(ERR_EXIT);
}

void User1Signal(int sig) {
    signal(sig, User1Signal);
}


#ifdef USE_PAM
App::App(int argc, char** argv)
  : pam(conv, static_cast<void*>(&LoginPanel)),
#else
App::App(int argc, char** argv)
  :
#endif
    mcookiesize(32)		// Must be divisible by 4
{
    int tmp;
    ServerPID = -1;
    testing = false;
    serverStarted = false;
    mcookie = string(App::mcookiesize, 'a');
    daemonmode = false;
    force_nodaemon = false;
    firstlogin = true;
    Dpy = NULL;

    // Parse command line
    // Note: we force a option for nodaemon switch to handle "-nodaemon"
    while((tmp = getopt(argc, argv, "vhp:n:d?")) != EOF) {
        switch (tmp) {
        case 'p':    // Test theme
            testtheme = optarg;
            testing = true;
            if (testtheme == NULL) {
                logStream << "The -p option requires an argument" << endl;
                exit(ERR_EXIT);
            }
            break;
        case 'd':    // Daemon mode
            daemonmode = true;
            break;
        case 'n':    // Daemon mode
            daemonmode = false;
            force_nodaemon = true;
            break;
        case 'v':    // Version
            std::cout << APPNAME << " version " << VERSION << endl;
            exit(OK_EXIT);
            break;
        case '?':    // Illegal
            logStream << endl;
        case 'h':   // Help
            logStream << "usage:  " << APPNAME << " [option ...]" << endl
            << "options:" << endl
            << "    -d: daemon mode" << endl
            << "    -nodaemon: no-daemon mode" << endl
            << "    -v: show version" << endl
            << "    -p /path/to/theme/dir: preview theme" << endl;
            exit(OK_EXIT);
            break;
        }
    }
#ifndef XNEST_DEBUG 
    if (getuid() != 0 && !testing) {
        logStream << APPNAME << ": only root can run this program" << endl;
        exit(ERR_EXIT);
    }
#endif /* XNEST_DEBUG */

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
    cfg = new Cfg;
    cfg->readConf(CFGFILE);
    string themebase = "";
    string themefile = "";
    string themedir = "";
    themeName = "";
    if (testing) {
        themeName = testtheme;
    } else {
        themebase = string(THEMESDIR) + "/";
        themeName = cfg->getOption("current_theme");
        string::size_type pos;
        if ((pos = themeName.find(",")) != string::npos) {
            // input is a set
            themeName = findValidRandomTheme(themeName);
            if (themeName == "") {
                themeName = "default";
            }
        }
    }

#ifdef USE_PAM
    try{
        pam.start("slim");
        pam.set_item(PAM::Authenticator::TTY, DisplayName);
        pam.set_item(PAM::Authenticator::Requestor, "root");
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
        exit(ERR_EXIT);
    };
#endif

    bool loaded = false;
    while (!loaded) {
        themedir =  themebase + themeName;
        themefile = themedir + THEMESFILE;
        if (!cfg->readConf(themefile)) {
            if (themeName == "default") {
                logStream << APPNAME << ": Failed to open default theme file "
                     << themefile << endl;
                exit(ERR_EXIT);
            } else {
                logStream << APPNAME << ": Invalid theme in config: "
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

#ifndef XNEST_DEBUG
        if (!force_nodaemon && cfg->getOption("daemon") == "yes") {
            daemonmode = true;
        }

        // Daemonize
        if (daemonmode) {
            if (daemon(0, 0) == -1) {
                logStream << APPNAME << ": " << strerror(errno) << endl;
                exit(ERR_EXIT);
            }
        }

        OpenLog();

        if (daemonmode)
            UpdatePid();

        CreateServerAuth();
        StartServer();
#endif

    }

    // Open display
    if((Dpy = XOpenDisplay(DisplayName)) == 0) {
        logStream << APPNAME << ": could not open display '"
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
        Root = XCreateSimpleWindow(Dpy, RealRoot, 0, 0, 1280, 1024, 0, 0, 0);
        XMapWindow(Dpy, Root);
        XFlush(Dpy);
    } else {
        blankScreen();
    }

    HideCursor();

    // Create panel
    LoginPanel = new Panel(Dpy, Scr, Root, cfg, themedir);
    bool firstloop = true; // 1st time panel is shown (for automatic username)
    bool focuspass = cfg->getOption("focus_password")=="yes";
    bool autologin = cfg->getOption("auto_login")=="yes";
    
    if (firstlogin && cfg->getOption("default_user") != "") {
        LoginPanel->SetName(cfg->getOption("default_user") );
        #ifdef USE_PAM
	pam.set_item(PAM::Authenticator::User, cfg->getOption("default_user").c_str());
	#endif
        firstlogin = false;
        if (autologin) {
            Login();
        }
    }

    // Set NumLock
    string numlock = cfg->getOption("numlock");
    if (numlock == "on") {
        NumLock::setOn(Dpy);
    } else if (numlock == "off") {
        NumLock::setOff(Dpy);
    }
    
    // Start looping
    int panelclosed = 1;
    Panel::ActionType Action;

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

        LoginPanel->Reset();
	
	
        if (firstloop && cfg->getOption("default_user") != "") {
            LoginPanel->SetName(cfg->getOption("default_user") );
        }


        if (!AuthenticateUser(focuspass && firstloop)){
            panelclosed = 0;
            firstloop = false;
            LoginPanel->ClearPanel();
            XBell(Dpy, 100);
            continue;
        }
	
	firstloop = false;

        Action = LoginPanel->getAction();
        // for themes test we just quit
        if (testing) {
            Action = Panel::Exit;
        }
        panelclosed = 1;
        LoginPanel->ClosePanel();

        switch(Action) {
            case Panel::Login:
                Login();
                break;
            case Panel::Console:
                Console();
                break;
            case Panel::Reboot:
                Reboot();
                break;
            case Panel::Halt:
                Halt();
                break;
            case Panel::Suspend:
                Suspend();
                break;
            case Panel::Exit:
                Exit();
                break;
        }
    }
}

#ifdef USE_PAM
bool App::AuthenticateUser(bool focuspass){
    // Reset the username
    try{
        if (!focuspass)
		    pam.set_item(PAM::Authenticator::User, 0);
        pam.authenticate();
    }
    catch(PAM::Auth_Exception& e){
        switch(LoginPanel->getAction()){
            case Panel::Exit:
            case Panel::Console:
                return true; // <--- This is simply fake!
            default:
                break;
        };
        logStream << APPNAME << ": " << e << endl;
        return false;
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
        exit(ERR_EXIT);
    };
    return true;
}
#else
bool App::AuthenticateUser(bool focuspass){
    if (!focuspass){
        LoginPanel->EventHandler(Panel::Get_Name);
        switch(LoginPanel->getAction()){
            case Panel::Exit:
            case Panel::Console:
                logStream << APPNAME << ": Got a special command (" << LoginPanel->GetName() << ")" << endl;
                return true; // <--- This is simply fake!
            default:
                break;
        }
    }
    LoginPanel->EventHandler(Panel::Get_Passwd);
    
    char *encrypted, *correct;
    struct passwd *pw;

    switch(LoginPanel->getAction()){
        case Panel::Suspend:
        case Panel::Halt:
        case Panel::Reboot:
            pw = getpwnam("root");
            break;
        case Panel::Console:
        case Panel::Exit:
        case Panel::Login:
            pw = getpwnam(LoginPanel->GetName().c_str());
            break;
    }
    endpwent();
    if(pw == 0)
        return false;

#ifdef HAVE_SHADOW
    struct spwd *sp = getspnam(pw->pw_name);    
    endspent();
    if(sp)
        correct = sp->sp_pwdp;
    else
#endif        // HAVE_SHADOW
        correct = pw->pw_passwd;

    if(correct == 0 || correct[0] == '\0')
        return true;

    encrypted = crypt(LoginPanel->GetPasswd().c_str(), correct);
    return ((strcmp(encrypted, correct) == 0) ? true : false);
}
#endif


int App::GetServerPID() {
    return ServerPID;
}

// Hide the cursor
void App::HideCursor() {
    if (cfg->getOption("hidecursor") == "true") {
        XColor            black;
        char            cursordata[1];
        Pixmap            cursorpixmap;
        Cursor            cursor;
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

#ifdef USE_PAM
    try{
        pam.open_session();
        pw = getpwnam(static_cast<const char*>(pam.get_item(PAM::Authenticator::User)));
    }
    catch(PAM::Cred_Exception& e){
        // Credentials couldn't be established
        logStream << APPNAME << ": " << e << endl;
        return;
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
        exit(ERR_EXIT);
    };
#else
    pw = getpwnam(LoginPanel->GetName().c_str());
#endif
    endpwent();
    if(pw == 0)
        return;
    if (pw->pw_shell[0] == '\0') {
        setusershell();
        strcpy(pw->pw_shell, getusershell());
        endusershell();
    }

    // Setup the environment
    char* term = getenv("TERM");
    string maildir = _PATH_MAILDIR;
    maildir.append("/");
    maildir.append(pw->pw_name);
    string xauthority = pw->pw_dir;
    xauthority.append("/.Xauthority");
    
#ifdef USE_PAM
    // Setup the PAM environment
    try{
        if(term) pam.setenv("TERM", term);
        pam.setenv("HOME", pw->pw_dir);
        pam.setenv("PWD", pw->pw_dir);
        pam.setenv("SHELL", pw->pw_shell);
        pam.setenv("USER", pw->pw_name);
        pam.setenv("LOGNAME", pw->pw_name);
        pam.setenv("PATH", cfg->getOption("default_path").c_str());
        pam.setenv("DISPLAY", DisplayName);
        pam.setenv("MAIL", maildir.c_str());
        pam.setenv("XAUTHORITY", xauthority.c_str());
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
        exit(ERR_EXIT);
    }
#endif

#ifdef USE_CONSOLEKIT
    // Setup the ConsoleKit session
    try {
        ck.open_session(DisplayName, pw->pw_uid);
    }
    catch(Ck::Exception &e) {
        logStream << APPNAME << ": " << e << endl;
        exit(ERR_EXIT);
    }
#endif

    // Create new process
    pid = fork();
    if(pid == 0) {
#ifdef USE_PAM
        // Get a copy of the environment and close the child's copy
        // of the PAM-handle.
        char** child_env = pam.getenvlist();

# ifdef USE_CONSOLEKIT
        char** old_env = child_env;

        // Grow the copy of the environment for the session cookie
        int n;
        for(n = 0; child_env[n] != NULL ; n++);

        n++;

        child_env = static_cast<char**>(malloc(sizeof(char*)*n));
        memcpy(child_env, old_env, sizeof(char*)*n);
        child_env[n - 1] = StrConcat("XDG_SESSION_COOKIE=", ck.get_xdg_session_cookie());
        child_env[n] = NULL;
# endif /* USE_CONSOLEKIT */

        pam.end();
#else

# ifdef USE_CONSOLEKIT
        const int Num_Of_Variables = 12; // Number of env. variables + 1
# else
        const int Num_Of_Variables = 11; // Number of env. variables + 1
# endif /* USE_CONSOLEKIT */
        char** child_env = static_cast<char**>(malloc(sizeof(char*)*Num_Of_Variables));
        int n = 0;
        if(term) child_env[n++]=StrConcat("TERM=", term);
        child_env[n++]=StrConcat("HOME=", pw->pw_dir);
        child_env[n++]=StrConcat("PWD=", pw->pw_dir);
        child_env[n++]=StrConcat("SHELL=", pw->pw_shell);
        child_env[n++]=StrConcat("USER=", pw->pw_name);
        child_env[n++]=StrConcat("LOGNAME=", pw->pw_name);
        child_env[n++]=StrConcat("PATH=", cfg->getOption("default_path").c_str());
        child_env[n++]=StrConcat("DISPLAY=", DisplayName);
        child_env[n++]=StrConcat("MAIL=", maildir.c_str());
        child_env[n++]=StrConcat("XAUTHORITY=", xauthority.c_str());
# ifdef USE_CONSOLEKIT
        child_env[n++]=StrConcat("XDG_SESSION_COOKIE=", ck.get_xdg_session_cookie());
# endif /* USE_CONSOLEKIT */
        child_env[n++]=0;

#endif

        // Login process starts here
        SwitchUser Su(pw, cfg, DisplayName, child_env);
        string session = LoginPanel->getSession();
        string loginCommand = cfg->getOption("login_cmd");
        replaceVariables(loginCommand, SESSION_VAR, session);
        replaceVariables(loginCommand, THEME_VAR, themeName);
        string sessStart = cfg->getOption("sessionstart_cmd");
        if (sessStart != "") {
            replaceVariables(sessStart, USER_VAR, pw->pw_name);
            system(sessStart.c_str());
        }
        Su.Login(loginCommand.c_str(), mcookie.c_str());
        _exit(OK_EXIT);
    }

#ifndef XNEST_DEBUG
    CloseLog();
#endif

    // Wait until user is logging out (login process terminates)
    pid_t wpid = -1;
    int status;
    while (wpid != pid) {
        wpid = wait(&status);
		if (wpid == ServerPID)
			xioerror(Dpy);	// Server died, simulate IO error
    }
    if (WIFEXITED(status) && WEXITSTATUS(status)) {
        LoginPanel->Message("Failed to execute login command");
        sleep(3);
    } else {
         string sessStop = cfg->getOption("sessionstop_cmd");
         if (sessStop != "") {
            replaceVariables(sessStop, USER_VAR, pw->pw_name);
            system(sessStop.c_str());
        }
    }

#ifdef USE_CONSOLEKIT
    try {
        ck.close_session();
    }
    catch(Ck::Exception &e) {
        logStream << APPNAME << ": " << e << endl;
    };
#endif

#ifdef USE_PAM
    try{
        pam.close_session();
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
    };
#endif

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
#ifdef USE_PAM
    try{
        pam.end();
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
    };
#endif

    // Write message
    LoginPanel->Message((char*)cfg->getOption("reboot_msg").c_str());
    sleep(3);

    // Stop server and reboot
    StopServer();
    RemoveLock();
    system(cfg->getOption("reboot_cmd").c_str());
    exit(OK_EXIT);
}


void App::Halt() {
#ifdef USE_PAM
    try{
        pam.end();
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
    };
#endif

    // Write message
    LoginPanel->Message((char*)cfg->getOption("shutdown_msg").c_str());
    sleep(3);

    // Stop server and halt
    StopServer();
    RemoveLock();
    system(cfg->getOption("halt_cmd").c_str());
    exit(OK_EXIT);
}

void App::Suspend() {
    sleep(1);
    system(cfg->getOption("suspend_cmd").c_str());
}


void App::Console() {
    int posx = 40;
    int posy = 40;
    int fontx = 9;
    int fonty = 15;
    int width = (XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posx * 2)) / fontx;
    int height = (XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)) - (posy * 2)) / fonty;

    // Execute console
    const char* cmd = cfg->getOption("console_cmd").c_str();
    char *tmp = new char[strlen(cmd) + 60];
    sprintf(tmp, cmd, width, height, posx, posy, fontx, fonty);
    system(tmp);
    delete [] tmp;
}


void App::Exit() {
#ifdef USE_PAM
    try{
        pam.end();
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
    };
#endif

    if (testing) {
        const char* testmsg = "This is a test message :-)";
        LoginPanel->Message(testmsg);
        sleep(3);
        delete LoginPanel;
        XCloseDisplay(Dpy);
    } else {
        delete LoginPanel;
        StopServer();
        RemoveLock();
    }
    delete cfg;
    exit(OK_EXIT);
}


int CatchErrors(Display *dpy, XErrorEvent *ev) {
    return 0;
}

void App::RestartServer() {
#ifdef USE_PAM
    try{
        pam.end();
    }
    catch(PAM::Exception& e){
        logStream << APPNAME << ": " << e << endl;
    };
#endif

    StopServer(); 
    RemoveLock();
	while (waitpid(-1, NULL, WNOHANG) > 0); // Collects all dead childrens
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
    int    i = 0;
    int pidfound = -1;
    static char    *lasttext;

    for(;;) {
        pidfound = waitpid(ServerPID, NULL, WNOHANG);
        if(pidfound == ServerPID)
            break;
        if(timeout) {
            if(i == 0 && text != lasttext)
                logStream << endl << APPNAME << ": waiting for " << text;
            else
                logStream << ".";
        }
        if(timeout)
            sleep(1);
        if(++i > timeout)
            break;
    }

    if(i > 0)
        logStream << endl;
    lasttext = text;

    return (ServerPID != pidfound);
}


int App::WaitForServer() {
    int    ncycles     = 120;
    int    cycles;

    for(cycles = 0; cycles < ncycles; cycles++) {
        if((Dpy = XOpenDisplay(DisplayName))) {
            XSetIOErrorHandler(xioerror);
            return 1;
        } else {
            if(!ServerTimeout(1, (char *) "X server to begin accepting connections"))
                break;
        }
    }

    logStream << "Giving up." << endl;

    return 0;
}


int App::StartServer() {
    ServerPID = fork();

    static const int MAX_XSERVER_ARGS = 256;
    static char* server[MAX_XSERVER_ARGS+2] = { NULL };
    server[0] = (char *)cfg->getOption("default_xserver").c_str();
    string argOption = cfg->getOption("xserver_arguments");
    /* Add mandatory -xauth option */
    argOption = argOption + " -auth " + cfg->getOption("authfile");
    char* args = new char[argOption.length()+2]; // NULL plus vt
    strcpy(args, argOption.c_str());

    serverStarted = false;

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
        ++pos;

        if (argc+1 >= MAX_XSERVER_ARGS) {
            // ignore _all_ arguments to make sure the server starts at
            // all
            argc = 1;
            break;
        }
    }

    for (int i=0; i<argc; i++) {
        if (server[i][0] == 'v' && server[i][1] == 't') {
            bool ok = false;
            Cfg::string2int(server[i]+2, &ok);
            if (ok) {
                hasVtSet = true;
            }
        }
    }

    if (!hasVtSet && daemonmode) {
        server[argc++] = (char*)"vt07";
    }
    server[argc] = NULL;

    switch(ServerPID) {
    case 0:
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);
        setpgid(0,getpid());


        execvp(server[0], server);
        logStream << APPNAME << ": X server could not be started" << endl;
        exit(ERR_EXIT);
        break;

    case -1:
        break;

    default:
        errno = 0;
        if(!ServerTimeout(0, (char *)"")) {
            ServerPID = -1;
            break;
        }

        // Wait for server to start up
        if(WaitForServer() == 0) {
            logStream << APPNAME << ": unable to connect to X server" << endl;
            StopServer();
            ServerPID = -1;
            exit(ERR_EXIT);
        }
        break;
    }

    delete args;

    serverStarted = true;

    return ServerPID;
}


jmp_buf CloseEnv;
int IgnoreXIO(Display *d) {
    logStream << APPNAME << ": connection to X server lost." << endl;
    longjmp(CloseEnv, 1);
}


void App::StopServer() {
    signal(SIGQUIT, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SIG_DFL);
    signal(SIGKILL, SIG_DFL);

    // Catch X error
    XSetIOErrorHandler(IgnoreXIO);
    if(!setjmp(CloseEnv) && Dpy)
        XCloseDisplay(Dpy);

    // Send HUP to process group
    errno = 0;
    if((killpg(getpid(), SIGHUP) != 0) && (errno != ESRCH))
        logStream << APPNAME << ": can't send HUP to process group " << getpid() << endl;

    // Send TERM to server
    if(ServerPID < 0)
        return;
    errno = 0;
    if(killpg(ServerPID, SIGTERM) < 0) {
        if(errno == EPERM) {
            logStream << APPNAME << ": can't kill X server" << endl;
            exit(ERR_EXIT);
        }
        if(errno == ESRCH)
            return;
    }

    // Wait for server to shut down
    if(!ServerTimeout(10, (char *)"X server to shut down")) {
        logStream << endl;
        return;
    }

    logStream << endl << APPNAME << ":  X server slow to shut down, sending KILL signal." << endl;

    // Send KILL to server
    errno = 0;
    if(killpg(ServerPID, SIGKILL) < 0) {
        if(errno == ESRCH)
            return;
    }

    // Wait for server to die
    if(ServerTimeout(3, (char*)"server to die")) {
        logStream << endl << APPNAME << ": can't kill server" << endl;
        exit(ERR_EXIT);
    }
    logStream << endl;
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
    image = new Image;
    bool loaded = image->Read(filename.c_str());
    if (!loaded){ // try jpeg if png failed
        filename = "";
        filename = themedir + "/background.jpg";
        loaded = image->Read(filename.c_str());
    }
    if (loaded) {
        string bgstyle = cfg->getOption("background_style");
        if (bgstyle == "stretch") {
            image->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        } else if (bgstyle == "tile") {
            image->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
        } else if (bgstyle == "center") {
            string hexvalue = cfg->getOption("background_color");
            hexvalue = hexvalue.substr(1,6);
            image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
                        hexvalue.c_str());
        } else { // plain color or error
            string hexvalue = cfg->getOption("background_color");
            hexvalue = hexvalue.substr(1,6);
            image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
                        hexvalue.c_str());
        }
        Pixmap p = image->createPixmap(Dpy, Scr, Root);
        XSetWindowBackgroundPixmap(Dpy, Root, p);
    }
    XClearWindow(Dpy, Root);
    
    XFlush(Dpy);
    delete image;
}

// Check if there is a lockfile and a corresponding process
void App::GetLock() {
    std::ifstream lockfile(cfg->getOption("lockfile").c_str());
    if (!lockfile) {
        // no lockfile present, create one
        std::ofstream lockfile(cfg->getOption("lockfile").c_str(), ios_base::out);
        if (!lockfile) {
            logStream << APPNAME << ": Could not create lock file: " << cfg->getOption("lockfile").c_str() << std::endl;
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
                logStream << APPNAME << ": Another instance of the program is already running with PID " << pid << std::endl;
                exit(0);
            } else {
                logStream << APPNAME << ": Stale lockfile found, removing it" << std::endl;
                std::ofstream lockfile(cfg->getOption("lockfile").c_str(), ios_base::out);
                if (!lockfile) {
                    logStream << APPNAME << ": Could not create new lock file: " << cfg->getOption("lockfile") << std::endl;
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
    remove(cfg->getOption("lockfile").c_str());
}

// Get server start check flag.
bool App::isServerStarted() {
    return serverStarted;
}

// Redirect stdout and stderr to log file
void App::OpenLog() {

    if ( !logStream.openLog( cfg->getOption("logfile").c_str() ) ) {
        logStream <<  APPNAME << ": Could not accesss log file: " << cfg->getOption("logfile") << endl;
        RemoveLock();
        exit(ERR_EXIT);
    }
    //
    // I should set the buffers to imediate write, but I just flush on every
    // << operation.
    //
    //
}

// Relases stdout/err
void App::CloseLog(){
	// Simply closing the log
	logStream.closeLog();
}

string App::findValidRandomTheme(const string& set)
{
    // extract random theme from theme set; return empty string on error
    string name = set;
    struct stat buf;

    if (name[name.length()-1] == ',') {
        name = name.substr(0, name.length() - 1);
    }

    Util::srandom(Util::makeseed());

    vector<string> themes;
    string themefile;
    Cfg::split(themes, name, ',');
    do {
        int sel = Util::random() % themes.size();

        name = Cfg::Trim(themes[sel]);
        themefile = string(THEMESDIR) +"/" + name + THEMESFILE;
        if (stat(themefile.c_str(), &buf) != 0) {
            themes.erase(find(themes.begin(), themes.end(), name));
            logStream << APPNAME << ": Invalid theme in config: "
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


/*
 * We rely on the fact that all bits generated by Util::random()
 * are usable, so we are taking full words from its output.
 */
void App::CreateServerAuth() {
    /* create mit cookie */
    uint16_t word;
    uint8_t hi, lo;
    int i;
    string authfile;
    const char *digits = "0123456789abcdef";
    Util::srandom(Util::makeseed());
    for (i = 0; i < App::mcookiesize; i+=4) {
        word = Util::random() & 0xffff;
        lo = word & 0xff;
        hi = word >> 8;
        mcookie[i] = digits[lo & 0x0f];
        mcookie[i+1] = digits[lo >> 4];
        mcookie[i+2] = digits[hi & 0x0f];
        mcookie[i+3] = digits[hi >> 4];
    }
    /* reinitialize auth file */
    authfile = cfg->getOption("authfile");
    remove(authfile.c_str());
    putenv(StrConcat("XAUTHORITY=", authfile.c_str()));
    Util::add_mcookie(mcookie, ":0", cfg->getOption("xauth_path"),
      authfile);
}

char* App::StrConcat(const char* str1, const char* str2) {
    char* tmp = new char[strlen(str1) + strlen(str2) + 1];
    strcpy(tmp, str1);
    strcat(tmp, str2);
    return tmp;
}

void App::UpdatePid() {
    std::ofstream lockfile(cfg->getOption("lockfile").c_str(), ios_base::out);
    if (!lockfile) {
        logStream << APPNAME << ": Could not update lock file: " << cfg->getOption("lockfile").c_str() << std::endl;
        exit(ERR_EXIT);
    }
    lockfile << getpid() << std::endl;
    lockfile.close();
}
