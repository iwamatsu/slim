/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include "switchuser.h"

using namespace std;

SwitchUser::SwitchUser(struct passwd *pw, Cfg *c, const string& display)
    : cfg(c),
      Pw(pw),
      displayName(display)
{
}


SwitchUser::~SwitchUser() {
    // Never called
}


void SwitchUser::Login(const char* cmd, const char* mcookie) {
    SetEnvironment();
    SetUserId();
    SetClientAuth(mcookie);
    Execute(cmd);
}


void SwitchUser::SetEnvironment() {
    char *term = getenv("TERM");
    char** environ;
    environ = (char **) new char*[2];
    environ[0] = 0;
    if(term)
        putenv(StrConcat("TERM=", term));
    putenv(StrConcat("HOME=", Pw->pw_dir));
    putenv(StrConcat("SHELL=", Pw->pw_shell));
    putenv(StrConcat("USER=", Pw->pw_name));
    putenv(StrConcat("LOGNAME=", Pw->pw_name));
    putenv(StrConcat("PATH=", cfg->getOption("default_path").c_str()));
    putenv(StrConcat("DISPLAY=", displayName.c_str()));
    putenv(StrConcat("MAIL="_PATH_MAILDIR"/", Pw->pw_name));
    putenv(StrConcat("XAUTHORITY=", StrConcat(Pw->pw_dir,"/.Xauthority")));
    /* putenv("XAUTHORITY=/tmp/serverauth"); */
    chdir(Pw->pw_dir);
}


void SwitchUser::SetUserId() {
    if( (Pw == 0) ||
            (initgroups(Pw->pw_name, Pw->pw_gid) != 0) ||
            (setgid(Pw->pw_gid) != 0) ||
            (setuid(Pw->pw_uid) != 0) ) {
        cerr << APPNAME << ": could not switch user id" << endl;
        exit(ERR_EXIT);
    }
}


void SwitchUser::Execute(const char* cmd) {
    char *args[4];
    char* shell = strdup(Pw->pw_shell);
    char *shell_basename = BaseName(shell);

    args[0] = new char[strlen(shell_basename) + 2];
    strcpy(args[0], "-");
    strcat(args[0], shell_basename);
    args[1] = "-c";
    args[2] = (char*)cmd;
    args[3] = 0;

    execv(shell, args);
    cerr << APPNAME << ": could not execute login command" << endl;
}


char* SwitchUser::BaseName(const char* name) {
    const char *base = name;

    while(*name) {
        if(*name == '/')
            base = name + 1;
        ++name;
    }

    return (char*) base;
}


char* SwitchUser::StrConcat(const char* str1, const char* str2) {
    char* tmp = new char[strlen(str1) + strlen(str2) + 1];
    strcpy(tmp, str1);
    strcat(tmp, str2);
    return tmp;
}

void SwitchUser::SetClientAuth(const char* mcookie) {
    int r;
    string home = string(Pw->pw_dir);
    string authfile = home + "/.Xauthority";
    remove(authfile.c_str());
    string cmd = cfg->getOption("xauth_path") + " -q -f " + authfile + " add :0 . " + mcookie;
    r = system(cmd.c_str());
}
