#ifndef __LUAWRAPPER_H_PRE__
#define __LUAWRAPPER_H_PRE__

#ifdef DEBUG_DEP
#warning "luawrapper.h-pre"
#endif

extern "C" {
/*#if defined(__APPLE__) || defined(MACOSX)
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
#else*/
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
//#endif
}

//#define LUA_DEBUG
#define LUA_FUNCTION
#define LuaVM                  lua_State
#define LuaNumResults          int

#define LUA_FUNCTION_FAIL_LIMIT 15

namespace Utilities
{
	namespace Scripting
	{
		class LuaVirtualMachine;

		void StartVM(void);
		void StartPlayerVMs(void);
		void StopVM(void);
		LuaVM* GetVM(void);
		LuaVM* GetPlayerVM(unsigned player);
		LuaVirtualMachine* GetPlayerVMInstance(unsigned player);
	}
}

#define __LUAWRAPPER_H_PRE_END__

#endif

#ifndef __LUAWRAPPER_H__
#define __LUAWRAPPER_H__

#ifdef DEBUG_DEP
#warning "luawrapper.h"
#endif

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
				std::string curFunction;

			public:
				static LuaVirtualMachine* Instance(void);
				static void Destroy(void);

				LuaVirtualMachine(void);
				~LuaVirtualMachine(void);

				void RegisterFunction(std::string alias, int (*fptr)(LuaVM*));

				int DoFile(std::string file) const;
				void SetFunction(std::string);
				int CallFunction(unsigned int arguments, unsigned int rets = 0);
				LuaVM* GetVM(void) const;
		};

	}
}

#ifdef DEBUG_DEP
#warning "luawrapper.h-end"
#endif

#define __LUAWRAPPER_H_END__

#endif

