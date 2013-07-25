/* SLiM - Simple Login Manager
   Copyright (C) 2004-06 Simone Rota <sip@varlock.com>
   Copyright (C) 2004-06 Johannes Winkelmann <jw@tks6.net>
   Copyright (C) 2012-13 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/

#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "cfg.h"

using namespace std;

typedef pair<string,string> option;

Cfg::Cfg()
	: currentSession(-1)
{
	/* Configuration options */
	options.insert(option("default_path","/bin:/usr/bin:/usr/local/bin"));
	options.insert(option("default_xserver","/usr/bin/X"));
	options.insert(option("xserver_arguments",""));
	options.insert(option("numlock",""));
	options.insert(option("daemon",""));
	options.insert(option("xauth_path","/usr/bin/xauth"));
	options.insert(option("login_cmd","exec /bin/bash -login ~/.xinitrc %session"));
	options.insert(option("halt_cmd","/sbin/shutdown -h now"));
	options.insert(option("reboot_cmd","/sbin/shutdown -r now"));
	options.insert(option("suspend_cmd",""));
	options.insert(option("sessionstart_cmd",""));
	options.insert(option("sessionstop_cmd",""));
	options.insert(option("console_cmd","/usr/bin/xterm -C -fg white -bg black +sb -g %dx%d+%d+%d -fn %dx%d -T ""Console login"" -e /bin/sh -c ""/bin/cat /etc/issue; exec /bin/login"""));
	options.insert(option("screenshot_cmd","import -window root /slim.png"));
	options.insert(option("welcome_msg","Welcome to %host"));
	options.insert(option("session_msg","Session:"));
	options.insert(option("default_user",""));
	options.insert(option("focus_password","no"));
	options.insert(option("auto_login","no"));
	options.insert(option("current_theme","default"));
	options.insert(option("lockfile","/var/run/slim.lock"));
	options.insert(option("logfile","/var/log/slim.log"));
	options.insert(option("authfile","/var/run/slim.auth"));
	options.insert(option("shutdown_msg","The system is halting..."));
	options.insert(option("reboot_msg","The system is rebooting..."));
	options.insert(option("sessiondir",""));
	options.insert(option("hidecursor","false"));

	/* Theme stuff */
	options.insert(option("input_panel_x","50%"));
	options.insert(option("input_panel_y","40%"));
	options.insert(option("input_name_x","200"));
	options.insert(option("input_name_y","154"));
	options.insert(option("input_pass_x","-1")); /* default is single inputbox */
	options.insert(option("input_pass_y","-1"));
	options.insert(option("input_font","Verdana:size=11"));
	options.insert(option("input_color", "#000000"));
	options.insert(option("input_cursor_height","20"));
	options.insert(option("input_maxlength_name","20"));
	options.insert(option("input_maxlength_passwd","20"));
	options.insert(option("input_shadow_xoffset", "0"));
	options.insert(option("input_shadow_yoffset", "0"));
	options.insert(option("input_shadow_color","#FFFFFF"));

	options.insert(option("welcome_font","Verdana:size=14"));
	options.insert(option("welcome_color","#FFFFFF"));
	options.insert(option("welcome_x","-1"));
	options.insert(option("welcome_y","-1"));
	options.insert(option("welcome_shadow_xoffset", "0"));
	options.insert(option("welcome_shadow_yoffset", "0"));
	options.insert(option("welcome_shadow_color","#FFFFFF"));

	options.insert(option("intro_msg",""));
	options.insert(option("intro_font","Verdana:size=14"));
	options.insert(option("intro_color","#FFFFFF"));
	options.insert(option("intro_x","-1"));
	options.insert(option("intro_y","-1"));

	options.insert(option("background_style","stretch"));
	options.insert(option("background_color","#CCCCCC"));

	options.insert(option("username_font","Verdana:size=12"));
	options.insert(option("username_color","#FFFFFF"));
	options.insert(option("username_x","-1"));
	options.insert(option("username_y","-1"));
	options.insert(option("username_msg","Please enter your username"));
	options.insert(option("username_shadow_xoffset", "0"));
	options.insert(option("username_shadow_yoffset", "0"));
	options.insert(option("username_shadow_color","#FFFFFF"));

	options.insert(option("password_x","-1"));
	options.insert(option("password_y","-1"));
	options.insert(option("password_msg","Please enter your password"));

	options.insert(option("msg_color","#FFFFFF"));
	options.insert(option("msg_font","Verdana:size=16:bold"));
	options.insert(option("msg_x","40"));
	options.insert(option("msg_y","40"));
	options.insert(option("msg_shadow_xoffset", "0"));
	options.insert(option("msg_shadow_yoffset", "0"));
	options.insert(option("msg_shadow_color","#FFFFFF"));

	options.insert(option("session_color","#FFFFFF"));
	options.insert(option("session_font","Verdana:size=16:bold"));
	options.insert(option("session_x","50%"));
	options.insert(option("session_y","90%"));
	options.insert(option("session_shadow_xoffset", "0"));
	options.insert(option("session_shadow_yoffset", "0"));
	options.insert(option("session_shadow_color","#FFFFFF"));

	// slimlock-specific options
	options.insert(option("dpms_standby_timeout", "60"));
	options.insert(option("dpms_off_timeout", "600"));
	options.insert(option("wrong_passwd_timeout", "2"));
	options.insert(option("passwd_feedback_x", "50%"));
	options.insert(option("passwd_feedback_y", "10%"));
	options.insert(option("passwd_feedback_msg", "Authentication failed"));
	options.insert(option("passwd_feedback_capslock", "Authentication failed (CapsLock is on)"));
	options.insert(option("show_username", "1"));
	options.insert(option("show_welcome_msg", "0"));
	options.insert(option("tty_lock", "1"));
	options.insert(option("bell", "1"));

	error = "";
}

