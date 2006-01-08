/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <sstream>
#include "panel.h"

using namespace std;

Panel::Panel(Display* dpy, int scr, Window root, Cfg* config,
             const string& themedir) {
    // Set display
    Dpy = dpy;
    Scr = scr;
    Root = root;
    cfg = config;

    session = "";

    // Init GC
    XGCValues gcv;
    unsigned long gcm;
    gcm = GCForeground|GCBackground|GCGraphicsExposures;
    gcv.foreground = GetColor("black");
    gcv.background = GetColor("white");
    gcv.graphics_exposures = False;
    TextGC = XCreateGC(Dpy, Root, gcm, &gcv);

    font = XftFontOpenName(Dpy, Scr, cfg->getOption("input_font").c_str());
    welcomefont = XftFontOpenName(Dpy, Scr, cfg->getOption("welcome_font").c_str());
    introfont = XftFontOpenName(Dpy, Scr, cfg->getOption("intro_font").c_str());
    enterfont = XftFontOpenName(Dpy, Scr, cfg->getOption("username_font").c_str());
    msgfont = XftFontOpenName(Dpy, Scr, cfg->getOption("msg_font").c_str());

    Visual* visual = DefaultVisual(Dpy, Scr);
    Colormap colormap = DefaultColormap(Dpy, Scr);
    // NOTE: using XftColorAllocValue() would be a better solution. Lazy me.
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("input_color").c_str(), &inputcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("input_shadow_color").c_str(), &inputshadowcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("welcome_color").c_str(), &welcomecolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("welcome_shadow_color").c_str(), &welcomeshadowcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("username_color").c_str(), &entercolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("username_shadow_color").c_str(), &entershadowcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("msg_color").c_str(), &msgcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("msg_shadow_color").c_str(), &msgshadowcolor);
    XftColorAllocName(Dpy, visual, colormap, cfg->getOption("intro_color").c_str(), &introcolor);

    // Load properties from config / theme
    input_name_x = Cfg::string2int(cfg->getOption("input_name_x").c_str());
    input_name_y = Cfg::string2int(cfg->getOption("input_name_y").c_str());
    input_pass_x = Cfg::string2int(cfg->getOption("input_pass_x").c_str());
    input_pass_y = Cfg::string2int(cfg->getOption("input_pass_y").c_str());
    inputShadowXOffset =
        Cfg::string2int(cfg->getOption("input_shadow_xoffset").c_str());
    inputShadowYOffset =
        Cfg::string2int(cfg->getOption("input_shadow_yoffset").c_str());

    if (input_pass_x < 0 || input_pass_y < 0){ // single inputbox mode
        input_pass_x = input_name_x;
        input_pass_y = input_name_y;
    }

    // Load panel and background image
    string panelpng = "";
    panelpng = panelpng + themedir +"/panel.png";
    image = new Image;
    bool loaded = image->Read(panelpng.c_str());
    if (!loaded) { // try jpeg if png failed
        panelpng = themedir + "/panel.jpg";
        loaded = image->Read(panelpng.c_str());
        if (!loaded) {
            cerr << APPNAME << ": could not load panel image for theme '"
		 << basename((char*)themedir.c_str()) << "'"
		 << endl;
            exit(ERR_EXIT);
        }
    }

    Image* bg = new Image();
    string bgstyle = cfg->getOption("background_style");
    if (bgstyle != "color") {
        panelpng = themedir +"/background.png";
        loaded = bg->Read(panelpng.c_str());
        if (!loaded) { // try jpeg if png failed
            panelpng = themedir + "/background.jpg";
            loaded = bg->Read(panelpng.c_str());
            if (!loaded){
                cerr << APPNAME << ": could not load background image for theme '"
		        << basename((char*)themedir.c_str()) << "'"
		        << endl;
                exit(ERR_EXIT);
            }
        }
    }
    if (bgstyle == "stretch") {
        bg->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    } else if (bgstyle == "tile") {
        bg->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    } else if (bgstyle == "center") {
 	    string hexvalue = cfg->getOption("background_color");
        hexvalue = hexvalue.substr(1,6);
  	    bg->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
       			    hexvalue.c_str());
    } else { // plain color or error
	    string hexvalue = cfg->getOption("background_color");
        hexvalue = hexvalue.substr(1,6);
  	    bg->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
    			    hexvalue.c_str());
    }

    string cfgX = cfg->getOption("input_panel_x");
    string cfgY = cfg->getOption("input_panel_y");
    X = Cfg::absolutepos(cfgX, XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), image->Width());
    Y = Cfg::absolutepos(cfgY, XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)), image->Height());

    // Merge image into background
    image->Merge(bg, X, Y);
    PanelPixmap = image->createPixmap(Dpy, Scr, Root);

    // Read (and substitute vars in) the welcome message
    welcome_message = cfg->getWelcomeMessage();
    intro_message = cfg->getOption("intro_msg");

    // Init In
    In = new Input(cfg);
}

