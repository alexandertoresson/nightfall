#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#define CONSOLE_COLOR_IMPORTANT		255, 255, 0
#define CONSOLE_COLOR_ERROR			255, 0, 0

class Console
{
	private:
		std::string*  m_buffer;
		bool dump;

		inline void IncrementPos(void);
		inline void CheckTranslation(void);

	public:
		float         lineHeight;
		int           lineCount;
		int           translate;
		int           start;
		int           end;
		int           visibleLines;

		static const char nl = '\n';
		static const char err = '@';
		static const char imp = '!';

	public:
		Console();
		~Console();
		const std::string* GetBuffer() const;

		void SetDumpFlag(bool flag);
		void SetLineHeight(float, int);
		void PutChar(char);
		void Write(std::string);
		void WriteLine(std::string);

		friend Console& operator<< (Console& c, int);
		friend Console& operator<< (Console& c, unsigned int);
		friend Console& operator<< (Console& c, long);
		friend Console& operator<< (Console& c, unsigned long);
		friend Console& operator<< (Console& c, short);
		friend Console& operator<< (Console& c, long double);
		friend Console& operator<< (Console& c, float);
		friend Console& operator<< (Console& c, double);
		friend Console& operator<< (Console& c, char);
		friend Console& operator<< (Console& c, char*);
		friend Console& operator<< (Console& c, const void*);
		friend Console& operator<< (Console& c, std::string);
		
/*#ifdef _GLIBCXX_USE_LONG_LONG
		friend Console& operator<< (Console& c, unsigned long long);
		friend Console& operator<< (Console& c, long long);
#endif*/
};

extern Console console;

#endif

