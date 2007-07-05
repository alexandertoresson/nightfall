/*
 *  utilfs.cpp
 *  Projektarbete
 *
 *  Created by Leonard Wickmark on 2007-01-11.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
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
		WIN32_FIND_DATAA fd;
		HANDLE hFile = FindFirstFileA(search, &fd);
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
			 
		} while (FindNextFileA(hFile, &fd));
		
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
					if (info.st_mode & S_IFMT == S_IFDIR)
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
	
	bool FileExists(std::string file)
	{
		std::fstream test(file.c_str(), std::fstream::in);
		
		if (!test.good())
		{
			test.close();
		
			return false;
		}
		
		test.close();
		
		return true;
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
}

