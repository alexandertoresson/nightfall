#ifndef __MINIXML_H__
#define __MINIXML_H__

#include "sdlheader.h"
#include "chunkallocator.h"
#include <string>
#include <fstream>
#include <stack>
#include <vector>
#include <map>

#ifdef DEBUG_DEP
#warning "minixml.h"
#endif

namespace Utilities
{
	
	typedef std::map<std::string, std::string> AttrList;

	class XMLWriter
	{
		private:
		std::ofstream ofile;
		std::stack<std::string> tags;
		bool last_was_push;
		bool deferred_tag_ending;

		void PrepareBeginTag(std::string tag);
		void FinishTag();

		public:
		XMLWriter();
		bool Open(std::string filename);
		void Close();
		void BeginTag(std::string tag);
		void BeginTag(std::string tag, AttrList attributes);
		void Write(std::string text);
		void Write(char* text);
		void Write(int i);
		void Write(Uint32 i);
		void Write(float f);
		void Write(double d);
		void EndTag();
	};

	struct XMLElement;

	struct XMLNode
	{
		XMLNode* parent;
		XMLNode()
		{
			parent = NULL;
		}
		virtual void Apply(std::map<std::string, void (*)(XMLElement *elem)> tag_funcs, void (*text_func)(std::string text));
		virtual void Apply(std::string tag, void (*tag_func)(XMLElement *elem));
		virtual void Apply(void (*text_func)(std::string text));
	};

	struct XMLTextNode : XMLNode
	{
		std::string str;
		virtual void Apply(std::map<std::string, void (*)(XMLElement *elem)> tag_funcs, void (*text_func)(std::string text));
		virtual void Apply(void (*text_func)(std::string text));
	};

	struct XMLElement : XMLNode
	{
		unsigned index;
		std::string tag;
		AttrList attributes;

		std::vector<XMLNode*> children;
		XMLElement()
		{
			index = 0;
		}
		
		void Iterate(std::map<std::string, void (*)(XMLElement *elem)> tag_funcs, void (*text_func)(std::string text));
		void Iterate(std::string tag, void (*tag_func)(XMLElement *elem));
		void Iterate(void (*text_func)(std::string text));
		
		virtual void Apply(std::map<std::string, void (*)(XMLElement *elem)> tag_funcs, void (*text_func)(std::string text));
		virtual void Apply(std::string tag, void (*tag_func)(XMLElement *elem));
	};

	class XMLReader
	{
		private:
		std::ifstream ifile;
		std::stack<std::string> tags;
		int level;
		ChunkAllocator<XMLElement> *xmlElementAlloc;
		ChunkAllocator<XMLTextNode> *xmlTextNodeAlloc;

		std::string ReadTag(bool &open, std::map<std::string, std::string> &attributes);
		std::string ReadTagEnd();
		bool ReadDeclaration();
		XMLElement *ReadTagBlock();
		XMLElement *ReadText();

		public:
		XMLReader();
		~XMLReader();
		bool Read(std::string filename);
		void Deallocate();
		XMLElement *root;
	};
}

#ifdef DEBUG_DEP
#warning "minixml.h-end"
#endif

#endif

