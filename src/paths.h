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

#ifndef __PATHS_H__
#define __PATHS_H__

#ifdef DEBUG_DEP
#warning "paths.h"
#endif

#include <string>

namespace Utilities
{
	
	void InitPaths(std::string programpath);

	enum PathType
	{
		PATHTYPE_CONFIG,
		PATHTYPE_DATA
	};

	std::string GetPath(PathType type, unsigned num);
	std::string GetFile(PathType type, std::string file);
	std::string GetWritableConfigFile(std::string file, bool &exists);
	std::string GetWritableDataFile(std::string file, bool &exists);
	std::string GetConfigFile(std::string file);
	std::string GetDataFile(std::string file);
}

#ifdef DEBUG_DEP
#warning "paths.h-end"
#endif

#endif

