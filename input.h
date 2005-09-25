/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-05 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-05 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _INPUT_H_
#define _INPUT_H_

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xft/Xft.h>
#include <pwd.h>
#include <unistd.h>
#include <string>
#include <ctype.h>
#include <iostream>

#include "const.h"
#include "cfg.h"

#ifdef HAVE_SHADOW
#include <shadow.h>
#endif

class Input {
public:
    Input(Cfg* c);
    ~Input();
    char Key(char ascii, KeySym keysym, bool singleInputMode);
    int GetAction();
    int GetField();
    char* GetName();
    char* GetHiddenPasswd();
    struct passwd* GetPasswdStruct();
    void Reset();
    void ResetName();
    void ResetPassword();
    void SetName(string name);

private:
    void Add(char ascii);
    char DeleteLast();
    int Correct();
    int SpecialWanted();
    int SpecialCorrect(int special);
    Cfg* cfg;

    char NameBuffer[INPUT_MAXLENGTH_NAME];
    char PasswdBuffer[INPUT_MAXLENGTH_PASSWD];
    char HiddenPasswdBuffer[INPUT_MAXLENGTH_PASSWD];
    int Action;
    int Field;

};

#endif

