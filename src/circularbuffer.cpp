/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "circularbuffer.h"

#include <sstream>
#include <fstream>

using namespace std;

CircularBuffer::CircularBuffer(int lines, std::string filename)
{
	m_buffer = new std::string[lines];

	this->filename = filename;

	lineCount = lines;
	start = 0;
	end = 0;
	remlineCallback = NULL;
}

CircularBuffer::~CircularBuffer()
{
	if (filename.length())
	{
		// Dump buffer to file
		std::ofstream file(filename.c_str());
		int i = start;
		while (1) 
		{
			if (m_buffer[i].length() > 0)
				file << m_buffer[i] << std::endl;

			if (i == end)
			{
				break;
			}

			i++;
			if (i == lineCount)
			{
				i = 0;
			}
		}
		file.close();
	}

	delete [] m_buffer;
	m_buffer = NULL;
}

void CircularBuffer::SetFile(std::string filename)
{
	this->filename = filename;
}

void CircularBuffer::SetRemLineCallback(void (*remline_callback)())
{
	this->remlineCallback = remline_callback;
}

void CircularBuffer::IncrementPos(void)
{
	end++;
	if (end == lineCount)
	{
		end = 0;
	}

	if (end == start)
	{
		start++;
		if (start == lineCount)
		{
			start = 0;
		}
		if (remlineCallback)
		{
			remlineCallback();
		}
	}

	m_buffer[end] = "";
}

void CircularBuffer::Write(std::string message)
{
	size_t index = std::string::npos;
	size_t pos = 0;

	while (true)
	{
		index = message.find('\n', pos);
		if (index == std::string::npos)
		{
			m_buffer[end] += message.substr(pos, message.length() - pos);
			break;
		}
		WriteLine(message.substr(pos, index - pos));
		pos = index+1;
	}
}

void CircularBuffer::WriteLine(std::string message)
{
	m_buffer[end] += message;
	IncrementPos();
}

std::string CircularBuffer::GetLine(int n)
{
	int index = start + n;
	if (index >= lineCount)
	{
		index -= lineCount;
	}
	return m_buffer[index];
}

int CircularBuffer::GetLineCount()
{
	if (end < start)
	{
		return end - start + 1 + lineCount;
	}
	else
	{
		return end - start + 1;
	}
}

template <typename T>
void CircularBuffer::WriteToCircularBuffer(T value)
{
	std::stringstream ss("");
	ss << value;

	Write(ss.str());
}

CircularBuffer& operator << (CircularBuffer& c, int value)
{
	c.WriteToCircularBuffer<int>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, unsigned int value)
{
	c.WriteToCircularBuffer<unsigned int>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, long value)
{
	c.WriteToCircularBuffer<long>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, unsigned long value)
{
	c.WriteToCircularBuffer<unsigned long>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, short value)
{
	c.WriteToCircularBuffer<short>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, long double value)
{
	c.WriteToCircularBuffer<long double>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, float value)
{
	c.WriteToCircularBuffer<float>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, double value)
{
	c.WriteToCircularBuffer<double>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, char value)
{
	c.WriteToCircularBuffer<char>(value);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, char* values)
{
	c.Write((std::string) values);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, const char* values)
{
	c.Write((std::string) values);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, const void* ptr)
{
	c.WriteToCircularBuffer<const void*>(ptr);

	return c;
}

CircularBuffer& operator << (CircularBuffer& c, std::string str)
{
	c.Write(str);

	return c;
}

