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
#include "luawrapper.h"

#include "errors.h"
#include "unit.h"
#include "unitsquares.h"
#include "dimension.h"
#include "aibase.h"
#include "vfs.h"
#include "unitinterface.h"
#include "console.h"
#include "game.h"
#include <cassert>
#include <iostream>

using namespace std;

namespace Utilities
{
	namespace Scripting
	{
		gc_root_ptr<LuaVMState>::type globalVMState;

		void SetEnum(lua_State *L, const char* field, int value)
		{
			lua_pushstring(L, field);
			lua_pushinteger(L, value);
			lua_settable(L, -3);
		}

		void InitializeEnums(lua_State *L)
		{
			lua_newtable(L);
			SetEnum(L, "None", Game::AI::ACTION_NONE);
			SetEnum(L, "Goto", Game::AI::ACTION_GOTO);
			SetEnum(L, "Attack", Game::AI::ACTION_ATTACK);
			SetEnum(L, "Collect", Game::AI::ACTION_COLLECT);
			SetEnum(L, "Build", Game::AI::ACTION_BUILD);
			SetEnum(L, "Research", Game::AI::ACTION_RESEARCH);
			SetEnum(L, "Repair", Game::AI::ACTION_REPAIR);
			SetEnum(L, "Follow", Game::AI::ACTION_FOLLOW);
			SetEnum(L, "MoveAttack", Game::AI::ACTION_MOVE_ATTACK);
			SetEnum(L, "MoveAttackUnit", Game::AI::ACTION_MOVE_ATTACK_UNIT);
			SetEnum(L, "Die", Game::AI::ACTION_DIE);
			lua_setglobal(L, "UnitAction");

			lua_newtable(L);
			SetEnum(L, "Human", Game::Dimension::MOVEMENT_HUMAN);
			SetEnum(L, "SmallVehicle", Game::Dimension::MOVEMENT_SMALLVEHICLE);
			SetEnum(L, "MediumVehicle", Game::Dimension::MOVEMENT_MEDIUMVEHICLE);
			SetEnum(L, "LargeVehicle", Game::Dimension::MOVEMENT_LARGEVEHICLE);
			SetEnum(L, "Building", Game::Dimension::MOVEMENT_BUILDING);
			SetEnum(L, "Airborne", Game::Dimension::MOVEMENT_AIRBORNE);
			SetEnum(L, "Sea", Game::Dimension::MOVEMENT_SEA);
			lua_setglobal(L, "MovementType");

			lua_newtable(L);
			SetEnum(L, "Human", Game::Dimension::PLAYER_TYPE_HUMAN);
			SetEnum(L, "AI", Game::Dimension::PLAYER_TYPE_AI);
			lua_setglobal(L, "PlayerType");
			
			lua_newtable(L);
			SetEnum(L, "Sight", Game::Dimension::RANGE_SIGHT);
			SetEnum(L, "Attack", Game::Dimension::RANGE_ATTACK);
			lua_setglobal(L, "RangeType");
			
			lua_newtable(L);
			SetEnum(L, "Ally", Game::Dimension::PLAYER_STATE_ALLY);
			SetEnum(L, "Neutral", Game::Dimension::PLAYER_STATE_NEUTRAL);
			SetEnum(L, "Enemy", Game::Dimension::PLAYER_STATE_ENEMY);
			lua_setglobal(L, "PlayerState");
			
			lua_newtable(L);
			SetEnum(L, "DayLight", Game::Dimension::POWERTYPE_DAYLIGHT);
			SetEnum(L, "TwentyFourSeven", Game::Dimension::POWERTYPE_TWENTYFOURSEVEN);
			lua_setglobal(L, "PowerType");
			
			lua_newtable(L);
			SetEnum(L, "CommandCompleted", UnitLuaInterface::EVENTTYPE_COMMANDCOMPLETED);
			SetEnum(L, "CommandCancelled", UnitLuaInterface::EVENTTYPE_COMMANDCANCELLED);
			SetEnum(L, "NewCommand", UnitLuaInterface::EVENTTYPE_NEWCOMMAND);
			SetEnum(L, "BecomeIdle", UnitLuaInterface::EVENTTYPE_BECOMEIDLE);
			SetEnum(L, "IsAttacked", UnitLuaInterface::EVENTTYPE_ISATTACKED);
			SetEnum(L, "PerformUnitAI", UnitLuaInterface::EVENTTYPE_PERFORMUNITAI);
			SetEnum(L, "UnitCreation", UnitLuaInterface::EVENTTYPE_UNITCREATION);
			SetEnum(L, "PerformPlayerAI", UnitLuaInterface::EVENTTYPE_PERFORMPLAYERAI);
			lua_setglobal(L, "EventType");
			
			lua_newtable(L);
			SetEnum(L, "Unit"    , UnitLuaInterface::AI_CONTEXT_UNIT);
			SetEnum(L, "UnitType", UnitLuaInterface::AI_CONTEXT_UNITTYPE);
			SetEnum(L, "Player"  , UnitLuaInterface::AI_CONTEXT_PLAYER);
			lua_setglobal(L, "AIContext");
			
			char newline[] = {Console::nl, 0};
			lua_newtable(L);
			lua_pushstring(L, "Newline");
			lua_pushstring(L, newline);
			lua_settable(L, -3);
			lua_setglobal(L, "Console");
		}

