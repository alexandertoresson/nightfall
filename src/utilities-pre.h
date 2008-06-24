#ifndef __UTILITIES_H_PRE__
#define __UTILITIES_H_PRE__

#ifdef DEBUG_DEP
#warning "utilities.h-pre"
#endif

#include <string>
#include <map>

namespace Utilities
{
	int power_of_two(int);
	bool IsOGLExtensionSupported(const char *extension);
	float RandomDegree();
	
	// Ensures that target consists of numbers. If letters were found, no is returned.
	int StringToInt(std::string target, int no = 0);
		
	//Trims ' ' and '\t' from string.
	void StringTrim(std::string target, std::string& result);
	
	void ReadLineFromFile(std::ifstream& file, std::string& buffer);
}

#endif

