/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __I18N_H__
#define __I18N_H__ 

#include "sdlheader.h"

#ifdef ENABLE_NLS
#include "gettext.h"
#endif

#include <string>

#define N_(x) (x)

// pure translation
std::string _(std::string x);

// translation with plurals handlning
std::string n_(std::string s, std::string p, int n);

// translation with sprintf
std::string s_(std::string x, ...);

// translation with sprintf and plurals handling
std::string sn_(std::string s, std::string p, int n, ...);

#endif // __I18N_H__
