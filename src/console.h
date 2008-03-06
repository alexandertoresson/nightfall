#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "circularbuffer.h"

#define CONSOLE_COLOR_IMPORTANT		255, 255, 0
#define CONSOLE_COLOR_ERROR			255, 0, 0

extern CircularBuffer console;

namespace Console
{

	const char nl = '\n';
	const char err = '@';
	const char imp = '!';
}

#endif