Panel::~Panel() {
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &inputcolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &msgcolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &welcomecolor);
    XftColorFree (Dpy, DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr), &entercolor);
    XFreeGC(Dpy, TextGC);
    delete In;
    delete image;

}

void Panel::OpenPanel() {
    // Create window
    Win = XCreateSimpleWindow(Dpy, Root, X, Y,
                              image->Width(),
                              image->Height(),
                              0, GetColor("white"), GetColor("white"));

    // Events
    XSelectInput(Dpy, Win, ExposureMask | KeyPressMask);

    // Set background
    XSetWindowBackgroundPixmap(Dpy, Win, PanelPixmap);

    // Show window
    XMapWindow(Dpy, Win);
    XMoveWindow(Dpy, Win, X, Y); // override wm positioning (for tests)

    // Grab keyboard
    XGrabKeyboard(Dpy, Win, False, GrabModeAsync, GrabModeAsync, CurrentTime);

    XFlush(Dpy);

}

void Panel::ClosePanel() {
    XUngrabKeyboard(Dpy, CurrentTime);
    XUnmapWindow(Dpy, Win);
    XDestroyWindow(Dpy, Win);
    XFlush(Dpy);
}

void Panel::ClearPanel() {
    session = "";
    In->Reset();
    XClearWindow(Dpy, Root);
    XClearWindow(Dpy, Win);
    Cursor(SHOW);
    ShowText();
    XFlush(Dpy);
}

void Panel::Message(const char* text) {
    string cfgX, cfgY;
    XGlyphInfo extents;
    XftDraw *draw = XftDrawCreate(Dpy, Root,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    XftTextExtents8(Dpy, msgfont, (XftChar8*)text,
                    strlen(text), &extents);
    cfgX = cfg->getOption("msg_x");
    cfgY = cfg->getOption("msg_y");
    int shadowXOffset =
        Cfg::string2int(cfg->getOption("msg_shadow_xoffset").c_str());
    int shadowYOffset =
        Cfg::string2int(cfg->getOption("msg_shadow_yoffset").c_str());

    int msg_x = Cfg::absolutepos(cfgX, XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), extents.width);
    int msg_y = Cfg::absolutepos(cfgY, XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)), extents.height);

    SlimDrawString8 (draw, &msgcolor, msgfont, msg_x, msg_y,
                     (XftChar8*)text, strlen(text),
                     &msgshadowcolor,
                     shadowXOffset, shadowYOffset);
    XFlush(Dpy);
    XftDrawDestroy(draw);
}

void Panel::Error(const char* text) {
    ClosePanel();
    Message(text);
    sleep(ERROR_DURATION);
    OpenPanel();
    ClearPanel();
}


Input* Panel::GetInput() {
    return In;
}

unsigned long Panel::GetColor(const char* colorname) {
    XColor color;
    XWindowAttributes attributes;

    XGetWindowAttributes(Dpy, Root, &attributes);
    color.pixel = 0;

    if(!XParseColor(Dpy, attributes.colormap, colorname, &color))
        cerr << APPNAME << ": can't parse color " << colorname << endl;
    else if(!XAllocColor(Dpy, attributes.colormap, &color))
        cerr << APPNAME << ": can't allocate color " << colorname << endl;

    return color.pixel;
}

void Panel::Cursor(int visible) {
    char* text;
    int xx, yy, x2,y2, cheight;
    char* txth = "Wj"; // used to get cursor height

    switch(In->GetField()) {
        case GET_PASSWD:
            text = In->GetHiddenPasswd();
            xx = input_pass_x;
            yy = input_pass_y;
            break;

        case GET_NAME:
            text = In->GetName();
            xx = input_name_x;
            yy = input_name_y;
            break;
    }


    XGlyphInfo extents;
    XftTextExtents8(Dpy, font, (XftChar8*)txth, strlen(txth), &extents);
    cheight = extents.height;
    y2 = yy - extents.y + extents.height;
    XftTextExtents8(Dpy, font, (XftChar8*)text, strlen(text), &extents);
    xx += extents.width;

    if(visible == SHOW) {
        XSetForeground(Dpy, TextGC,
                       GetColor(cfg->getOption("input_color").c_str()));
        XDrawLine(Dpy, Win, TextGC,
                  xx+1, yy-cheight,
                  xx+1, y2);
    } else {
        XClearArea(Dpy, Win, xx+1, yy-cheight,
                   1, y2-(yy-cheight)+1, false);
    }
}

int Panel::EventHandler(XEvent* event) {
    Action = WAIT;

    switch(event->type) {
    case Expose:
        OnExpose(event);
        break;

    case KeyPress:
        OnKeyPress(event);
        break;
    }

    return Action;
}

