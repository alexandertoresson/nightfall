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
#include "filesystem.h"
#include <Poco/DirectoryIterator.h>
#include <Poco/File.h>
#include <iostream>

namespace Utilities
{
	bool ListFilesInDirectory(const std::string& directory, FSEntryList& list)
	{
		Poco::DirectoryIterator end;
		for (Poco::DirectoryIterator it = Poco::DirectoryIterator(directory); it != end; ++it)
		{
			FSEntry entry;
			entry.size = it->getSize();
			entry.isDirectory = it->isDirectory();
			entry.lastModified = it->getLastModified().epochTime();
			list[it.name()] = entry;
		}
		return true;
	}
	
	bool FileExists(const std::string& file)
	{
		return Poco::File(file).exists();
	}

	bool FileIsReadable(const std::string& file)
	{
		return std::ifstream(file.c_str()).good();
	}

	bool FileIsWritable(const std::string& file)
	{
		if (std::ofstream(file.c_str()).good())
		{
			return true;
		}
		else
		{
			CreateDirectory(GetDirectoryInPath(file));
			return std::ofstream(file.c_str()).good();
		}
	}
	
	std::string GetDirectoryInPath(const std::string& path)
	{
		//TODO: Use find_last_of()
		for (int i = path.size()-1; i >= 0; i--)
		{
			if (path[i] == '/' || path[i] == '\\')
			{
				return path.substr(0, i+1);
			}
		}
		return "";
	}

	void CreateDirectory(const std::string& directory)
	{
#ifdef WIN32
		// Attempt to create directories recursively
		if (!::CreateDirectory(directory.c_str(), NULL))
		{
			if (GetLastError() == ERROR_PATH_NOT_FOUND) // Parent directory does not exist, attempt to create it too
			{
				size_t pos = std::string.find_last_of("/\\");
				if (pos != std::string::npos)
				{
					CreateDirectory(directory.substr(0, pos));
					::CreateDirectory(directory.c_str(), NULL);
				}
			}
		}
#else
		system(("mkdir -p " + directory).c_str());
#endif
	}
}

