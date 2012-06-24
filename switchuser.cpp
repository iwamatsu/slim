/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <cstdio>
#include "switchuser.h"
#include "util.h"

using namespace std;

SwitchUser::SwitchUser(struct passwd *pw, Cfg *c, const string& display,
                       char** _env)
    : cfg(c),
      Pw(pw),
      displayName(display),
      env(_env)
{
}

SwitchUser::~SwitchUser() {
    // Never called
}

void SwitchUser::Login(const char* cmd, const char* mcookie) {
    SetUserId();
    SetClientAuth(mcookie);
    Execute(cmd);
}

void SwitchUser::SetUserId() {
    if( (Pw == 0) ||
            (initgroups(Pw->pw_name, Pw->pw_gid) != 0) ||
            (setgid(Pw->pw_gid) != 0) ||
            (setuid(Pw->pw_uid) != 0) ) {
        logStream << APPNAME << ": could not switch user id" << endl;
        exit(ERR_EXIT);
    }
}

void SwitchUser::Execute(const char* cmd) {
    chdir(Pw->pw_dir);
    execle(Pw->pw_shell, Pw->pw_shell, "-c", cmd, NULL, env);
    logStream << APPNAME << ": could not execute login command" << endl;
}

void SwitchUser::SetClientAuth(const char* mcookie) {
    bool r;
    string home = string(Pw->pw_dir);
    string authfile = home + "/.Xauthority";
    remove(authfile.c_str());
    r = Util::add_mcookie(mcookie, ":0", cfg->getOption("xauth_path"),
      authfile);
}
