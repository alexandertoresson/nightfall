#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__

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

