#include "minixml.h"
#include <string>
#include <iostream>

namespace Utilities
{
	XMLWriter::XMLWriter()
	{
		
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

	void XMLWriter::BeginTag(std::string tag)
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
		ofile << "<" << tag << ">";
		tags.push(tag);
		last_was_push = true;
	}

	void XMLWriter::Write(std::string text)
	{
		ofile << text;
	}

	void XMLWriter::Write(char* text)
	{
		ofile << text;
	}

	void XMLWriter::Write(int i)
	{
		ofile << i;
	}

	void XMLWriter::Write(Uint32 i)
	{
		ofile << i;
	}

	void XMLWriter::Write(float f)
	{
		ofile << f;
	}

	void XMLWriter::Write(double d)
	{
		ofile << d;
	}

	void XMLWriter::EndTag()
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
		last_was_push = false;
	}
		
	XMLReader::XMLReader()
	{
		data = NULL;
	}

	XMLReader::~XMLReader()
	{
		Deallocate();
	}

	void XMLReader::Deallocate(XMLData *data)
	{
		for (unsigned i = 0; i < data->items.size(); i++)
		{
			if (data->types[i] == XMLTYPE_STRING)
			{
				delete data->items[i].str;
			}
			else
			{
				Deallocate(data->items[i].xmlData);
			}
		}
		data->items.clear();
		data->types.clear();
		data->itemsByTag.clear();
		delete data;
	}

	void XMLReader::Deallocate()
	{
		if (data)
		{
			Deallocate(data);
			data = NULL;
		}
	}

	std::string XMLReader::ReadTag()
	{
		std::string tag = "";
		while (!ifile.eof())
		{
			char c;
			ifile >> c;
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
			ifile >> c;
			if (c == '>')
			{
				return true;
			}
		}
		return false;
	 }
	
	XMLData *XMLReader::ReadTagBlock()
	{
		std::string tag1, tag2;
		char c;
		ifile >> c;

		if (c == '?')
		{
			ReadDeclaration();
			return (XMLData*) -1;
		}
		else
		{
			ifile.putback(c);
		}
		XMLData *data;

		tag1 = ReadTag();
		level++;

		data = ReadText();

		tag2 = ReadTag();

		if (tag1 != tag2)
		{
			return NULL;
		}

		data->tag = tag1;

		level--;

		return data;
	}

	XMLData *XMLReader::ReadText()
	{
		XMLData *data = new XMLData;
		std::string *text = new std::string;
		while (!ifile.eof())
		{
			char c;
			ifile >> c;
			if (c == '<')
			{
				if (text->length())
				{
					XMLItem item;
					item.str = text;
					data->items.push_back(item);
					data->types.push_back(XMLTYPE_STRING);
					text = new std::string;
				}
				ifile >> c;
				if (c == '/')
				{
					delete text;
					return level != 0 ? data : NULL;
				}
				else
				{
					ifile.putback(c);
					
					XMLItem item;
					item.xmlData = ReadTagBlock();
					if (item.xmlData != (XMLData*) -1)
					{
						if (!item.xmlData)
						{
							delete text;
							return NULL;
						}
						data->items.push_back(item);
						data->types.push_back(XMLTYPE_DATA);
						std::map<std::string, std::vector<XMLData*> >::iterator it = data->itemsByTag.find(item.xmlData->tag);
						if (it != data->itemsByTag.end())
						{
							item.xmlData->index = data->itemsByTag[item.xmlData->tag].size();
							data->itemsByTag[item.xmlData->tag].push_back(item.xmlData);
						}
						else
						{
							std::vector<XMLData*> vec;
							vec.push_back(item.xmlData);
							data->itemsByTag.insert(make_pair(item.xmlData->tag, vec));
						}
					}
				}
			}
			else
			{
				text->push_back(c);
			}
		}
		if (text->length())
		{
			XMLItem item;
			item.str = text;
			data->items.push_back(item);
			data->types.push_back(XMLTYPE_STRING);
			text = new std::string;
		}
		else
		{
			delete text;
		}
		return data;
	}

	bool XMLReader::Read(std::string filename)
	{
		Deallocate();
		level = 0;
		ifile.open(filename.c_str());
		data = ReadText();
		ifile.close();
		return data != NULL;
	}

	void XMLReader::Iterate(XMLData *data, std::map<std::string, void (*)(XMLData *data)> tag_funcs, void (*text_func)(std::string text))
	{
		for (unsigned i = 0; i < data->items.size(); i++)
		{
			if (data->types[i] == XMLTYPE_STRING && text_func)
			{
				text_func(*data->items[i].str);
			}
			else
			{
				void (*tag_func)(XMLData *data) = tag_funcs[data->items[i].xmlData->tag];
				if (tag_func)
				{
					tag_func(data->items[i].xmlData);
				}
			}
		}
	}
		
	void XMLReader::Iterate(XMLData *data, std::string tag, void (*tag_func)(XMLData *data), void (*text_func)(std::string text))
	{
		std::map<std::string, void (*)(XMLData *data)> tag_funcs;
		tag_funcs[tag] = tag_func;
		Iterate(data, tag_funcs, text_func);
	}

	void XMLReader::Iterate(XMLData *data, std::string tag, void (*tag_func)(XMLData *data))
	{
		std::map<std::string, std::vector<XMLData*> >::iterator it = data->itemsByTag.find(tag);
		if (it != data->itemsByTag.end())
		{
			for (std::vector<XMLData*>::iterator it_vec = it->second.begin(); it_vec != it->second.end(); it_vec++)
			{
				tag_func(*it_vec);
			}
		}
	}

	void XMLReader::Iterate(XMLData *data, void (*text_func)(std::string text))
	{
		for (unsigned i = 0; i < data->items.size(); i++)
		{
			if (data->types[i] == XMLTYPE_STRING && text_func)
			{
				text_func(*data->items[i].str);
			}
		}
	}
		
	void XMLReader::Iterate(std::map<std::string, void (*)(XMLData *data)> tag_funcs, void (*text_func)(std::string text))
	{
		Iterate(data, tag_funcs, text_func);
	}

	void XMLReader::Iterate(std::string tag, void (*tag_func)(XMLData *data), void (*text_func)(std::string text))
	{
		Iterate(data, tag, tag_func, text_func);
	}
	
	void XMLReader::Iterate(std::string tag, void (*tag_func)(XMLData *data))
	{
		Iterate(data, tag, tag_func);
	}
	
}