		LuaVMState::LuaVMState(const gc_ptr<Game::Dimension::Player>& player)
		{
#ifdef LUA_DEBUG
			std::cout << "[Lua VM] Constructing" << std::endl;
#endif
			m_pState = lua_open();

			if (player)
			{
				Game::Dimension::pWorld->luaStateToPlayer[m_pState] = player;
				Game::Dimension::pWorld->luaStateToObject[m_pState] = this;
			}

			global = !player;
			
			if (m_pState == NULL)
			{
				std::cerr << "[Lua VM] Failed to initialize Lua State." << std::endl;
				return;
			}

			luaL_openlibs(m_pState);
			InitializeEnums(m_pState);
			UnitLuaInterface::Init(this);

			CallErrMutex = SDL_CreateMutex();
		}

		LuaVMState::~LuaVMState(void)
		{
			if (m_pState)
			{
				if (!global)
				{
					Game::Dimension::pWorld->luaStateToPlayer.erase(m_pState);
					Game::Dimension::pWorld->luaStateToObject.erase(m_pState);
				}
				lua_close(m_pState);
			}
			SDL_DestroyMutex(CallErrMutex);
		}

		void LuaVMState::RegisterFunction(std::string alias, int (*fptr)(lua_State*))
		{
			if (m_pState == NULL)
				return;
#ifdef LUA_DEBUG
			std::cout << "[Lua VM] Registering function " << alias << std::endl;
#endif
			lua_register(m_pState, alias.c_str(), fptr);
		}

		int LuaVMState::DoFile(std::string file) const
		{
			if (m_pState == NULL)
				return 1;
			std::string filepath = VFS::ResolveReadable(file);

			if (!filepath.length())
			{
				std::cout << "Could not locate LUA script " << file << "!" << endl;
			}
		
			int result = luaL_dofile(m_pState, filepath.c_str());
#ifdef LUA_DEBUG
			std::cout << "Executing " << filepath << ". ";
			if (result)
				std::cout << "Error: " << lua_tostring(m_pState, -1) << std::endl;
			else
				std::cout << "Success!!" << std::endl;
#else
			if (result)
				std::cout << "Error executing " << filepath << ": " << lua_tostring(m_pState, -1) << std::endl;
#endif
			return result;
		}

		void LuaVMState::SetFunction(std::string luaFunction)
		{
			lua_getglobal(m_pState,  luaFunction.c_str());
			curFunction = luaFunction;
		}

