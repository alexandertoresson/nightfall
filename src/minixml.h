#ifndef __MINIXML_H__
#define __MINIXML_H__

#include <string>
#include <fstream>
#include <stack>
#include <vector>
#include <map>
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

	enum XMLType
	{
		XMLTYPE_STRING,
		XMLTYPE_DATA
	};

	struct XMLData;

	union XMLItem
	{
		std::string *str;
		XMLData *xmlData;
	};

	struct XMLData
	{
		// public, feel free to read
		unsigned index;
		std::string tag;

		// private; implementation-specific
		std::vector<XMLType> types;
		std::vector<XMLItem> items;
		std::map<std::string, std::vector<XMLData*> > itemsByTag;
		XMLData()
		{
			index = 0;
		}
	};

	class XMLReader
	{
		private:
		std::ifstream ifile;
		std::stack<std::string> tags;
		int level;
		XMLData *data;

		std::string ReadTag();
		bool ReadDeclaration();
		XMLData *ReadTagBlock();
		XMLData *ReadText();
		void Deallocate(XMLData *data);

		public:
		XMLReader();
		~XMLReader();
		bool Read(std::string filename);
		void Iterate(XMLData *data, std::map<std::string, void (*)(XMLData *data)> tag_funcs, void (*text_func)(std::string text));
		void Iterate(XMLData *data, std::string tag, void (*tag_func)(XMLData *data), void (*text_func)(std::string text));
		void Iterate(XMLData *data, std::string tag, void (*tag_func)(XMLData *data));
		void Iterate(XMLData *data, void (*text_func)(std::string text));
		void Iterate(std::map<std::string, void (*)(XMLData *data)> tag_funcs, void (*text_func)(std::string text));
		void Iterate(std::string tag, void (*tag_func)(XMLData *data), void (*text_func)(std::string text));
		void Iterate(std::string tag, void (*tag_func)(XMLData *data));
		void Deallocate();
	};
}

#ifdef DEBUG_DEP
#warning "minixml.h-end"
#endif

#define __MINIXML_H_END__

#endif

