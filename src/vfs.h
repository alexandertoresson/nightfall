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
#ifndef VFS_H
#define VFS_H 

#ifdef DEBUG_DEP
	#warning "vfs.h"
#endif

#include "vfs-pre.h"
#include "filesystem-pre.h"

namespace Utilities
{
	namespace VFS
	{
		bool GetDirectoryListing(const std::string& path, FSEntryList& list, VFSLevel level = -1);
	}
}

#ifdef DEBUG_DEP
	#warning "vfs.h-end"
#endif

#endif

