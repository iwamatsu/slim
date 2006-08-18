/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _PANEL_H_
#define _PANEL_H_

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xmu/WinUtil.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#ifdef NEEDS_BASENAME
#include <libgen.h>
#endif

#include "switchuser.h"
#include "input.h"
#include "const.h"
#include "image.h"


class Panel {
public:
    Panel(Display* dpy, int scr, Window root, Cfg* config,
          const string& themed);
    ~Panel();
    void OpenPanel();
    void ClosePanel();
    void ClearPanel();
    void Message(const char* text);
    void Error(const char* text);
    Input* GetInput();
    int EventHandler(XEvent* event);
    string getSession();

private:
    Panel();
    void Cursor(int visible);
    unsigned long GetColor(const char* colorname);
    void OnExpose(XEvent* event);
    void OnKeyPress(XEvent* event);
    void ShowText();
    void SwitchSession();
    void ShowSession();

    void SlimDrawString8(XftDraw* d, XftColor* color, XftFont* font,
                         int x, int y, XftChar8 *string, int len,
                         XftColor* shadowColor,
                         int xOffset, int yOffset);

    Cfg* cfg;

    // Private data
    Window Win;
    Window Root;
    Display* Dpy;
    int Scr;
    int X, Y;
    GC TextGC;
    XftFont* font;
    XftColor inputshadowcolor;
    XftColor inputcolor;
    XftColor msgcolor;
    XftColor msgshadowcolor;
    XftFont* msgfont;
    XftColor introcolor;
    XftFont* introfont;
    XftFont* welcomefont;
    XftColor welcomecolor;
    XftColor welcomeshadowcolor;
    XftFont* enterfont;
    XftColor entercolor;
    XftColor entershadowcolor;
    int Action;

    // Configuration
    int input_name_x;
    int input_name_y;
    int input_pass_x;
    int input_pass_y;
    int inputShadowXOffset;
    int inputShadowYOffset;
    int input_cursor_height;
    int welcome_x;
    int welcome_y;
    int welcome_shadow_xoffset;
    int welcome_shadow_yoffset;
    int intro_x;
    int intro_y;
    int username_x;
    int username_y;
    int username_shadow_xoffset;
    int username_shadow_yoffset;
    int password_x;
    int password_y;
    string welcome_message;
    string intro_message;

    // Pixmap data
    Pixmap PanelPixmap;

    // Name/Passwd handler
    Input* In;

    Image* image;

    // For thesting themes
    bool testing;
    string themedir;

    // Session handling
    string session;

};

#endif


