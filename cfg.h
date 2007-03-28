/* SLiM - Simple Login Manager
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#ifndef _CFG_H_
#define _CFG_H_

#include <string>
#include <map>
#include <vector>

#define INPUT_MAXLENGTH_NAME    30
#define INPUT_MAXLENGTH_PASSWD  50

#define CFGFILE SYSCONFDIR"/slim.conf"
#define THEMESDIR PKGDATADIR"/themes"
#define THEMESFILE "/slim.theme"

using namespace std;

class Cfg {

public:
    Cfg();
    ~Cfg();
    bool readConf(string configfile);
    string parseOption(string line, string option);
    const string& getError() const;
    string& getOption(string option);
    string getWelcomeMessage();

    static int absolutepos(const string& position, int max, int width);
    static int string2int(const char* string, bool* ok = 0);
    static void split(vector<string>& v, const string& str, 
		      char c, bool useEmpty=true);
    static string Trim(const string& s);

    string nextSession(string current);

private:
    map<string,string> options;
    vector<string> sessions;
    int currentSession;
    string error;

};

#endif
