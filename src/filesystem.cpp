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
#include <iostream>

namespace Utilities
{
	int  ListFilesInDirectory(std::string directory, FSDataList* list)
	{
		assert(directory.size() > 0);
		assert(list != NULL);
		
#ifdef WIN32
		if (directory.at(directory.size()-1) != '\\')
			directory += '\\';

		char search[MAX_PATH];
		strcpy_s(search, MAX_PATH, directory.c_str());
		strcat_s(search, "*");
		
		// Warning: not tested!
		WIN32_FIND_DATA fd;
		HANDLE hFile = FindFirstFile(search, &fd);
		do
		{
			// Do some magic...
			
			 FSData* entry = new FSData;
			 entry->name = fd.cFileName;
			 entry->lastModified = new FSTimestamp;
			 
			 SYSTEMTIME time = { 0, 0, 0, 0, 0, 0, 0, 0 };
			 FileTimeToSystemTime(&fd.ftLastWriteTime, &time);
			 
			 entry->lastModified->nanoSeconds = 0;
			 entry->lastModified->seconds = time.wSecond;

			 if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				entry->isDirectory = true;
			 
		} while (FindNextFile(hFile, &fd));
		
		FindClose(hFile);
#else
		if (directory.at(directory.size()-1) != '/')
			directory += '/';

		DIR* h = opendir(directory.c_str());
		
		if (h == NULL)
			return ERROR_GENERAL;
			
		dirent* data;
		while (true)
		{
			data = readdir(h);
			
			if (data == NULL)
				break;
			
			if (data->d_name[0] == ',')
				continue;
			
			FSData* entry = new FSData;
			entry->name = data->d_name;
			
			{
				struct stat info;
				std::string file = directory + entry->name;
				
				if (stat(file.c_str(), &info) == 0)
				{
					if ((info.st_mode & S_IFMT) == S_IFDIR)
						entry->isDirectory = true;
						
					entry->size = info.st_size;
					
					entry->lastModified = new FSTimestamp;
#ifdef MAC
					entry->lastModified->seconds = info.st_mtimespec.tv_sec;
					entry->lastModified->nanoSeconds = info.st_mtimespec.tv_nsec;
#else
					entry->lastModified->seconds = 0;
					entry->lastModified->nanoSeconds = 0;
#endif
				}
			}
			
			list->push_back(entry);
		}
		
		closedir(h);
#endif

		return SUCCESS;
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
	
	void DeallocateFSData(FSData* ptr)
	{
		assert(ptr != NULL);
		
		if (ptr->lastModified != NULL)
			delete ptr->lastModified;
			
		delete ptr;
		ptr = NULL;
	}
	
	void DeallocateFSDataList(FSDataList* list)
	{
		assert(list != NULL);
		
		for (int i = list->size()-1; i >= 0; i--)
		{
			DeallocateFSData(list->at(i));
		}
		
		list->clear();
	}
	
	std::string FSTimestampToString(const FSTimestamp* time)
	{
#ifndef WIN32
		time_t t = time->seconds;
		return ctime(&t);
#else
		return "";
#endif
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

