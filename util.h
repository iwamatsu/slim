/* SLiM - Simple Login Manager
   Copyright (C) 2009 Eygene Ryabinkin <rea@codelabs.ru>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
*/
#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>

namespace Util {
	bool add_mcookie(const std::string &mcookie, const char *display,
	    const std::string &xauth_cmd, const std::string &authfile);

	void srandom(unsigned long seed);
	long random(void);

	long makeseed(void);
};

#endif /* __UTIL_H__ */
