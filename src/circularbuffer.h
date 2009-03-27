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
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <string>

class CircularBuffer
{
	private:
		std::string*  m_buffer;

		void IncrementPos(void);

		template <typename T>
		void WriteToCircularBuffer(T value);

		int           start;
		int           end;
		int           lineCount;

		std::string filename;
		void (*remlineCallback)();

	public:
		CircularBuffer(int lines, std::string filename = "");
		~CircularBuffer();
		
		void SetFile(std::string);

		void Write(std::string);
		void WriteLine(std::string);
		int GetLineCount();
		std::string GetLine(int n);

		void SetRemLineCallback(void (*remline_callback)());

		friend CircularBuffer& operator<< (CircularBuffer& c, int);
		friend CircularBuffer& operator<< (CircularBuffer& c, unsigned int);
		friend CircularBuffer& operator<< (CircularBuffer& c, long);
		friend CircularBuffer& operator<< (CircularBuffer& c, unsigned long);
		friend CircularBuffer& operator<< (CircularBuffer& c, short);
		friend CircularBuffer& operator<< (CircularBuffer& c, long double);
		friend CircularBuffer& operator<< (CircularBuffer& c, float);
		friend CircularBuffer& operator<< (CircularBuffer& c, double);
		friend CircularBuffer& operator<< (CircularBuffer& c, char);
		friend CircularBuffer& operator<< (CircularBuffer& c, char*);
		friend CircularBuffer& operator<< (CircularBuffer& c, const char*);
		friend CircularBuffer& operator<< (CircularBuffer& c, const void*);
		friend CircularBuffer& operator<< (CircularBuffer& c, std::string);
		
};

#endif