void Panel::OnExpose(XEvent* event) {
    char* name = In->GetName();
    char* passwd = In->GetHiddenPasswd();
    XftDraw *draw = XftDrawCreate(Dpy, Win,
                        DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    if (input_pass_x != input_name_x || input_pass_y != input_name_y){
        SlimDrawString8 (draw, &inputcolor, font, input_name_x, input_name_y,
                         (XftChar8*)name, strlen(name),
                         &inputshadowcolor,
                         inputShadowXOffset, inputShadowYOffset);
        SlimDrawString8 (draw, &inputcolor, font, input_pass_x, input_pass_y,
                         (XftChar8*)passwd, strlen(passwd),
                         &inputshadowcolor,
                         inputShadowXOffset, inputShadowYOffset);
    } else { //single input mode
        switch(In->GetField()) {
            case GET_PASSWD:
                SlimDrawString8 (draw, &inputcolor, font,
                                 input_pass_x, input_pass_y,
                                 (XftChar8*)passwd, strlen(passwd),
                                 &inputshadowcolor,
                                 inputShadowXOffset, inputShadowYOffset);
                break;
            case GET_NAME:
                SlimDrawString8 (draw, &inputcolor, font,
                                 input_name_x, input_name_y,
                                 (XftChar8*)name, strlen(name),
                                 &inputshadowcolor,
                                 inputShadowXOffset, inputShadowYOffset);
                break;
        }
    }

    XftDrawDestroy (draw);
    Cursor(SHOW);
    ShowText();
}

void Panel::OnKeyPress(XEvent* event) {
    char del;
    char buffer;
    KeySym keysym;
    XComposeStatus compstatus;
    int xx;
    int yy;
    char* text;

    bool singleInputMode =
        input_name_x == input_pass_x &&
        input_name_y == input_pass_y;
    Cursor(HIDE);
    XLookupString(&event->xkey, &buffer, 1, &keysym, &compstatus);
    del = In->Key(buffer, keysym, singleInputMode);
    Action = In->GetAction();

    XGlyphInfo extents, delextents;
    XftDraw *draw = XftDrawCreate(Dpy, Win,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));

    if (keysym == XK_F1) {
        SwitchSession();
    }

    bool clearField = false;
    string formerString = "";
    if ((((XKeyEvent*)event)->state & ControlMask)) {
        if (keysym == XK_w || keysym == XK_u) {
            clearField = true;
        }
    }

    switch(In->GetField()) {
        case GET_PASSWD:
            if (strlen(In->GetHiddenPasswd()) == 0){
                // clear name and welcome label if we just entered the
                // password field
                if (singleInputMode) {
                    xx = input_name_x;
                    yy = input_name_y;
                    text = In->GetName();
                    XftTextExtents8(Dpy, font, (XftChar8*)text,
                                    strlen(text), &extents);
                    XClearWindow(Dpy, Win);
                    ShowText();
                }
            }

            if (clearField) {
                formerString = In->GetHiddenPasswd();
                In->ResetPassword();
            }
            text = In->GetHiddenPasswd();
            xx = input_pass_x;
            yy = input_pass_y;
            break;

        case GET_NAME:
            if (clearField) {
                formerString = In->GetName();
                In->ResetName();
            }
            text = In->GetName();
            xx = input_name_x;
            yy = input_name_y;
            break;
    }

    char* txth = "Wj"; // get proper maximum height ?
    XftTextExtents8(Dpy, font, (XftChar8*)txth, strlen(txth), &extents);
    int maxHeight = extents.height;

    string tmp = "";
    if (clearField) {
        tmp = formerString;
    } else {
        tmp = text;
        tmp = tmp + del;
    }
    XftTextExtents8(Dpy, font, (XftChar8*)tmp.c_str(),
                    strlen(tmp.c_str()), &extents);
    int maxLength = extents.width;

    XClearArea(Dpy, Win, xx-3, yy-maxHeight-3,
               maxLength+6, maxHeight+6, false);

    if (!clearField) {
        SlimDrawString8 (draw, &inputcolor, font, xx, yy,
                         (XftChar8*)text, strlen(text),
                         &inputshadowcolor,
                         inputShadowXOffset, inputShadowYOffset);
    }

    XftDrawDestroy (draw);
    Cursor(SHOW);
}

