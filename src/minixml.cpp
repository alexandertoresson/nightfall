#include "minixml.h"
#include <string>
#include <iostream>
#include <locale>

namespace Utilities
{
	XMLWriter::XMLWriter()
	{
		last_was_push = false;
		deferred_tag_ending = false;
	}

	bool XMLWriter::Open(std::string filename)
	{
		ofile.open(filename.c_str());
		ofile.precision(16);
		ofile << std::scientific;
		ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
		return true;
	}

	void XMLWriter::Close()
	{
		ofile.close();
	}

	void XMLWriter::PrepareBeginTag(std::string tag)
	{
		if (tags.size() > 0)
		{
			if (last_was_push)
			{
				ofile << std::endl;
			}
			for (unsigned i = 0; i < tags.size(); i++)
			{
				ofile << "\t";
			}
		}
		tags.push(tag);
		last_was_push = true;
	}

	void XMLWriter::BeginTag(std::string tag)
	{
		FinishTag();
		PrepareBeginTag(tag);
		ofile << "<" << tag;
		deferred_tag_ending = true;
	}

	void XMLWriter::BeginTag(std::string tag, AttrList attributes)
	{
		FinishTag();
		PrepareBeginTag(tag);
		ofile << "<" << tag << " ";
		for (AttrList::iterator it = attributes.begin(); it != attributes.end(); )
		{
			ofile << it->first << "=\"" << it->second << "\"";
			it++;
			if (it != attributes.end())
			{
				ofile << " ";
			}
		}
		deferred_tag_ending = true;
	}

	void XMLWriter::FinishTag()
	{
		if (deferred_tag_ending)
		{
			ofile << ">";
			deferred_tag_ending = false;
		}
	}

	void XMLWriter::Write(std::string text)
	{
		FinishTag();
		ofile << text;
	}

	void XMLWriter::Write(char* text)
	{
		FinishTag();
		ofile << text;
	}

	void XMLWriter::Write(int i)
	{
		FinishTag();
		ofile << i;
	}

	void XMLWriter::Write(Uint32 i)
	{
		FinishTag();
		ofile << i;
	}

	void XMLWriter::Write(float f)
	{
		FinishTag();
		ofile << f;
	}

	void XMLWriter::Write(double d)
	{
		FinishTag();
		ofile << d;
	}

	void XMLWriter::EndTag()
	{
		if (deferred_tag_ending)
		{
			ofile << "/>" << std::endl;
			deferred_tag_ending = false;
			tags.pop();
		}
		else
		{
			std::string tag = tags.top();
			tags.pop();
			if (!last_was_push)
			{
				for (unsigned i = 0; i < tags.size(); i++)
				{
					ofile << "\t";
				}
			}
			ofile << "</" << tag << ">" << std::endl;
		}
		last_was_push = false;
	}
	
	XMLReader::XMLReader()
	{
		xmlElementAlloc = new ChunkAllocator<XMLElement>(65536);
		xmlTextNodeAlloc = new ChunkAllocator<XMLTextNode>(65536);
		root = NULL;
	}

	XMLReader::~XMLReader()
	{
		delete xmlElementAlloc;
		delete xmlTextNodeAlloc;
	}

	void XMLReader::Deallocate()
	{
		xmlTextNodeAlloc->DeallocChunks();
		xmlElementAlloc->DeallocChunks();
		root = NULL;
	}

	std::string XMLReader::ReadTag(bool &open, AttrList &attributes)
	{
		std::string tag = "";
		bool tag_read = false;
		int attr_state = 0;
		std::string attr, val;
		while (!ifile.eof())
		{
			char c;
			ifile.get(c);
			if (c == '>')
			{
				open = true;
				break;
			}
			if (c == '/')
			{
				char c2;
				ifile.get(c2);
				if (c2 == '>')
				{
					open = false;
					break;
				}
				else
				{
					ifile.putback(c2);
				}
			}
			if (tag_read)
			{
				switch (attr_state)
				{
					case 0:
						if (c == '=')
						{
							attr_state = 1;
						}
						else if (!IsWhitespace(c))
						{
							attr.push_back(c);
						}
						break;
					case 1:
						if (c == '"')
						{
							attr_state = 2;
						}
						break;
					case 2:
						if (c == '"')
						{
							attr_state = 0;
							attributes[attr] = val;
							attr = "";
							val = "";
						}
						else
						{
							val.push_back(c);
						}
						break;
				}
			}
			else
			{
				if (IsWhitespace(c))
				{
					tag_read = true;
					attr_state = 0;
					attr = "";
					val = "";
				}
				else
				{
					tag.push_back(c);
				}
			}
		}
		return tag;
	}
	
	std::string XMLReader::ReadTagEnd()
	{
		std::string tag = "";
		while (!ifile.eof())
		{
			char c;
			ifile.get(c);
			if (c == '>')
			{
				break;
			}
			tag.push_back(c);
		}
		return tag;
	}
	
	bool XMLReader::ReadDeclaration()
	{
		while (!ifile.eof())
		{
			char c;
			ifile.get(c);
			if (c == '>')
			{
				return true;
			}
		}
		return false;
	 }
	
