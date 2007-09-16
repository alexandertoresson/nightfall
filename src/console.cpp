#include "console.h"

Console console;

Console::Console()
{
	m_buffer = new std::string[1024];

	lineHeight = 0.0f;
	translate = 0;
	lineCount = 1024;
	start = 0;
	end = 0;

	dump = true;
}

Console::~Console()
{
	if (dump)
	{
		// Dump buffer to file
		std::ofstream file("console.txt");
		for (int i = 0; i < lineCount; i++)
		{
			if (m_buffer[i].length() > 0)
				file << m_buffer[i] << std::endl;
		}
		file.close();
	}
	delete [] m_buffer;
	m_buffer = NULL;
}

const std::string* Console::GetBuffer() const
{
	return m_buffer;
}

void Console::SetDumpFlag(bool flag)
{
	dump = flag;
}

void Console::SetLineHeight(float value, int vlines)
{
	lineHeight = value;
	visibleLines = vlines;
	translate = lineCount - vlines;
}

inline void Console::IncrementPos(void)
{
	if(end + 1 == lineCount)
	{
		end = 0;
	}
	else
	{
		end++;
	}

	if(start + 1 == lineCount)
	{
		start = 0;
	}
	else
	{
		start++;
	}

	m_buffer[end] = "";
}

inline void Console::CheckTranslation(void)
{
	if(translate < lineCount - visibleLines)
	{
		translate--;
		if(translate < 0)
			translate = 0;
	}
}

void Console::PutChar(char c)
{
	if (c == Console::nl)
	{
		IncrementPos();
		CheckTranslation();
	}
	else
		m_buffer[end].push_back(c);
}

void Console::Write(std::string message)
{
	size_t index = std::string::npos;
	size_t pos = 0;
	int nth = 0;

	while (true)
	{
		index = message.find(Console::nl, nth);
		if (index == std::string::npos)
		{
			m_buffer[end] += message.substr(pos, message.length() - pos);
			break;
		}
		console.WriteLine(message.substr(pos, index - pos));
		pos = index;
		nth++;
	}
}

void Console::WriteLine(std::string message)
{
	m_buffer[end] += message;
	IncrementPos();
	CheckTranslation();
}

template <typename T>
inline void WriteToConsole(T value)
{
	std::stringstream ss("");
	ss << value;

	console.Write(ss.str());
}

Console& operator << (Console& c, int value)
{
	WriteToConsole<int>(value);

	return c;
}

Console& operator << (Console& c, unsigned int value)
{
	WriteToConsole<unsigned int>(value);

	return c;
}

Console& operator << (Console& c, long value)
{
	WriteToConsole<long>(value);

	return c;
}

Console& operator << (Console& c, unsigned long value)
{
	WriteToConsole<unsigned long>(value);

	return c;
}

Console& operator << (Console& c, short value)
{
	WriteToConsole<short>(value);

	return c;
}

Console& operator << (Console& c, long double value)
{
	WriteToConsole<long double>(value);

	return c;
}

Console& operator << (Console& c, float value)
{
	WriteToConsole<float>(value);

	return c;
}

Console& operator << (Console& c, double value)
{
	WriteToConsole<double>(value);

	return c;
}

Console& operator << (Console& c, char value)
{
	console.PutChar(value);

	return c;
}

Console& operator << (Console& c, char* values)
{
	console.Write(values);

	return c;
}

Console& operator << (Console& c, const char* values)
{
	console.Write(values);

	return c;
}

Console& operator << (Console& c, const void* ptr)
{
	char adress[32];
#ifndef WIN32
	sprintf(adress, "%p", ptr);
#else
	sprintf_s(adress, "%p", ptr);
#endif
	console.Write(adress);

	return c;
}

Console& operator << (Console& c, std::string str)
{
	console.Write(str);

	return c;
}

/*#ifdef _GLIBCXX_USE_LONG_LONG
Console& operator << (Console& c, unsigned long long value)
{
	WriteToConsole<unsigned long long>(value);

	return c;
}

Console& operator << (Console& c, long long value)
{
	WriteToConsole<long long>(value);

	return c;
}
#endif*/