Cfg::~Cfg() {
	options.clear();
}
/*
 * Creates the Cfg object and parses
 * known options from the given configfile / themefile
 */
bool Cfg::readConf(string configfile) {
	int n = -1;
	size_t pos = 0;
	string line, next, op, fn(configfile);
	map<string,string>::iterator it;
	ifstream cfgfile(fn.c_str());

	if (!cfgfile) {
		error = "Cannot read configuration file: " + configfile;
		return false;
	}
	while (getline(cfgfile, line)) {
		if ((pos = line.find('\\')) != string::npos) {
			if (line.length() == pos + 1) {
				line.replace(pos, 1, " ");
				next = next + line;
				continue;
			} else
				line.replace(pos, line.length() - pos, " ");
		}

		if (!next.empty()) {
			line = next + line;
			next = "";
		}
		it = options.begin();
		while (it != options.end()) {
			op = it->first;
			n = line.find(op);
			if (n == 0)
				options[op] = parseOption(line, op);
			++it;
		}
	}
	cfgfile.close();

	fillSessionList();

	return true;
}

/* Returns the option value, trimmed */
string Cfg::parseOption(string line, string option ) {
	return Trim( line.substr(option.size(), line.size() - option.size()));
}

const string& Cfg::getError() const {
	return error;
}

string& Cfg::getOption(string option) {
	return options[option];
}

/* return a trimmed string */
string Cfg::Trim( const string& s ) {
	if ( s.empty() ) {
		return s;
	}
	int pos = 0;
	string line = s;
	int len = line.length();
	while ( pos < len && isspace( line[pos] ) ) {
		++pos;
	}
	line.erase( 0, pos );
	pos = line.length()-1;
	while ( pos > -1 && isspace( line[pos] ) ) {
		--pos;
	}
	if ( pos != -1 ) {
		line.erase( pos+1 );
	}
	return line;
}

/* Return the welcome message with replaced vars */
string Cfg::getWelcomeMessage(){
	string s = getOption("welcome_msg");
	int n = s.find("%host");
	if (n >= 0) {
		string tmp = s.substr(0, n);
		char host[40];
		gethostname(host,40);
		tmp = tmp + host;
		tmp = tmp + s.substr(n+5, s.size() - n);
		s = tmp;
	}
	n = s.find("%domain");
	if (n >= 0) {
		string tmp = s.substr(0, n);;
		char domain[40];
		getdomainname(domain,40);
		tmp = tmp + domain;
		tmp = tmp + s.substr(n+7, s.size() - n);
		s = tmp;
	}
	return s;
}

int Cfg::string2int(const char* string, bool* ok) {
	char* err = 0;
	int l = (int)strtol(string, &err, 10);
	if (ok) {
		*ok = (*err == 0);
	}
	return (*err == 0) ? l : 0;
}

int Cfg::getIntOption(std::string option) {
	return string2int(options[option].c_str());
}

/* Get absolute position */
int Cfg::absolutepos(const string& position, int max, int width) {
	int n = position.find("%");
	if (n>0) { /* X Position expressed in percentage */
		int result = (max*string2int(position.substr(0, n).c_str())/100) - (width / 2);
		return result < 0 ? 0 : result ;
	} else { /* Absolute X position */
		return string2int(position.c_str());
	}
}

/* split a comma separated string into a vector of strings */
void Cfg::split(vector<string>& v, const string& str, char c, bool useEmpty) {
	v.clear();
	string::const_iterator s = str.begin();
	string tmp;
	while (true) {
		string::const_iterator begin = s;
		while (*s != c && s != str.end()) { ++s; }
	tmp = string(begin, s);
	if (useEmpty || tmp.size() > 0)
			v.push_back(tmp);
		if (s == str.end()) {
			break;
		}
		if (++s == str.end()) {
		if (useEmpty)
				v.push_back("");
			break;
		}
	}
}

void Cfg::fillSessionList(){
	string strSessionDir  = getOption("sessiondir");

	sessions.clear();

	if( !strSessionDir.empty() ) {
		DIR *pDir = opendir(strSessionDir.c_str());

		if (pDir != NULL) {
			struct dirent *pDirent = NULL;

			while ((pDirent = readdir(pDir)) != NULL) {
				string strFile(strSessionDir);
				strFile += "/";
				strFile += pDirent->d_name;

				struct stat oFileStat;

				if (stat(strFile.c_str(), &oFileStat) == 0) {
                    if (S_ISREG(oFileStat.st_mode) &&
                            access(strFile.c_str(), R_OK) == 0){
                        ifstream desktop_file( strFile.c_str() );
                        if (desktop_file){
                             string line, session_name = "", session_exec = "";
                             while (getline( desktop_file, line )) {
                                 if (line.substr(0, 5) == "Name=") {
                                     session_name = line.substr(5);
                                     if (!session_exec.empty())
                                         break;
                                 } else
                                     if (line.substr(0, 5) == "Exec=") {
                                         session_exec = line.substr(5);
                                         if (!session_name.empty())
                                             break;
                                     }
                             }
                             desktop_file.close();
                             pair<string,string> session(session_name,session_exec);
                             sessions.push_back(session);
                             cout << session_exec << " - " << session_name << endl;
                        }

					}
				}
			}
			closedir(pDir);
		}
	}

	if (sessions.empty()){
        pair<string,string> session("","");
        sessions.push_back(session);
	}
}

pair<string,string> Cfg::nextSession() {
	currentSession = (currentSession + 1) % sessions.size();
	return sessions[currentSession];
}
