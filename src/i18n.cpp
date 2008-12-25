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

#include "i18n.h"

#include "utilities.h"

#ifdef ENABLE_NLS

std::string _(std::string x)
{
	return gettext(x.c_str());
}

std::string n_(std::string s, std::string p, int n)
{
	return ngettext(s.c_str(), p.c_str(), n);
}

#else

std::string _(std::string x)
{
	return x;
}

std::string n_(std::string s, std::string p, int n)
{
	return n == 1 ? s : p;
}

#endif // ENABLE_NLS

std::string s_(std::string x, ...)
{
	va_list v;
	va_start(v, x);
	std::string str = Utilities::stdvsprintf(_(x), v);
	va_end(v);
	return str;
}

std::string sn_(std::string s, std::string p, int n , ...)
{
	va_list v;
	va_start(v, n);
	std::string str = Utilities::stdvsprintf(n_(s, p, n), v);
	va_end(v);
	return str;
}
