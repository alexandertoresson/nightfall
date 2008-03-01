//ß#include "memfragdebug.h" // Enable for memory fragmentation debugging

#ifdef __CONFIG_H__
#include "config.h"
#endif

#ifdef WIN32
	#pragma warning(disable:4250)
	#pragma warning(disable:4244) // << note!! Could be useful in some situations!
	#define _CRT_SECURE_NO_DEPRECATE
	#include <windows.h>
#endif

#if defined(__APPLE__) || defined(MACOSX)
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
	#include <SDL_mixer/SDL_mixer.h>
	#include <SDL_image/SDL_image.h>
	#include <SDL_ttf/SDL_ttf.h>
	#include <SDL_net/SDL_net.h>
	#include <glew.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
	#include <Carbon/Carbon.h>
	
	/*
	#define MAC
	#undef inline
	#define inline
	*/
	
	#define SDL_EVENT_DUMP_CONSOLE 31
	#define MAC
#else

	#include <SDL.h>
	#include <SDL_thread.h>
	#include <SDL_mixer.h>
	#include <SDL_image.h>
	#include <SDL_ttf.h>
	#include <SDL_net.h>

	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#ifndef GL_ARB_multitexture
#error GL_ARB_multitexture is needed for this program
#endif

#ifndef GL_ARB_texture_env_combine
#error GL_ARB_texture_env_combine is needed for this program
#endif

#ifndef GL_ARB_texture_env_crossbar
#error GL_ARB_texture_env_crossbar is needed for this program
#endif


#ifndef RTS_ZeroMemory
#define RTS_ZeroMemory(x) \
	memset(x, 0, sizeof(x))
#endif

#include <iostream>

