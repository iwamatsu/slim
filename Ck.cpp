/* SLiM - Simple Login Manager
   Copyright (C) 2011 David Hauweele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <cstdio>
#include <iostream>

#include <ck-connector.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdarg.h>

#include "Ck.h"

namespace Ck {
  Exception::Exception(const std::string &func,
                       const std::string &errstr):
    func(func),
    errstr(errstr)
  {}

  dbus_bool_t Session::ck_connector_open_graphic_session(const std::string &display,
                                                         uid_t uid)
  {
    dbus_bool_t local        = true;
    const char *session_type = "x11";
    const char *x11_display  = display.c_str();
    const char *x11_device   = get_x11_device(display);
    const char *remote_host  = "";
    const char *display_dev  = "";

    return ck_connector_open_session_with_parameters(ckc, &error,
                                                     "unix-user", &uid,
                                                     "session-type", &session_type,
                                                     "x11-display", &x11_display,
                                                     "x11-display-device", &x11_device,
                                                     "display-device", &display_dev,
                                                     "remote-host-name", &remote_host,
                                                     "is-local", &local,
                                                     NULL);
  }

  const char * Session::get_x11_device(const std::string &display)
  {
    static char device[32];

    Display *xdisplay = XOpenDisplay(display.c_str());

    if(!xdisplay)
      throw Exception(__func__, "cannot open display");

    Window root;
    Atom xfree86_vt_atom;
    Atom return_type_atom;
    int return_format;
    unsigned long return_count;
    unsigned long bytes_left;
    unsigned char *return_value;
    long vt;

    xfree86_vt_atom = XInternAtom(xdisplay, "XFree86_VT", true);

    if(xfree86_vt_atom == None)
      throw Exception(__func__, "cannot get XFree86_VT");

    root = DefaultRootWindow(xdisplay);

    if(XGetWindowProperty(xdisplay, root, xfree86_vt_atom,
                          0L, 1L, false, XA_INTEGER,
                          &return_type_atom, &return_format,
                          &return_count, &bytes_left,
                          &return_value) != Success)
      throw Exception(__func__, "cannot get root window property");

    if(return_type_atom != XA_INTEGER)
      throw Exception(__func__, "bad atom type");

    if(return_format != 32)
      throw Exception(__func__, "invalid return format");

    if(return_count != 1)
      throw Exception(__func__, "invalid count");

    if(bytes_left != 0)
      throw Exception(__func__, "invalid bytes left");

    vt = *((long *)return_value);

    std::snprintf(device, 32, "/dev/tty%ld", vt);

    if(return_value)
      XFree(return_value);

    return device;
  }

  void Session::open_session(const std::string &display, uid_t uid)
  {
    ckc = ck_connector_new();

    if(!ckc)
      throw Exception(__func__, "error setting up connection to ConsoleKit");

    if(!ck_connector_open_graphic_session(display, uid)) {
      if(dbus_error_is_set(&error))
        throw Exception(__func__, error.message);
      else
        throw Exception(__func__, "cannot open ConsoleKit session: OOM, DBus system bus "
                        " not available or insufficient privileges");
    }
  }

  const char * Session::get_xdg_session_cookie()
  {
    return ck_connector_get_cookie(ckc);
  }

  void Session::close_session()
  {
    if(!ck_connector_close_session(ckc, &error)) {
      if(dbus_error_is_set(&error))
        throw Exception(__func__, error.message);
      else
        throw Exception(__func__, "cannot close ConsoleKit session: OOM, DBus system bus "
                        " not available or insufficient privileges");
    }
  }

  Session::Session()
  {
    dbus_error_init(&error);
  }

  Session::~Session()
  {
    dbus_error_free(&error);
  }
};

std::ostream& operator<<( std::ostream& os, const Ck::Exception& e)
{
  os << e.func << ": " << e.errstr;
  return os;
}
