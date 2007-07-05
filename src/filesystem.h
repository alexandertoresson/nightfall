#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <vector>
#include <string>
#include <assert.h>
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
	
	void DeallocateFSData(FSData*);
	void DeallocateFSDataList(FSDataList*);
	
	std::string FSTimestampToString(const FSTimestamp* time);
}

#endif

