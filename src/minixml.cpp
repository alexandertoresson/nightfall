#include "minixml.h"
#include <string>

namespace Utilities
{
	XMLWriter::XMLWriter()
	{
		
	}

	bool XMLWriter::Open(std::string filename)
	{
		ofile.open(filename.c_str());
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
}
