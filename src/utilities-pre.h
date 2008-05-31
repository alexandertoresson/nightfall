#ifndef __UTILITIES_H_PRE__
#define __UTILITIES_H_PRE__

#ifdef DEBUG_DEP
#warning "utilities.h-pre"
#endif

#include <string>
#include <map>

typedef std::map<std::string, std::string>           Hashtable;
typedef std::map<std::string, std::string>::iterator HashtableIterator;

namespace Utilities
{
	class ConfigurationFile;
	int power_of_two(int);
	bool IsOGLExtensionSupported(const char *extension);
	float RandomDegree();
}

#endif

