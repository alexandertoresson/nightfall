#ifndef __MINIXML_H__
#define __MINIXML_H__

#include <string>
#include <fstream>
#include <stack>
#include "sdlheader.h"

#ifdef DEBUG_DEP
#warning "minixml.h"
#endif

namespace Utilities
{
	class XMLWriter
	{
		private:
		std::ofstream ofile;
		std::stack<std::string> tags;
		bool last_was_push;

		public:
		XMLWriter();
		bool Open(std::string filename);
		void Close();
//		bool OpenForRead(std::string filename);
		void BeginTag(std::string tag);
		void Write(std::string text);
		void Write(char* text);
		void Write(int i);
		void Write(Uint32 i);
		void Write(float f);
		void Write(double d);
		void EndTag();
	};
}

#ifdef DEBUG_DEP
#warning "minixml.h-end"
#endif

#define __MINIXML_H_END__

#endif

