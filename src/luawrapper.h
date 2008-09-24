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
#ifndef __LUAWRAPPER_H__
#define __LUAWRAPPER_H__

#ifdef DEBUG_DEP
#warning "luawrapper.h"
#endif

#include "luawrapper-pre.h"

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

#include "dimension-pre.h"
#include "sdlheader.h"
#include <map>
#include <string>

namespace Utilities
{
	namespace Scripting
	{
		class LuaVMState
		{
			private:
				lua_State* m_pState;
				std::map<const void*, int> callErrs;
				std::string curFunction;
				bool global;

			public:

				SDL_mutex *CallErrMutex;

				LuaVMState();
				LuaVMState(const gc_ptr<Game::Dimension::Player>& player);
				~LuaVMState();

				void RegisterFunction(std::string alias, int (*fptr)(lua_State*));

				int DoFile(std::string file) const;
				void SetFunction(std::string);
				void SetCurFunction(std::string);
				int CallFunction(unsigned int arguments, unsigned int rets = 0);
				lua_State* GetState(void) const;

				void shade()
				{
					
				}
		};

		extern gc_root_ptr<LuaVMState>::type globalVMState;
		
		const gc_ptr<Game::Dimension::Player>& GetPlayerByVMstate(lua_State *vmState);
		LuaVMState *GetObjectByVMstate(lua_State *vmState);
		void InitGlobalState();
		bool IsGlobalLuaState(lua_State *vmState);

	}
}

#ifdef DEBUG_DEP
#warning "luawrapper.h-end"
#endif

#endif
