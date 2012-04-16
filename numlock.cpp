/* SLiM - Simple Login Manager
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>
   Copyright (C) 2012    Nobuhiro Iwamatsu <iwamatsu@nigauri.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Code adapted from NumLockX, look at the end of this file for
   the original Copyright information.

*/

#include "numlock.h"
#include <string.h>

NumLock::NumLock() {
}

int NumLock::xkb_init(Display* dpy) {
    int xkb_opcode, xkb_event, xkb_error;
    int xkb_lmaj = XkbMajorVersion;
    int xkb_lmin = XkbMinorVersion;

    return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin )
        && XkbQueryExtension( dpy, &xkb_opcode, &xkb_event, &xkb_error,
                   &xkb_lmaj, &xkb_lmin );
}
    
unsigned int NumLock::xkb_mask_modifier( XkbDescPtr xkb, const char *name ) {
    int i;
    if( !xkb || !xkb->names )
        return 0;

    for( i = 0; i < XkbNumVirtualMods; i++ ) {
        char* modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
        if( modStr != NULL && strcmp(name, modStr) == 0 ) {
            unsigned int mask;
            XkbVirtualModsToReal( xkb, 1 << i, &mask );
            return mask;
        }
    }
    return 0;
}

unsigned int NumLock::xkb_numlock_mask(Display* dpy) {
    XkbDescPtr xkb;

    xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd );
    if( xkb != NULL ) {
        unsigned int mask = xkb_mask_modifier( xkb, "NumLock" );
        XkbFreeKeyboard( xkb, 0, True );
        return mask;
    }
    return 0;
}

void NumLock::control_numlock(Display *dpy, bool flag) {
    unsigned int mask;
    
	if( !xkb_init(dpy) )
        return;

    mask = xkb_numlock_mask(dpy);
    if( mask == 0 )
        return;

    if( flag == true )
        XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, mask);
    else
        XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, 0);
}

void NumLock::setOn(Display *dpy) {
	control_numlock(dpy, true);
}

void NumLock::setOff(Display *dpy) {
	control_numlock(dpy, false);
}

/* 
 Copyright (C) 2000-2001 Lubos Lunak        <l.lunak@kde.org>
 Copyright (C) 2001      Oswald Buddenhagen <ossi@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/
