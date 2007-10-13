
#include <string>

#ifndef __PATHS_H__
#define __PATHS_H__

#ifdef DEBUG_DEP
#warning "paths.h"
#endif

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

#define __PATHS_H_END__

#endif

