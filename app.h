/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _APP_H_
#define _APP_H_

#include <X11/Xlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <setjmp.h>
#include <stdlib.h>
#include <iostream>
#include "panel.h"
#include "cfg.h"
#include "image.h"

#ifdef USE_PAM
#include "PAM.h"
#endif
#ifdef USE_CONSOLEKIT
#include "Ck.h"
#endif

class App {
public:
    App(int argc, char** argv);
    ~App();
    void Run();
    int GetServerPID();
    void RestartServer();
    void StopServer();

    // Lock functions
    void GetLock();
    void RemoveLock();

    bool isServerStarted();

private:
    void Login();
    void Reboot();
    void Halt();
    void Suspend();
    void Console();
    void Exit();
    void KillAllClients(Bool top);
    void ReadConfig();
    void OpenLog();
    void CloseLog();
    void HideCursor();
    void CreateServerAuth();
    char* StrConcat(const char* str1, const char* str2);
    void UpdatePid();

    bool AuthenticateUser(bool focuspass);
 
    static std::string findValidRandomTheme(const std::string& set);
    static void replaceVariables(std::string& input,
                                 const std::string& var,
                                 const std::string& value);

    // Server functions
    int StartServer();
    int ServerTimeout(int timeout, char *string);
    int WaitForServer();

    // Private data
    Window Root;
    Display* Dpy;
    int Scr;
    Panel* LoginPanel;
    int ServerPID;
    const char* DisplayName;
    bool serverStarted;

#ifdef USE_PAM
	PAM::Authenticator pam;
#endif
#ifdef USE_CONSOLEKIT
    Ck::Session ck;
#endif

    // Options
    char* DispName;

    Cfg *cfg;

    Pixmap BackgroundPixmap;

    void blankScreen();
    Image* image;
    void setBackground(const std::string& themedir);

    bool firstlogin;
    bool daemonmode;
    bool force_nodaemon;
	// For testing themes
	char* testtheme;
    bool testing;
    
    std::string themeName;
    std::string mcookie;

    const int mcookiesize;
};


#endif

