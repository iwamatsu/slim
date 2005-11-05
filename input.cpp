/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-05 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-05 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include "input.h"
#include <cstdlib>

Input::Input(Cfg* c) {
    NameBuffer[0] = '\0';
    PasswdBuffer[0] = '\0';
    HiddenPasswdBuffer[0] = '\0';
    Field = GET_NAME;
    Action = WAIT;
    cfg = c;
}


Input::~Input() {
    // Does nothing
}


char Input::Key(char ascii, KeySym keysym, bool singleInputMode) {
    char tmp = 0;

    // Take a screenshot
    if(keysym == XK_F11) {
        system(cfg->getOption("screenshot_cmd").c_str());
    }

    if  (!singleInputMode && (keysym == XK_Tab || keysym == XK_ISO_Left_Tab)) {
        if (Field == GET_NAME) {
            // Move to next field
            Field = GET_PASSWD;
        } else {
            Field = GET_NAME;
        }
    } else if(keysym == XK_Return || keysym == XK_KP_Enter) {
        if(!strcmp(NameBuffer, ""))
            return tmp;

        // Check for special command (console, halt, reboot, exit)
        int special = SpecialWanted();

        if(Field == GET_NAME) {
            // Move to next field
            Field = GET_PASSWD;

            // Check for special command (console, exit)
            if(special == CONSOLE || special == EXIT)
                Action = special;
        } else {
            // Check for special command (halt, reboot)
            if(special == REBOOT || special == HALT || special == SUSPEND)
                Action = SpecialCorrect(special);

            // Regular login
            else {
                if(Correct())
                    Action = LOGIN;
                else
                    Action = FAIL;
            }
        }
    } else if(keysym == XK_Delete || keysym == XK_BackSpace)
        tmp = DeleteLast();
    else if(isprint(ascii))
        if(keysym < XK_Shift_L || keysym > XK_Hyper_R)
            Add(ascii);

    return tmp;
}


int Input::GetAction() {
    return Action;
}


int Input::GetField() {
    return Field;
}


char* Input::GetName() {
    return NameBuffer;
}


char* Input::GetHiddenPasswd() {
    return HiddenPasswdBuffer;
}


struct passwd* Input::GetPasswdStruct() {
    struct passwd* pw = getpwnam(NameBuffer);
    endpwent();
    if (pw->pw_shell[0] == '\0') {
        setusershell();
        pw->pw_shell = getusershell();
        endusershell();
    }
    return pw;
}


void Input::Add(char ascii) {
    char tmp[2];
    tmp[0] = ascii;
    tmp[1] = '\0';

    switch(Field) {
    case GET_NAME:
        if(strlen(NameBuffer) < INPUT_MAXLENGTH_NAME-1)
            strcat(NameBuffer, tmp);
        break;
    case GET_PASSWD:
        if(strlen(PasswdBuffer) < INPUT_MAXLENGTH_PASSWD-1) {
            strcat(PasswdBuffer, tmp);
            tmp[0] = '*';
            strcat(HiddenPasswdBuffer, tmp);
        }
        break;
    }
}


char Input::DeleteLast() {
    char tmp = 0;
    int len = 0;

    switch(Field) {
    case GET_NAME:
        len = strlen(NameBuffer) - 1;
        tmp = NameBuffer[len];
        NameBuffer[len] = '\0';
        break;
    case GET_PASSWD:
        len = strlen(PasswdBuffer) - 1;
        tmp = '*';
        PasswdBuffer[len] = '\0';
        HiddenPasswdBuffer[len] = '\0';
        break;
    }

    return tmp;
}


void Input::Reset() {
    ResetName();
    ResetPassword();
    Field = GET_NAME;
    Action = WAIT;
}

void Input::ResetName() {
    NameBuffer[0] = '\0';
    Action = WAIT;
}

void Input::ResetPassword() {
    PasswdBuffer[0] = '\0';
    HiddenPasswdBuffer[0] = '\0';
    Action = WAIT;
}

// used for automatically fill name at startup
void Input::SetName(string name) {
    NameBuffer[0] = '\0';
    if(name.size() < INPUT_MAXLENGTH_NAME) {
        strcat(NameBuffer, name.c_str());
    }
    Field = GET_PASSWD;
    Action = WAIT;
}

int Input::Correct() {
    char *unencrypted, *encrypted, *correct;
    struct passwd *pw;

    pw = getpwnam(NameBuffer);
    endpwent();
    if(pw == 0)
        return 0;

#ifdef HAVE_SHADOW
    struct spwd *sp = getspnam(pw->pw_name);    
    endspent();
    if(sp)
	correct = sp->sp_pwdp;
    else
#endif
	correct = pw->pw_passwd;

    if(correct == 0 || correct[0] == '\0')
        return 1;

    unencrypted = PasswdBuffer;
    encrypted = crypt(unencrypted, correct);
    memset(unencrypted, 0, strlen (unencrypted));
    return (strcmp(encrypted, correct) == 0);
}


int Input::SpecialWanted() {
    int result;

    if(!strcmp(NameBuffer, CONSOLE_STR))
        result = CONSOLE;
    else if(!strcmp(NameBuffer, HALT_STR))
        result = HALT;
    else if(!strcmp(NameBuffer, REBOOT_STR))
        result = REBOOT;
    else if(!strcmp(NameBuffer, SUSPEND_STR))
        result = SUSPEND;
    else if(!strcmp(NameBuffer, EXIT_STR))
        result = EXIT;
    else
        result = 0;

    return result;
}


int Input::SpecialCorrect(int special) {
    int result, c;
    char tmp[INPUT_MAXLENGTH_NAME];

    strcpy(tmp, NameBuffer);
    strcpy(NameBuffer, "root");
    c = Correct();
    strcpy(NameBuffer, tmp);

    if(c)
        result = special;
    else
        result = FAIL;

    return result;
}

