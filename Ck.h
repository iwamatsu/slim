/* SLiM - Simple Login Manager
   Copyright (C) 2007 Martin Parm

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _CK_H_
#define _CK_H_

#include <string>

#include <ck-connector.h>
#include <dbus/dbus.h>

namespace Ck {
  class Exception {
  public:
    std::string func;
    std::string errstr;
    Exception(const std::string &func, const std::string &errstr);
  };

  class Session {
  private:
    CkConnector *ckc;
    DBusError error;

    const char * get_x11_device(const std::string &display);
    dbus_bool_t ck_connector_open_graphic_session(const std::string &display,
                                                  uid_t uid);
  public:
    const char * get_xdg_session_cookie();
    void open_session(const std::string &display, uid_t uid);
    void close_session();

    Session();
    ~Session();
  };
};

std::ostream& operator<<( std::ostream& os, const Ck::Exception& e);

#endif /* _CK_H_ */