		void LuaVMState::SetCurFunction(std::string luaFunction)
		{
			curFunction = luaFunction;
		}

		int LuaVMState::CallFunction(unsigned int arguments, unsigned int rets)
		{
			const void *func = lua_topointer(m_pState, -(1 + arguments));
			SDL_LockMutex(CallErrMutex);
			if (callErrs[func] < LUA_FUNCTION_FAIL_LIMIT)
			{
				SDL_UnlockMutex(CallErrMutex);
				if (lua_pcall(m_pState, arguments, rets, 0) != 0)
				{
					std::cerr << "[Lua VM] Error on call to " << curFunction << ": " << lua_tostring(m_pState, -1) << std::endl;
					SDL_LockMutex(CallErrMutex);
					callErrs[func]++;
					SDL_UnlockMutex(CallErrMutex);
					if (callErrs[func] == LUA_FUNCTION_FAIL_LIMIT)
					{
						std::cout << "Lua function " << curFunction << "() exceeded the number of allowed errors; disabling it." << std::endl;
					}
					if (rets)
					{
						lua_pop(m_pState, rets);
					}
					return ERROR_GENERAL;
				}
			}
			else
			{
				SDL_UnlockMutex(CallErrMutex);
				lua_pop(m_pState, 1 + arguments);
				return ERROR_GENERAL;
			}
			
			bool ret = SUCCESS;

			if (lua_isboolean(m_pState, 1))
			{
				if (!lua_toboolean(m_pState, 1))
				{
					ret = ERROR_GENERAL;
				}
			}
			if (rets)
			{
				lua_pop(m_pState, rets);
			}
			return ret;
		}

		lua_State* LuaVMState::GetState() const
		{
			return m_pState;
		}
		
		void InitGlobalState()
		{
			globalVMState = new LuaVMState(NULL);
		}

		void StartPlayerStates(void)
		{
			int i = 1;
			for (vector<gc_ptr<Game::Dimension::Player> >::iterator it = Game::Dimension::pWorld->vPlayers.begin(); it != Game::Dimension::pWorld->vPlayers.end(); it++)
			{
				const gc_ptr<Game::Dimension::Player>& player = *it;
				if (!player->isRemote)
				{
					std::cout << "Initializing LUA states for player " << i++ << std::endl;
					player->aiState->DoFile("/data/scripts/race/" + player->raceScript + "/" + player->raceScript + ".lua");
					player->aiState->DoFile("/data/scripts/race/" + player->raceScript + "/" + player->aiScript + "/" + player->aiScript + ".lua");
					player->aiState->DoFile(Game::Rules::CurrentLevelScript);
				}
			}
		}

		void InitAI(void)
		{
			for (unsigned i = 0; i < Game::Dimension::pWorld->vPlayers.size(); i++)
			{
				const gc_ptr<Game::Dimension::Player>& player = Game::Dimension::pWorld->vPlayers[i];
				if (!player->isRemote)
				{
					player->aiState->SetFunction("InitRace");
					lua_pushlightuserdata(player->aiState->GetState(), (void*) player->GetHandle());
					player->aiState->CallFunction(1);

					player->aiState->SetFunction("InitAI");
					lua_pushlightuserdata(player->aiState->GetState(), (void*) player->GetHandle());
					player->aiState->CallFunction(1);

					RecheckAllRequirements(player);

				}
			}
			UnitLuaInterface::PostProcessStrings();
		}

		const gc_ptr<Game::Dimension::Player>& GetPlayerByVMstate(lua_State *vmState)
		{
			return Game::Dimension::pWorld->luaStateToPlayer[vmState];
		}

		LuaVMState *GetObjectByVMstate(lua_State *vmState)
		{
			return Game::Dimension::pWorld->luaStateToObject[vmState];
		}

		bool IsGlobalLuaState(lua_State *vmState)
		{
			return vmState == globalVMState->GetState();
		}

	}
}
