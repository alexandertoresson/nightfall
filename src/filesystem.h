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
#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <vector>
#include <string>
#include <cassert>
#include <fstream>
#include "sdlheader.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <dirent.h>
#endif

#include "errors.h"

namespace Utilities
{
	struct FSTimestamp
	{
		long seconds;
        long nanoSeconds;
	};

	struct FSData
	{
		std::string name;
		long         size;
		bool         isDirectory;
		FSTimestamp* lastModified;
		
		FSData()
		{
			size = 0;
			isDirectory = false;
			lastModified = NULL;
		}
	};
	
	typedef std::vector< FSData* > FSDataList;
	
	int  ListFilesInDirectory(std::string directory, FSDataList*);
	bool FileExists(std::string file);
	bool CanOpenForWriting(std::string file);
	std::string GetDirectoryInPath(std::string path);
	void CreateDirectory(std::string directory);
	
	void DeallocateFSData(FSData*);
	void DeallocateFSDataList(FSDataList*);
	
	std::string FSTimestampToString(const FSTimestamp* time);
}

#endif

