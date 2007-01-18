/* SLiM - Simple Login Manager
   Copyright (C) 1997, 1998 Per Liden
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>
      
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

/* Simple app to set the backround image of the X root window */

#include <iostream>
#include "image.h"


#define DISPLAY         ":0.0"
#define BG_STRETCH 1 
#define BG_TILE 2
#define BG_CENTER 3 
#define BG_COLOR 4

using namespace std;



void setBackground(int bgstyle, char* fname, char* bgcolor) {
    bool loaded;
    Display* Dpy;
    int Scr;
    Window Root;
   
    string filename;
    string bg_color;
    
    if (fname != NULL) {
        filename.assign(fname);
    }

    if (bgcolor != NULL) {
        bg_color.assign(bgcolor);
    }


    if((Dpy = XOpenDisplay(DISPLAY)) == 0) {
        cerr << "could not open display" << endl;
        exit(1);
    }
    
    Scr = DefaultScreen(Dpy);
    Root = RootWindow(Dpy, Scr);

    Image* image = new Image;
    if (bgstyle != BG_COLOR) {
        loaded = image->Read(fname);
        if (!loaded) {
            cerr << "Could not load image!" << endl;
            exit(1);
        }
    }
    
    if (bgstyle == BG_STRETCH) {
        image->Resize(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    } else if (bgstyle == BG_TILE) {
        image->Tile(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)));
    } else if (bgstyle == BG_CENTER) {
        image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
                        bg_color.c_str());
    } else {
        image->Center(XWidthOfScreen(ScreenOfDisplay(Dpy, Scr)), XHeightOfScreen(ScreenOfDisplay(Dpy, Scr)),
                    bg_color.c_str());
    }
    
    Pixmap p = image->createPixmap(Dpy, Scr, Root);
    XSetWindowBackgroundPixmap(Dpy, Root, p);
    XClearWindow(Dpy, Root);
    XFlush(Dpy);
    delete image;
    XCloseDisplay(Dpy);
}


int main(int argc, char** argv) {
    int tmp;
    char* filename;
    char* bg_color;
    int bg_style;

    bg_style = -1;
    
    while((tmp = getopt(argc, argv, "hs:t:n:c:?")) != EOF) {
        switch (tmp) {
        case 's':
            filename = optarg;
            bg_style = BG_STRETCH;
            break;
        case 't':
            filename = optarg;
            bg_style = BG_TILE;
            break;
        case 'n':
            filename = optarg;
            bg_style = BG_CENTER;
            break;
        case 'c':
            bg_color = optarg;
            bg_style = BG_COLOR;
            break;
        case '?':
            cerr << endl;
        case 'h':
            cerr << "usage: ssetbg [-c color] [-s|-t|-n filename]" << endl
            << "Available modes:" << endl
            << "    -s: stretch" << endl
            << "    -t: tiled" << endl
            << "    -n: center" << endl
            << "    -c: color" << endl;
            exit(0);
            break;
        }
    }

    if (bg_style == -1) {
        if (argc > 0)
            filename = argv[1];
        bg_style = BG_STRETCH;
    }

    if (bg_style != BG_COLOR && filename == NULL) {
        cerr << "No filename given!" << endl;
        exit(1);
    }
    if (bg_style == BG_COLOR && bg_color == NULL) {
        cerr << "No color given!" << endl;
        exit(1);
    }

    // if (bg_color == NULL)
    
    setBackground(bg_style, filename, bg_color);
    return(0);
}

