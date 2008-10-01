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
//#include "memfragdebug.h" // Enable for memory fragmentation debugging

#define NO_LOCKCHECKER

#ifdef HAVE_CONFIG_H
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

#include "i18n.h"

#include "gc_ptr.h"

#ifndef GL_ARB_multitexture
#error GL_ARB_multitexture is needed for this program
#endif

#ifndef GL_ARB_texture_env_combine
#error GL_ARB_texture_env_combine is needed for this program
#endif

#ifndef GL_ARB_texture_env_crossbar
#error GL_ARB_texture_env_crossbar is needed for this program
#endif

#ifndef NO_LOCKCHECKER
#include "lockchecker.h" // Enable for mutex look debugging and statistics
#endif

