
#ifndef __WINDOW_H_PRE__
#define __WINDOW_H_PRE__

#ifdef DEBUG_DEP
#warning "window.h-pre"
#endif

namespace Window
{
	extern bool initialized;

	extern int windowWidth, windowHeight;
	extern float guiWidth, guiHeight, guiResolution;

	extern bool noWindow;

	extern bool hasVBOs;
}

#endif

