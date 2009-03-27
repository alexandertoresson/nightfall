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
#ifndef LUAWRAPPER_H_PRE
#define LUAWRAPPER_H_PRE

#ifdef DEBUG_DEP
#warning "luawrapper.h-pre"
#endif

extern "C" {
#if defined(__APPLE__) || defined(MACOSX)
	#include <Lua/lua.h>
	#include <Lua/lualib.h>
	#include <Lua/lauxlib.h>
#else
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
#endif
}

//#define LUA_DEBUG
#define LUA_FUNCTION
#define LuaNumResults          int

#define LUA_FUNCTION_FAIL_LIMIT 15

namespace Utilities
{
	namespace Scripting
	{
		class LuaVMState;

		void StartPlayerStates(void);
		void InitAI(void);
	}
}

#endif

