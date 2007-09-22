#include "luawrapper.h"

#include "errors.h"
#include "unit.h"
#include "dimension.h"
#include "aibase.h"
#include "paths.h"
#include <cassert>

namespace Utilities
{
	namespace Scripting
	{
		void SetEnum(LuaVM *L, const char* field, int value)
		{
			lua_pushstring(L, field);
			lua_pushinteger(L, value);
			lua_settable(L, -3);
		}

		void InitializeEnums(LuaVM *L)
		{
			lua_newtable(L);
			SetEnum(L, "None", Game::AI::ACTION_NONE);
			SetEnum(L, "Goto", Game::AI::ACTION_GOTO);
			SetEnum(L, "Attack", Game::AI::ACTION_ATTACK);
			SetEnum(L, "Collect", Game::AI::ACTION_COLLECT);
			SetEnum(L, "Build", Game::AI::ACTION_BUILD);
			SetEnum(L, "Repair", Game::AI::ACTION_REPAIR);
			SetEnum(L, "Follow", Game::AI::ACTION_FOLLOW);
			SetEnum(L, "MoveAttack", Game::AI::ACTION_MOVE_ATTACK);
			SetEnum(L, "MoveAttackUnit", Game::AI::ACTION_MOVE_ATTACK_UNIT);
			SetEnum(L, "Die", Game::AI::ACTION_DIE);
			lua_setglobal(L, "UnitAction");

			lua_newtable(L);
			SetEnum(L, "Human", Game::Dimension::MOVEMENT_HUMAN);
			SetEnum(L, "Vehicle", Game::Dimension::MOVEMENT_VEHICLE);
			SetEnum(L, "Tank", Game::Dimension::MOVEMENT_TANK);
			SetEnum(L, "Building", Game::Dimension::MOVEMENT_BUILDING);
			SetEnum(L, "Airborne", Game::Dimension::MOVEMENT_AIRBORNE);
			SetEnum(L, "Sea", Game::Dimension::MOVEMENT_SEA);
			lua_setglobal(L, "MovementType");

			lua_newtable(L);
			SetEnum(L, "Human", Game::Dimension::PLAYER_TYPE_HUMAN);
			SetEnum(L, "AI", Game::Dimension::PLAYER_TYPE_AI);
			SetEnum(L, "Gaia", Game::Dimension::PLAYER_TYPE_GAIA);
			SetEnum(L, "Remote", Game::Dimension::PLAYER_TYPE_REMOTE);
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
			
			char newline[] = {Console::nl, 0};
			lua_newtable(L);
			lua_pushstring(L, "Newline");
			lua_pushstring(L, newline);
			lua_settable(L, -3);
			lua_setglobal(L, "Console");
		}

		LuaVirtualMachine* LuaVirtualMachine::m_pInstance = NULL;
		LuaVirtualMachine* LuaVirtualMachine::Instance(void)
		{
			if (m_pInstance == NULL)
				m_pInstance = new LuaVirtualMachine;

			return LuaVirtualMachine::m_pInstance;
		}

		void LuaVirtualMachine::Destroy(void)
		{
			if (m_pInstance == NULL)
				return;

			delete m_pInstance;
			m_pInstance = NULL;
		}

		LuaVirtualMachine::LuaVirtualMachine(void)
		{
#ifdef LUA_DEBUG
			std::cout << "[Lua VM] Constructing" << std::endl;
#endif
			m_pVM = lua_open();
			
			if (m_pVM == NULL)
			{
				std::cerr << "[Lua VM] Failed to initialize Lua." << std::endl;
				return;
			}

			luaL_openlibs(m_pVM);
			InitializeEnums(m_pVM);
		}

		LuaVirtualMachine::~LuaVirtualMachine(void)
		{
			if (m_pVM == NULL)
				lua_close(m_pVM);
		}

		void LuaVirtualMachine::RegisterFunction(std::string alias, int (*fptr)(LuaVM*))
		{
			if (m_pVM == NULL)
				return;
#ifdef LUA_DEBUG
			std::cout << "[Lua VM] Registering function " << alias << std::endl;
#endif
			lua_register(m_pVM, alias.c_str(), fptr);
		}

		int LuaVirtualMachine::DoFile(std::string file) const
		{
			if (m_pVM == NULL)
				return 1;
			std::string filepath = Utilities::GetDataFile(file);

			if (!filepath.length())
			{
				std::cout << "Could not locate LUA script " << file << "!" << endl;
			}
		
			int result = luaL_dofile(m_pVM, filepath.c_str());
#ifdef LUA_DEBUG
			std::cout << "Executing " << filepath << ". ";
			if (result)
				std::cout << "Error: " << lua_tostring(m_pVM, -1) << std::endl;
			else
				std::cout << "Success!!" << std::endl;
#endif
			return result;
		}

		void LuaVirtualMachine::SetFunction(std::string luaFunction)
		{
			lua_getglobal(m_pVM,  luaFunction.c_str());
			curFunction = luaFunction;
		}

		int LuaVirtualMachine::CallFunction(unsigned int arguments, unsigned int rets)
		{
			const void *func = lua_topointer(m_pVM, -(1 + arguments));
			if (callErrs[func] < LUA_FUNCTION_FAIL_LIMIT)
			{
				if (lua_pcall(m_pVM, arguments, rets, 0) != 0)
				{
					std::cerr << "[Lua VM] Error on call to " << curFunction << ": " << lua_tostring(m_pVM, -1) << std::endl;
					callErrs[func]++;
					if (callErrs[func] == LUA_FUNCTION_FAIL_LIMIT)
					{
						std::cout << "Lua function " << curFunction << "() exceeded the number of allowed errors; disabling it." << std::endl;
					}
					if (rets)
					{
						lua_pop(m_pVM, rets);
					}
					return ERROR_GENERAL;
				}
			}
			else
			{
				lua_pop(m_pVM, 1 + arguments);
				return ERROR_GENERAL;
			}
			
			bool ret = SUCCESS;

			if (lua_isboolean(m_pVM, 1))
			{
				if (!lua_toboolean(m_pVM, 1))
				{
					ret = ERROR_GENERAL;
				}
			}
			if (rets)
			{
				lua_pop(m_pVM, rets);
			}
			return ret;
		}

		LuaVM* const LuaVirtualMachine::GetVM() const
		{
			return m_pVM;
		}
		
		void StartVM(void)
		{
			LuaVirtualMachine::Instance();
		}

		void StopVM(void)
		{
			LuaVirtualMachine::Destroy();
		}

		LuaVM* GetVM(void)
		{
			return LuaVirtualMachine::Instance()->GetVM();
		}
	}
}
