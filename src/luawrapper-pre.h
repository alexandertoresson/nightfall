#ifndef __LUAWRAPPER_H_PRE__
#define __LUAWRAPPER_H_PRE__

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

