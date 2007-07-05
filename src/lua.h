#ifndef __LUA_H_PRE__
#define __LUA_H_PRE__

#ifdef DEBUG_DEP
#warning "lua.h-pre"
#endif

#define __LUA_H_PRE_END__

#endif

#ifndef __LUA_H__
#define __LUA_H__

#ifdef DEBUG_DEP
#warning "lua.h"
#endif

extern "C" {
#if defined(__APPLE__) || defined(MACOSX)
	#include <lua/lua.h>
	#include <lua/lualib.h>
	#include <lua/lauxlib.h>
#else
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
#endif
}

#ifndef luaL_dofile
// no luaL_dofile => lua 5.0
#define luaL_dofile(L, str) \
	(luaL_loadfile(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define lua_pushinteger(L, num) \
	lua_pushnumber(L, (lua_Number) num)

#define lua_tointeger(L, i) \
	((int) lua_tonumber(L, i))

#define luaL_openlibs(L) \
	{ \
		lua_pushcfunction(L, luaopen_base); CallFunction(0); \
		lua_pushcfunction(L, luaopen_table);  CallFunction(0); \
		lua_pushcfunction(L, luaopen_io);  CallFunction(0); \
		lua_pushcfunction(L, luaopen_string);  CallFunction(0); \
		lua_pushcfunction(L, luaopen_math);  CallFunction(0); \
		lua_pushcfunction(L, luaopen_debug);  CallFunction(0); \
	}

#endif

#include <iostream>
#include <map>

#define LUA_DEBUG
#define LUA_FUNCTION
#define LuaVM                  lua_State
#define LuaNumResults          int

#define LUA_FUNCTION_FAIL_LIMIT 15

namespace Utilities
{
	namespace Scripting
	{
		class LuaVirtualMachine
		{
			private:
				static LuaVirtualMachine* m_pInstance;
				LuaVM* m_pVM;
				std::map<const void*, int> callErrs;
				const char* curFunction;

			public:
				static LuaVirtualMachine* Instance(void);
				static void Destroy(void);

				LuaVirtualMachine(void);
				~LuaVirtualMachine(void);

				void RegisterFunction(const char* alias, int (*fptr)(LuaVM*));

				int DoFile(const char* file) const;
				void SetFunction(const char*);
				int CallFunction(unsigned int arguments);
				LuaVM* const GetVM(void) const;
		};

		void StartVM(void);
		void StopVM(void);
		LuaVM* GetVM(void);
	}
}

#ifdef DEBUG_DEP
#warning "lua.h-end"
#endif

#define __LUA_H_END__

#endif

