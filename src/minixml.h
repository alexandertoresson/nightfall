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
	struct XMLElement;

	typedef void (*TextFunc)(std::string);

	typedef void (*TagFunc)(XMLElement*);
	typedef std::map<std::string, TagFunc> TagFuncMap;

	typedef std::map<std::string, std::string> AttrList;
	typedef void (*AttrFunc)(XMLElement*, std::string);
	typedef std::map<std::string, AttrFunc> AttrFuncMap;

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
		virtual void Apply(TagFuncMap tag_funcs, TextFunc text_func);
		virtual void Apply(std::string tag, TagFunc tag_func);
		virtual void Apply(TextFunc text_func);
	};

	struct XMLTextNode : XMLNode
	{
		std::string str;
		virtual void Apply(TagFuncMap tag_funcs, TextFunc text_func);
		virtual void Apply(TextFunc text_func);
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
		
		void Iterate(TagFuncMap tag_funcs, TextFunc text_func);
		void Iterate(std::string tag, TagFunc tag_func);
		void Iterate(TextFunc text_func);
		void Iterate(AttrFuncMap attr_funcs);

		std::string GetAttribute(std::string attr, std::string def);
		std::string GetAttribute(std::string attr);
		bool HasAttribute(std::string attr);
		
		virtual void Apply(TagFuncMap tag_funcs, TextFunc text_func);
		virtual void Apply(std::string tag, TagFunc tag_func);
	
		unsigned Count(std::string tag);
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
		bool IsWhitespace(char c);
		std::string CleanWhitespace(std::string s);
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