	XMLElement *XMLReader::ReadTagBlock()
	{
		std::string tag1, tag2;
		char c;
		ifile.get(c);
		bool open;
		AttrList attributes;

		if (c == '?' || c == '!')
		{
			ReadDeclaration();
			return (XMLElement*) -1;
		}
		else
		{
			ifile.putback(c);
		}
		XMLElement *node;

		tag1 = ReadTag(open, attributes);
		if (open)
		{
			level++;

			node = ReadText();

			tag2 = ReadTagEnd();

			if (tag1 != tag2)
			{
				return NULL;
			}

			level--;
		}
		else
		{
			node = new XMLElement();
		}

		node->tag = tag1;
		node->attributes = attributes;

		return node;
	}

	bool XMLReader::IsWhitespace(char c)
	{
		return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	}

	std::string XMLReader::CleanWhitespace(std::string s)
	{
		std::string out;
		bool hasWS = false;
		for (std::string::iterator it = s.begin(); it != s.end(); it++)
		{
			char c = *it;
			if (IsWhitespace(c))
			{
				hasWS = true;
			}
			else
			{
				if (hasWS && out.length())
				{
					out.push_back(' ');
				}
				out.push_back(c);
			}
		}
		return out;
	}

	XMLElement *XMLReader::ReadText()
	{
		XMLElement *node = xmlElementAlloc->New();
		std::string text;
		while (!ifile.eof())
		{
			char c;
			ifile.get(c);
			if (c == '<')
			{
				if (text.length())
				{
					XMLTextNode *textNode = xmlTextNodeAlloc->New();
					textNode->str = CleanWhitespace(text);
					node->children.push_back(textNode);

					text = "";
				}
				
				ifile.get(c);
				
				if (c == '/')
				{
					return level != 0 ? node : NULL;
				}
				else
				{
					ifile.putback(c);
					
					XMLElement *elem = ReadTagBlock();
					if (elem != (XMLElement*) -1)
					{
						if (!elem)
						{
							return NULL;
						}
						elem->index = node->children.size();
						node->children.push_back(elem);
					}
				}
			}
			else
			{
				text.push_back(c);
			}
		}
		if (text.length())
		{
			XMLTextNode *textNode = xmlTextNodeAlloc->New();
			textNode->str = CleanWhitespace(text);
			node->children.push_back(textNode);
		}
		return node;
	}

	bool XMLReader::Read(std::string filename)
	{
		Deallocate();
		level = 0;
		ifile.open(filename.c_str());
		root = ReadText();
		ifile.close();
		return root != NULL;
	}

	void XMLNode::Apply(TagFuncMap tag_funcs, TextFunc text_func)
	{
		
	}

	void XMLNode::Apply(std::string tag, TagFunc tag_func)
	{
		
	}

	void XMLNode::Apply(TextFunc text_func)
	{
		
	}

	void XMLTextNode::Apply(TagFuncMap tag_funcs, TextFunc text_func)
	{
		if (text_func)
		{
			text_func(str);
		}
	}

	void XMLTextNode::Apply(TextFunc text_func)
	{
		if (text_func)
		{
			text_func(str);
		}
	}

	void XMLElement::Apply(TagFuncMap tag_funcs, TextFunc text_func)
	{
		void (*tag_func)(XMLElement *elem) = tag_funcs[tag];
		if (tag_func)
		{
			tag_func(this);
		}
	}

	void XMLElement::Apply(std::string tag, TagFunc tag_func)
	{
		if (this->tag == tag)
		{
			tag_func(this);
		}
	}

	void XMLElement::Iterate(TagFuncMap tag_funcs, TextFunc text_func)
	{
		for (std::vector<XMLNode*>::iterator it = children.begin(); it != children.end(); it++)
		{
			(*it)->Apply(tag_funcs, text_func);
		}
	}
		
	void XMLElement::Iterate(std::string tag, TagFunc tag_func)
	{
		for (std::vector<XMLNode*>::iterator it = children.begin(); it != children.end(); it++)
		{
			(*it)->Apply(tag, tag_func);
		}
	}

	void XMLElement::Iterate(TextFunc text_func)
	{
		for (std::vector<XMLNode*>::iterator it = children.begin(); it != children.end(); it++)
		{
			(*it)->Apply(text_func);
		}
	}
		
	void XMLElement::Iterate(AttrFuncMap attr_funcs)
	{
		for (AttrList::iterator it = attributes.begin(); it != attributes.end(); it++)
		{
			std::string attr = it->first;
			AttrFunc f = attr_funcs[attr];
			if (f)
			{
				f(this, it->second);
			}
		}
	}
	
	std::string XMLElement::GetAttribute(std::string attr, std::string def)
	{
		AttrList::iterator it = attributes.find(attr);
		if (it != attributes.end())
		{
			return it->second;
		}
		return def;
	}
	
	std::string XMLElement::GetAttribute(std::string attr)
	{
		return GetAttribute(attr, "");
	}
	
	bool XMLElement::HasAttribute(std::string attr)
	{
		return attributes.find(attr) != attributes.end();
	}
	
	unsigned XMLElement::Count(std::string tag)
	{
		unsigned n = 0;
		for (std::vector<XMLNode*>::iterator it = children.begin(); it != children.end(); it++)
		{
			XMLElement *elem = dynamic_cast<XMLElement*>(*it);
			if (elem && elem->tag == tag)
			{
				n++;
			}
		}
		return n;
	}

}