// Draw welcome and "enter username" message
void Panel::ShowText(){
    string cfgX, cfgY;
    int n=-1;
    XGlyphInfo extents;

    bool singleInputMode =
    input_name_x == input_pass_x &&
    input_name_y == input_pass_y;

    XftDraw *draw = XftDrawCreate(Dpy, Win,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    /* welcome message */
    XftTextExtents8(Dpy, welcomefont, (XftChar8*)welcome_message.c_str(),
                    strlen(welcome_message.c_str()), &extents);
    cfgX = cfg->getOption("welcome_x");
    cfgY = cfg->getOption("welcome_y");
    int shadowXOffset =
        Cfg::string2int(cfg->getOption("welcome_shadow_xoffset").c_str());
    int shadowYOffset =
        Cfg::string2int(cfg->getOption("welcome_shadow_yoffset").c_str());
    welcome_x = Cfg::absolutepos(cfgX, image->Width(), extents.width);
    welcome_y = Cfg::absolutepos(cfgY, image->Height(), extents.height);
    if (welcome_x >= 0 && welcome_y >= 0) {
        SlimDrawString8 (draw, &welcomecolor, welcomefont,
                         welcome_x, welcome_y,
                         (XftChar8*)welcome_message.c_str(),
                         strlen(welcome_message.c_str()),
                         &welcomeshadowcolor, shadowXOffset, shadowYOffset);
    }

    /* Enter username-password message */
    string msg;
    if (!singleInputMode|| In->GetField() == GET_PASSWD ) {
        msg = cfg->getOption("password_msg");
        XftTextExtents8(Dpy, enterfont, (XftChar8*)msg.c_str(),
                        strlen(msg.c_str()), &extents);
        cfgX = cfg->getOption("password_x");
        cfgY = cfg->getOption("password_y");
        int shadowXOffset =
            Cfg::string2int(cfg->getOption("username_shadow_xoffset").c_str());
        int shadowYOffset =
            Cfg::string2int(cfg->getOption("username_shadow_yoffset").c_str());
        password_x = Cfg::absolutepos(cfgX, image->Width(), extents.width);
        password_y = Cfg::absolutepos(cfgY, image->Height(), extents.height);
        if (password_x >= 0 && password_y >= 0){
            SlimDrawString8 (draw, &entercolor, enterfont, password_x, password_y,
                             (XftChar8*)msg.c_str(), strlen(msg.c_str()),
                             &entershadowcolor, shadowXOffset, shadowYOffset);
        }
    }
    if (!singleInputMode|| In->GetField() == GET_NAME ) {
        msg = cfg->getOption("username_msg");
        XftTextExtents8(Dpy, enterfont, (XftChar8*)msg.c_str(),
                        strlen(msg.c_str()), &extents);
        cfgX = cfg->getOption("username_x");
        cfgY = cfg->getOption("username_y");
        int shadowXOffset =
            Cfg::string2int(cfg->getOption("username_shadow_xoffset").c_str());
        int shadowYOffset =
            Cfg::string2int(cfg->getOption("username_shadow_yoffset").c_str());
        username_x = Cfg::absolutepos(cfgX, image->Width(), extents.width);
        username_y = Cfg::absolutepos(cfgY, image->Height(), extents.height);
        if (username_x >= 0 && username_y >= 0){
            SlimDrawString8 (draw, &entercolor, enterfont, username_x, username_y,
                             (XftChar8*)msg.c_str(), strlen(msg.c_str()),
                             &entershadowcolor, shadowXOffset, shadowYOffset);
        }
    }
    XftDrawDestroy(draw);
}

string Panel::getSession() {
    return session;
}

// choose next available session type
void Panel::SwitchSession() {
    session = cfg->nextSession(session);
    //TODO: get sessions from cfg and cycle to the next one
    ShowSession();
}

// Display session type on the screen
void Panel::ShowSession() {
    XClearWindow(Dpy, Root);
    string currsession = "Session: " + session;
    char* text = (char*) currsession.c_str();
    XGlyphInfo extents;
    XftDraw *draw = XftDrawCreate(Dpy, Root,
                                  DefaultVisual(Dpy, Scr), DefaultColormap(Dpy, Scr));
    XftTextExtents8(Dpy, msgfont, (XftChar8*)text,
                    strlen(text), &extents);
    int msg_x = Cfg::absolutepos("50%", XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), extents.width);
    int msg_y = XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)) - extents.height -100;
    int shadowXOffset =
        Cfg::string2int(cfg->getOption("welcome_shadow_xoffset").c_str());
    int shadowYOffset =
        Cfg::string2int(cfg->getOption("welcome_shadow_yoffset").c_str());

    SlimDrawString8(draw, &msgcolor, msgfont, msg_x, msg_y,
                    (XftChar8*)text, strlen(text),
                    &msgshadowcolor,
                    shadowXOffset, shadowYOffset);
    XFlush(Dpy);
    XftDrawDestroy(draw);
}


void Panel::SlimDrawString8(XftDraw *d, XftColor *color, XftFont *font,
                            int x, int y, XftChar8 *string, int len,
                            XftColor* shadowColor,
                            int xOffset, int yOffset)
{
    if (xOffset && yOffset) {
        XftDrawString8(d, shadowColor, font, x+xOffset, y+yOffset,
                       string, len);
    }
    XftDrawString8(d, color, font, x, y, string, len);
}
