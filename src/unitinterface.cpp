#include "unitinterface.h"

#include "terrain.h"
#include "unit.h"
#include "unitsquares.h"
#include "unitrender.h"
#include "aipathfinding.h"
#include "environment.h"
#include "networking.h"
#include "paths.h"
#include "dimension.h"
#include "aibase.h"
#include "camera.h"
#include "gamegui.h"
#include "aibase.h"
#include "luawrapper.h"
#include "gui.h"
#include "game.h"
#include <sstream>

namespace UnitLuaInterface
{
	using namespace Game::Dimension;
	using namespace Game::AI;
	using namespace Utilities;
	using namespace Utilities::Scripting;
	using namespace std;

#define CHECK_UNIT_PTR(x) \
	if ((x) == NULL) \
	{ \
		lua_pushboolean(pVM, 0); \
		std::cout << "[LUA SET] Failure:" << __FUNCTION__ << ": " << #x << " Invalid unit - null pointer" << std::endl; \
		return 1; \
	}  \
	if (!IsDisplayedUnitPointer(x)) \
	{ \
		lua_pushboolean(pVM, 0); \
		std::cout << "[LUA SET] Failure:" << __FUNCTION__ << ": " << #x << " Invalid unit - not in list of displayed ones" << std::endl; \
		return 1; \
	} 

#define CHECK_UNIT_PTR_NULL_VALID(x) \
	if ((x) != NULL) \
	{ \
		if (!IsDisplayedUnitPointer(x)) \
		{ \
			lua_pushboolean(pVM, 0); \
			std::cout << "[LUA SET] Failure:" << __FUNCTION__ << ": " << #x << " Invalid unit - not in list of displayed ones" << std::endl; \
			return 1; \
		} \
	} 

	Unit* _GetUnit(void* ptr)
	{
		Unit* unit = GetUnitByID((unsigned long)ptr);
/*		if (unit == NULL || !IsDisplayedUnitPointer(unit))
		{
			std::cout << "[LUA SET] Failure: " << (int) ptr << " is not a valid unit id " << std::endl;
		}*/
		return unit;
	}

	int LGetUnitHealth(lua_State* pVM)
	{
		float health = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit);

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			health = pUnit->health;

		lua_pushnumber(pVM, health);
		return 1;
	}

	int LGetUnitMaxHealth(lua_State* pVM)
	{
		int health = -1;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			health = pUnit->type->maxHealth;

		lua_pushnumber(pVM, health);
		return 1;
	}

	int LGetUnitPower(lua_State* pVM)
	{
		float power = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			power = pUnit->power;

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetUnitMaxPower(lua_State* pVM)
	{
		int power = -1;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			power = pUnit->type->maxPower;

		lua_pushinteger(pVM, power);
		return 1;
	}

	int LGetUnitAction(lua_State* pVM)
	{
		UnitAction action = ACTION_NONE;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			action = pUnit->pMovementData->action.action;

		lua_pushinteger(pVM, action);
		return 1;
	}

	int LGetUnitActionArg(lua_State* pVM)
	{
		void* arg = NULL;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			arg = pUnit->pMovementData->action.arg;

		lua_pushlightuserdata(pVM, arg);
		return 1;
	}

	int LGetUnitPosition(lua_State* pVM)
	{
		float position[2] = { 0, 0 };
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
		{
			position[0] = pUnit->pos.x;
			position[1] = pUnit->pos.y;
		}

		lua_pushnumber(pVM, position[0]);
		lua_pushnumber(pVM, position[1]);
		return 2;
	}

	int LGetUnitRotation(lua_State* pVM)
	{
		float rotation = 0;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			rotation = pUnit->rotation;

		lua_pushnumber(pVM, rotation);
		return 1;
	}

	int LGetUnitType(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		UnitType* pUType = NULL;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
		{
			pUType = pUnit->type;
		}

		lua_pushlightuserdata(pVM, static_cast<void*>(pUType));
		return 1;
	}

	int LGetUnitOwner(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Player* pPlayer = NULL;
		int result = 0;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
		{
			result = 1;
			pPlayer = pUnit->owner;
		}

//		lua_pushnumber(pVM, result);
		lua_pushlightuserdata(pVM, (void*) pPlayer);
		return 1;
	}

	int LGetUnitIsMobile(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool isMobile = false;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			isMobile = pUnit->type->isMobile;

		lua_pushboolean(pVM, isMobile);
		return 1;
	}

	int LGetUnitTypeIsMobile(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		bool isMobile = false;

		if (pUnitType != NULL)
			isMobile = pUnitType->isMobile;

		lua_pushboolean(pVM, isMobile);
		return 1;
	}

	int LGetUnitLightAmount(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		float lightAmount = 0.0;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			lightAmount = GetLightAmountOnUnit(pUnit);

		lua_pushnumber(pVM, lightAmount);
		return 1;
	}

	int LGetUnitCanAttack(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool canAttack = false;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			canAttack = pUnit->type->canAttack;

		lua_pushboolean(pVM, canAttack);
		return 1;
	}

	int LGetUnitCanBuild(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool canBuild = false;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			canBuild = pUnit->type->canBuild.size() > 0;

		lua_pushboolean(pVM, canBuild);
		return 1;
	}

	int LGetUnitLastAttack(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Uint32 time = 0;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			time = pUnit->lastAttack;
		
		lua_pushnumber(pVM, time);
		return 1;
	}

	int LGetUnitLastAttacked(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Uint32 time = 0;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			time = pUnit->lastAttacked;
		
		lua_pushnumber(pVM, time);
		return 1;
	}

	int LGetUnitTargetUnit(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Unit* target = NULL;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			if (pUnit->pMovementData != NULL)
				target = pUnit->pMovementData->action.goal.unit;
		
		lua_pushlightuserdata(pVM, (void*) target);
		return 1;
	}

	int LGetUnitTargetPos(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		IntPosition target;

		target.x = 0;
		target.y = 0;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
			if (pUnit->pMovementData != NULL)
				target = pUnit->pMovementData->action.goal.pos;
		
		lua_pushnumber(pVM, target.x);
		lua_pushnumber(pVM, target.y);
		return 2;
	}

	int LGetNearestUnitInRange(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		RangeType rangeType = (RangeType) lua_tointeger(pVM, 2);
		PlayerState PSBitmask = lua_tointeger(pVM, 3);

		Unit* retUnit = NULL;

		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit))
		{
			retUnit = GetNearestUnitInRange(pUnit, rangeType, PSBitmask);
		}
		
		if (retUnit)
		{
			lua_pushlightuserdata(pVM, (void*) retUnit->id);
		}
		else
		{
			lua_pushlightuserdata(pVM, (void*) 0);
		}
		return 1;
	}

	int LGetNearestSuitableAndLightedPosition(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		int x = lua_tointeger(pVM, 2), y = lua_tointeger(pVM, 3);
		bool ret = false;

		if (pUnitType != NULL)
		{
			ret = GetNearestSuitableAndLightedPosition(pUnitType, x, y);
		}
		
		lua_pushinteger(pVM, x);
		lua_pushinteger(pVM, y);
		lua_pushboolean(pVM, ret);
		return 3;
	}

	int LGetSuitablePositionForLightTower(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		int x = lua_tointeger(pVM, 2), y = lua_tointeger(pVM, 3);
		bool needLighted = lua_toboolean(pVM, 4);
		bool ret = false;

		if (pUnitType != NULL)
		{
			ret = GetSuitablePositionForLightTower(pUnitType, x, y, needLighted);
		}
		
		lua_pushinteger(pVM, x);
		lua_pushinteger(pVM, y);
		lua_pushboolean(pVM, ret);
		return 3;
	}

	int LGetCurrentFrame(lua_State* pVM)
	{
		lua_pushinteger(pVM, Game::AI::currentFrame);
		return 1;
	}

	int LGetAIFPS(lua_State* pVM)
	{
		lua_pushinteger(pVM, Game::AI::aiFps);
		return 1;
	}

	int LGetTime(lua_State* pVM)
	{
		Environment::FourthDimension* pDimension = Environment::FourthDimension::Instance();
		lua_pushnumber(pVM, pDimension->GetCurrentHour());
		return 1;
	}

	int LGetPower(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float power = 0;

		if (player)
			power = (float) GetPower(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetMoney(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float money = 0;

		if (player)
			money = (float) GetMoney(player);

		lua_pushnumber(pVM, money);
		return 1;
	}

	int LGetIncomeAtNoon(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float income = 0;

		if (player)
			income = (float) GetIncomeAtNoon(player);

		lua_pushnumber(pVM, income);
		return 1;
	}
	
	int LGetIncomeAtNight(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float income = 0;

		if (player)
			income = (float) GetIncomeAtNight(player);

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetUnitTypeIncomeAtNoon(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float income;

		income = 0;

		if (pUnitType != NULL)
		{
			
			income += (float) pUnitType->powerIncrement;
			income -= float(pUnitType->powerUsage + pUnitType->lightPowerUsage + (pUnitType->attackPowerUsage + pUnitType->movePowerUsage + pUnitType->buildPowerUsage) * 0.1);
		}

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetUnitTypeIncomeAtNight(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float income;

		income = 0;

		if (pUnitType != NULL)
		{
			
			if (pUnitType->powerType == POWERTYPE_TWENTYFOURSEVEN)
			{
				income += (float) pUnitType->powerIncrement;
			}
			income -= float(pUnitType->powerUsage + pUnitType->lightPowerUsage + (pUnitType->attackPowerUsage + pUnitType->movePowerUsage + pUnitType->buildPowerUsage) * 0.1);
		}

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetPowerAtDawn(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float power = 0;

		if (player)
			power = (float) GetPowerAtDawn(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetPowerAtDusk(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		float power = 0;

		if (player)
			power = (float) GetPowerAtDusk(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LSellPower(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		int power = lua_tointeger(pVM, 2);

		if (player)
		{
			if (Game::Networking::isNetworked)
			{
				Game::Networking::PrepareSell(player, power);
			}
			else
			{
				SellPower(player, power);
			}
		}

		return 0;
	}

	int LGetDayLength(lua_State* pVM)
	{
		lua_pushnumber(pVM, GetDayLength());
		return 1;
	}

	int LGetNightLength(lua_State* pVM)
	{
		lua_pushnumber(pVM, GetNightLength());
		return 1;
	}

	int LGetMapDimensions(lua_State* pVM)
	{
		lua_pushinteger(pVM, pWorld->width);
		lua_pushinteger(pVM, pWorld->height);
		return 2;
	}

#define LUA_FAILURE(x) \
	{ \
		lua_pushboolean(pVM, 0); \
		std::cout << "[LUA] Failure: " << __FUNCTION__ << ": " <<  x << std::endl; \
		return 1; \
	}

#define LUA_SUCCESS \
	{ \
		lua_pushboolean(pVM, 1); \
		return 1; \
	}

#define LUA_FAIL \
	{ \
		lua_pushboolean(pVM, 0); \
		return 1; \
	}

	int LSetUnitHealth(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float health = (float) lua_tonumber(pVM, 2);
		if (health < 0)
			LUA_FAILURE("Health value lower than zero.")

		if (health > pUnit->type->maxHealth)
			health = (float) pUnit->type->maxHealth;

		pUnit->health = health;

		LUA_SUCCESS
	}

	int LSetUnitPower(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float power = (float) lua_tonumber(pVM, 2);
		if (power < 0)
			LUA_FAILURE("Power value lower than zero")

		if (power > pUnit->type->maxPower)
			power = (float) pUnit->type->maxPower;

		pUnit->power = power;

		LUA_SUCCESS
	}

	int LSetUnitRotation(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float rotation = (float) lua_tonumber(pVM, 2);

		while (rotation >= 360.0f)
			rotation -= 360.0f;
		
		while (rotation < 0.0f)
			rotation += 360.0f;
		
		pUnit->rotation = rotation;

		LUA_SUCCESS
	}

	int LGetUnitTypeBuildCost(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		int cost = 0;

		if (pUnitType)
			cost = pUnitType->requirements.money;

		lua_pushinteger(pVM, cost);
		return 1;
	}
	
	int LIsResearched(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		if (pUnitType == NULL)
		{
			lua_pushboolean(pVM, false);
			return 1;
		}
		lua_pushboolean(pVM, pUnitType->requirements.creation.isSatisfied && pUnitType->requirements.existance.isSatisfied);
		lua_pushlightuserdata(pVM, player);
		return 1;
	}
	
	int LGetResearcher(lua_State* pVM)
	{
/*		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player *player = GetPlayerByVMstate(pVM);
		for (vector<UnitType*>::iterator it = player->vUnitTypes.begin(); it != player->vUnitTypes.end(); it++)
		{
			for (vector<UnitType*>::iterator it2 = (*it)->canResearch.begin(); it2 != (*it)->canResearch.end(); it2++)
			{
				if (*it2 == pUnitType)
				{
					lua_pushlightuserdata(pVM, static_cast<void*>(*it));
					return 1;
				}
			}
		}*/
		lua_pushlightuserdata(pVM, NULL);
		return 1;
	}
	
	int LGetBuilder(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player *player = GetPlayerByVMstate(pVM);
		for (vector<UnitType*>::iterator it = player->vUnitTypes.begin(); it != player->vUnitTypes.end(); it++)
		{
			for (vector<UnitType*>::iterator it2 = (*it)->canBuild.begin(); it2 != (*it)->canBuild.end(); it2++)
			{
				if (*it2 == pUnitType)
				{
					lua_pushlightuserdata(pVM, static_cast<void*>(*it));
					return 1;
				}
			}
		}
		lua_pushlightuserdata(pVM, NULL);
		return 1;
	}
	
	int LSquaresAreLightedAround(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		int lighted = false;
		if (pUnitType)
		{
			lighted = SquaresAreLightedAround(pUnitType, lua_tointeger(pVM, 2), lua_tointeger(pVM, 3));
		}
		lua_pushboolean(pVM, lighted);
		return 1;
	}
	
	int LSquaresAreLighted(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		int lighted = false;
		if (pUnitType)
		{
			lighted = SquaresAreLighted(pUnitType, lua_tointeger(pVM, 2), lua_tointeger(pVM, 3));
		}
		lua_pushboolean(pVM, lighted);
		return 1;
	}
	
	int LIsWithinRangeForBuilding(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		lua_pushboolean(pVM, IsWithinRangeForBuilding(pUnit));
		return 1;
	}

	struct ScheduledDamaging
	{
		Unit* unit;
		float damage;
	};

	vector<ScheduledDamaging*> scheduledDamagings;
	SDL_mutex* scheduledDamagingsMutex = SDL_CreateMutex();

	void ApplyScheduledDamagings()
	{
		for (vector<ScheduledDamaging*>::iterator it = scheduledDamagings.begin(); it != scheduledDamagings.end(); it++)
		{
			ScheduledDamaging *damaging = *it;
			if (IsValidUnitPointer(damaging->unit))
			{
				Attack(damaging->unit, damaging->damage);
			}
			delete damaging;
		}
		scheduledDamagings.clear();
	}

	int LAttack(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareDamaging(pUnit, (float) lua_tonumber(pVM, 2));
		}
		else
		{
			ScheduledDamaging *damaging = new ScheduledDamaging;
			damaging->unit = pUnit;
			damaging->damage = (float) lua_tonumber(pVM, 2);
			SDL_LockMutex(scheduledDamagingsMutex);
			scheduledDamagings.push_back(damaging);
			SDL_UnlockMutex(scheduledDamagingsMutex);
		}
		return 0;
	}

	int LCanReach(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		lua_pushboolean(pVM, CanReach(pUnit01, pUnit02));
		return 1;
	}

	struct ScheduledAction
	{
		Unit *unit;
		int x;
		int y;
		Unit *target;
		Game::AI::UnitAction action;
		void* arg;
	};

	vector<ScheduledAction*> scheduledActions;
	SDL_mutex* scheduledActionsMutex = SDL_CreateMutex();

	void ApplyScheduledActions()
	{
		for (vector<ScheduledAction*>::iterator it = scheduledActions.begin(); it != scheduledActions.end(); it++)
		{
			ScheduledAction *action = *it;
			if (IsValidUnitPointer(action->unit))
			{
				if (action->target && !IsValidUnitPointer(action->target))
				{
					action->action = ACTION_GOTO;
					action->target = NULL;
				}

				ApplyAction(action->unit, action->action, action->x, action->y, action->target, action->arg);
				Game::Dimension::ChangePath(action->unit, action->x, action->y, action->action, action->target, action->arg);
			}
			delete action;
		}
		scheduledActions.clear();
	}

	void CommandUnit_TargetUnit(Unit* unit, Unit* target, UnitAction action, void* arg)
	{
		if (action == ACTION_NONE)
			return;
		
		Game::AI::action_changes++;
		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareAction(unit, target, target->curAssociatedSquare.x, target->curAssociatedSquare.y, action, arg);
		}
		else
		{
			ScheduledAction *sAction = new ScheduledAction;
			sAction->unit = unit;
			sAction->action = action;
			sAction->x = target->curAssociatedSquare.x;
			sAction->y = target->curAssociatedSquare.y;
			sAction->target = target;
			sAction->arg = arg;

			SDL_LockMutex(scheduledActionsMutex);
			scheduledActions.push_back(sAction);
			SDL_UnlockMutex(scheduledActionsMutex);
		}
	}

	void CommandUnit_TargetPos(Unit* unit, int x, int y, UnitAction action, void* arg)
	{
		if (!unit->type->isMobile && action == ACTION_GOTO)
			return;

		if (action == ACTION_NONE)
			return;
		
		Game::AI::action_changes++;
		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareAction(unit, NULL, x, y, action, arg);
		}
		else
		{
			ScheduledAction *sAction = new ScheduledAction;
			sAction->unit = unit;
			sAction->action = action;
			sAction->x = x;
			sAction->y = y;
			sAction->target = NULL;
			sAction->arg = arg;

			SDL_LockMutex(scheduledActionsMutex);
			scheduledActions.push_back(sAction);
			SDL_UnlockMutex(scheduledActionsMutex);
		}
	}

	int LCommandUnit_TargetUnit(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, (UnitAction) lua_tointeger(pVM, 3), lua_touserdata(pVM, 4));

		LUA_SUCCESS
	}

	int LCommandUnit_TargetPos(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		CommandUnit_TargetPos(pUnit, lua_tointeger(pVM, 2), lua_tointeger(pVM, 3), (UnitAction) lua_tointeger(pVM, 4), lua_touserdata(pVM, 5));

		LUA_SUCCESS
	}

	int LCommandGoto(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		int position[2] = { lua_tointeger(pVM, 2), 
		                    lua_tointeger(pVM, 3) };

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_GOTO, NULL);
		LUA_SUCCESS
	}

	int LCommandMoveAttack(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		int position[2] = { lua_tointeger(pVM, 2), 
		                    lua_tointeger(pVM, 3) };

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_MOVE_ATTACK, NULL);
		LUA_SUCCESS
	}

	int LCommandBuild(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		int position[2] = { lua_tointeger(pVM, 2), 
		                    lua_tointeger(pVM, 3) };

		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 4);

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_BUILD, pUnitType);
		LUA_SUCCESS
	}

	int LCommandResearch(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 2);

		CommandUnit_TargetPos(pUnit, 0, 0, ACTION_RESEARCH, pUnitType);
		LUA_SUCCESS
	}

	int LCommandFollow(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_FOLLOW, NULL);
		LUA_SUCCESS
	}

	int LCommandAttack(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));
		
		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_ATTACK, NULL);
		LUA_SUCCESS
	}

	int LCommandCollect(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_COLLECT, NULL);
		LUA_SUCCESS
	}

	int LCommandRepair(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_BUILD, NULL);
		LUA_SUCCESS
	}

	int LCommandMoveAttackUnit(lua_State* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_MOVE_ATTACK_UNIT, NULL);
		LUA_SUCCESS
	}

	int LMove(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		MoveUnit(pUnit);
		LUA_SUCCESS
	}

	int LSetEventHandler(lua_State* pVM)
	{
		void* context = lua_touserdata(pVM, 1);
		EventType eventtype = (EventType) lua_tointeger(pVM, 2);
		const char* handler = lua_tostring(pVM, 3);
		Player *player = GetPlayerByVMstate(pVM);

		if (!handler)
		{
			LUA_FAILURE("Null pointer handler string received")
		}

		std::string handlerString = (std::string) handler;

		if (!context)
		{
			LUA_FAILURE("Null pointer context received")
		}

		if (IsValidUnitTypePointer((UnitType*)context))
		{
			UnitType* unittype = (UnitType*) context;
			if (IsValidPlayerPointer(player))
			{
				switch (eventtype)
				{
					case EVENTTYPE_COMMANDCOMPLETED:
						unittype->unitAIFuncs.commandCompleted.func = handlerString;
						break;
					case EVENTTYPE_COMMANDCANCELLED:
						unittype->unitAIFuncs.commandCancelled.func = handlerString;
						break;
					case EVENTTYPE_NEWCOMMAND:
						unittype->unitAIFuncs.newCommand.func = handlerString;
						break;
					case EVENTTYPE_BECOMEIDLE:
						unittype->unitAIFuncs.becomeIdle.func = handlerString;
						break;
					case EVENTTYPE_ISATTACKED:
						unittype->unitAIFuncs.isAttacked.func = handlerString;
						break;
					case EVENTTYPE_UNITKILLED:
						unittype->unitAIFuncs.unitKilled.func = handlerString;
						break;
					case EVENTTYPE_PERFORMUNITAI:
						unittype->unitAIFuncs.performUnitAI.func = handlerString;
						break;
					case EVENTTYPE_UNITCREATION:
						unittype->playerAIFuncs.unitCreation.func = handlerString;
						break;
					default:
						LUA_FAILURE("Event type not valid for unit type")
				}
			}
			else
			{
				LUA_FAILURE("Invalid player pointer received")
			}
		}
		else if (IsDisplayedUnitPointer(_GetUnit(context)))
		{
			Unit* unit = _GetUnit(context);
			switch (eventtype)
			{
				case EVENTTYPE_COMMANDCOMPLETED:
					unit->unitAIFuncs.commandCompleted.func = handlerString;
					break;
				case EVENTTYPE_COMMANDCANCELLED:
					unit->unitAIFuncs.commandCancelled.func = handlerString;
					break;
				case EVENTTYPE_NEWCOMMAND:
					unit->unitAIFuncs.newCommand.func = handlerString;
					break;
				case EVENTTYPE_BECOMEIDLE:
					unit->unitAIFuncs.becomeIdle.func = handlerString;
					break;
				case EVENTTYPE_ISATTACKED:
					unit->unitAIFuncs.isAttacked.func = handlerString;
					break;
				case EVENTTYPE_UNITKILLED:
					unit->unitAIFuncs.unitKilled.func = handlerString;
					break;
				case EVENTTYPE_PERFORMUNITAI:
					unit->unitAIFuncs.performUnitAI.func = handlerString;
					break;
				default:
					LUA_FAILURE("Event type not valid for unit")
			}
		}
		else if (IsValidPlayerPointer(player))
		{
			switch (eventtype)
			{
				case EVENTTYPE_COMMANDCOMPLETED:
					player->unitAIFuncs.commandCompleted.func = handlerString;
					break;
				case EVENTTYPE_COMMANDCANCELLED:
					player->unitAIFuncs.commandCancelled.func = handlerString;
					break;
				case EVENTTYPE_NEWCOMMAND:
					player->unitAIFuncs.newCommand.func = handlerString;
					break;
				case EVENTTYPE_BECOMEIDLE:
					player->unitAIFuncs.becomeIdle.func = handlerString;
					break;
				case EVENTTYPE_ISATTACKED:
					player->unitAIFuncs.isAttacked.func = handlerString;
					break;
				case EVENTTYPE_UNITKILLED:
					player->unitAIFuncs.unitKilled.func = handlerString;
					break;
				case EVENTTYPE_PERFORMUNITAI:
					player->unitAIFuncs.performUnitAI.func = handlerString;
					break;
				case EVENTTYPE_UNITCREATION:
					player->playerAIFuncs.unitCreation.func = handlerString;
					break;
				case EVENTTYPE_PERFORMPLAYERAI:
					player->playerAIFuncs.performPlayerAI.func = handlerString;
					break;
				default:
					LUA_FAILURE("Event type not valid for player")
			}
		}
		else
		{
			LUA_FAILURE("Invalid pointer received")
		}

		LUA_SUCCESS
	}

	int LSetRegularAIDelay(lua_State* pVM)
	{
		void* context = lua_touserdata(pVM, 1);
		EventType eventtype = (EventType) lua_tointeger(pVM, 2);
		int delay = lua_tointeger(pVM, 3);
		Player *player = GetPlayerByVMstate(pVM);

		if (!context)
		{
			LUA_FAILURE("Null pointer context received")
		}
		
		if (IsValidUnitTypePointer((UnitType*)context))
		{
			UnitType* unittype = (UnitType*) context;
			if (IsValidPlayerPointer(player))
			{
				if (eventtype == EVENTTYPE_PERFORMUNITAI)
				{
					unittype->unitAIFuncs.performUnitAI.delay = delay;
				}
				else
				{
					LUA_FAILURE("Event type not valid for unit type")
				}
			}
			else
			{
				LUA_FAILURE("Invalid player pointer received")
			}
		}
		else if (IsDisplayedUnitPointer(_GetUnit(context)))
		{
			Unit* unit = _GetUnit(context);
			if (eventtype == EVENTTYPE_PERFORMUNITAI)
			{
				unit->unitAIFuncs.performUnitAI.delay = delay;
			}
			else
			{
				LUA_FAILURE("Event type not valid for unit")
			}
		}
		else if (IsValidPlayerPointer(player))
		{
			if (eventtype == EVENTTYPE_PERFORMUNITAI)
			{
				player->unitAIFuncs.performUnitAI.delay = delay;
			}
			else if (eventtype == EVENTTYPE_PERFORMPLAYERAI)
			{
				player->playerAIFuncs.performPlayerAI.delay = delay;
			}
			else
			{
				LUA_FAILURE("Event type not valid for player")
			}
		}
		else
		{
			LUA_FAILURE("Invalid pointer received")
		}

		LUA_SUCCESS
	}

	int LSetRegularAIEnabled(lua_State* pVM)
	{
		void* context = lua_touserdata(pVM, 1);
		EventType eventtype = (EventType) lua_tointeger(pVM, 2);
		bool enabled = lua_toboolean(pVM, 3);
		Player *player = GetPlayerByVMstate(pVM);

		if (!context)
		{
			LUA_FAILURE("Null pointer context received")
		}
		
		if (IsValidUnitTypePointer((UnitType*)context))
		{
			UnitType* unittype = (UnitType*) context;
			if (IsValidPlayerPointer(player))
			{
				if (eventtype == EVENTTYPE_PERFORMUNITAI)
				{
					unittype->unitAIFuncs.performUnitAI.enabled = enabled;
				}
				else
				{
					LUA_FAILURE("Event type not valid for unit type")
				}
			}
			else
			{
				LUA_FAILURE("Invalid player pointer received")
			}
		}
		else if (IsDisplayedUnitPointer(_GetUnit(context)))
		{
			Unit* unit = _GetUnit(context);
			if (eventtype == EVENTTYPE_PERFORMUNITAI)
			{
				unit->unitAIFuncs.performUnitAI.enabled = enabled;
			}
			else
			{
				LUA_FAILURE("Event type not valid for unit")
			}
		}
		else if (IsValidPlayerPointer(player))
		{
			if (eventtype == EVENTTYPE_PERFORMUNITAI)
			{
				player->unitAIFuncs.performUnitAI.enabled = enabled;
			}
			else if (eventtype == EVENTTYPE_PERFORMPLAYERAI)
			{
				player->playerAIFuncs.performPlayerAI.enabled = enabled;
			}
			else
			{
				LUA_FAILURE("Event type not valid for player")
			}
		}
		else
		{
			LUA_FAILURE("Invalid pointer received")
		}

		LUA_SUCCESS
	}

	int LClearAllActions(lua_State* pVM)
	{
		LUA_FAILURE("Not yet implemented")
	}

	int LCreateUnit(lua_State* pVM)
	{
		UnitType* pUnitType = static_cast<UnitType*>(lua_touserdata(pVM, 1));
		if (pUnitType == NULL)
			LUA_FAILURE("Invalid unit type - null pointer")

		Player* pOwner = GetPlayerByVMstate(pVM);
		if (pOwner == NULL)
		{
			if (IsGlobalLuaState(pVM))
			{
				pOwner = (Player*) lua_touserdata(pVM, 5);
			}
			else
			{
				LUA_FAILURE("Could not determine originating player and is not global lua state")
			}
		}

		int position[2] = { lua_tointeger(pVM, 2), lua_tointeger(pVM, 3) };

		if (SquaresAreWalkable(pUnitType, position[0], position[1], SIW_ALLKNOWING) == false)
		{
			LUA_FAILURE("Designated goal isn't walkable (or is preoccupied by another unit).")
		}

		int rotation = (int) (rand()/((double)RAND_MAX + 1) * 360);

		if (lua_isnumber(pVM, 4))
		{
			rotation = lua_tointeger(pVM, 4);

			while (rotation >= 360)
				rotation -= 360;

			while (rotation < 0)
				rotation += 360;

		}

		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareCreation(pUnitType, position[0], position[1], rotation);
		}
		else
		{
			Unit* pUnit = CreateUnit(pUnitType, position[0], position[1]);
			if (pUnit)
				pUnit->rotation = (float) rotation;
		}

		return 0;
	}

	int LCanCreateUnitAt(lua_State* pVM)
	{
		UnitType* pUnitType = static_cast<UnitType*>(lua_touserdata(pVM, 1));
		if (pUnitType == NULL)
			LUA_FAIL

		Player* pOwner = GetPlayerByVMstate(pVM);
		if (pOwner == NULL)
			LUA_FAIL

		float position[2] = { (float) lua_tonumber(pVM, 2), (float) lua_tonumber(pVM, 3) };
		if (position[0] < 0 || position[1] < 0)
			LUA_FAIL

		if (SquaresAreWalkable(pUnitType, (int)position[0], (int)position[1], SIW_ALLKNOWING) == false)
			LUA_FAIL

		LUA_SUCCESS
	}

	int LGetUnitTypeFromString(lua_State* pVM)
	{
		Player *player = GetPlayerByVMstate(pVM);
		const char *sz_name = lua_tostring(pVM, 1);
		const string name = sz_name;
		if (!player)
		{
			player = (Player*) lua_touserdata(pVM, 2);
		}
		lua_pushlightuserdata(pVM, player->unitTypeMap[name]);
		return 1;
	}

	int LGetPlayerByIndex(lua_State* pVM)
	{
		const Uint32 index = static_cast<const Uint32>(lua_tonumber(pVM, 1));

		if (index >= pWorld->vPlayers.size())
			LUA_FAILURE("Player index out of vector bounds")

		lua_pushlightuserdata(pVM, static_cast<void*>(pWorld->vPlayers.at(index)));
		return 1;
	}

	int LGetPlayerByName(lua_State* pVM)
	{
		const char* name = lua_tostring(pVM, 1);

		if (sizeof(name)/sizeof(char) <= 0)
			LUA_FAILURE("Player name too short")

		std::vector<Player*>::const_iterator it;
		Player* pPlayer = NULL;
		for (it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
		{
			if (strcmp((*it)->name.c_str(), name) == 0)
			{
				pPlayer = *it;
				break;
			}
		}

		lua_pushlightuserdata(pVM, static_cast<void*>(pPlayer));
		return 1;
	}
	
	int LAddPlayer(lua_State* pVM)
	{
		const char *sz_name = lua_tostring(pVM, 1);
		PlayerType type = (PlayerType) lua_tointeger(pVM, 2);
		const char *sz_texture = lua_tostring(pVM, 3);
		const char *raceScript = lua_tostring(pVM, 4);
		const char *aiScript = lua_tostring(pVM, 5);
		std::string texture = sz_texture, name = sz_name;
		if (name == "USER")
		{
			std::stringstream ss("Player ");
			ss << (pWorld->vPlayers.size() + 1);
			name = ss.str();
		}
		if (texture == "USER")
		{
			std::stringstream ss;
			ss << "textures/player_" << pWorld->vPlayers.size() << ".png";
			texture = ss.str();
		}
		if (!strcmp(raceScript, "USER"))
		{
			raceScript = "robots";
		}
		new Player(name, type, texture, std::string(raceScript), std::string(aiScript));
		LUA_SUCCESS
	}

	int LSetCurrentPlayer(lua_State* pVM)
	{
		currentPlayer = pWorld->vPlayers[lua_tointeger(pVM, 1)];
		LUA_SUCCESS
	}

	int LSetCurrentPlayerView(lua_State* pVM)
	{
		currentPlayerView = pWorld->vPlayers[lua_tointeger(pVM, 1)];
		LUA_SUCCESS
	}

	int LIsNonNull(lua_State* pVM)
	{
		void* val = lua_touserdata(pVM, 1);
		lua_pushboolean(pVM, val != NULL);
		return 1;
	}

	int LNull(lua_State* pVM)
	{
		lua_pushlightuserdata(pVM, NULL);
		return 1;
	}

	int LIsValidUnit(lua_State* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		if (pUnit != NULL && IsDisplayedUnitPointer(pUnit) && pUnit->isDisplayed)
		{
			LUA_SUCCESS
		}
		else
		{
			LUA_FAIL
		}
	}

	int LGetUnitTypeName(lua_State* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		const char* name = "";
		if (pUnitType)
			name = pUnitType->id.c_str();

		lua_pushstring(pVM, name);
		return 1;
	}

	int LOutput(lua_State* pVM)
	{
		const char *str = lua_tostring(pVM, 1);
		cout << str;
		return 0;
	}
	
	int LLoadHeightmap(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LLoadHeightmap: Incorrect arguments")
		}
		
		if (!lua_isstring(pVM, 1))
		{
			LUA_FAILURE("LLoadHeightmap: First argument not string")
		}
	
		std::string filename = "maps/" + (std::string) lua_tostring(pVM, 1);
		int ret = Game::Dimension::LoadWorld(filename);
		if (ret == SUCCESS)
		{
			LUA_SUCCESS
		}
		else
		{
			if (ret == FILE_DOES_NOT_EXIST)
				std::cout << filename << " doesn't exist!!!!" << std::endl;
			LUA_FAILURE("Error during world load")
		}
		
		LUA_FAILURE("Should never happen!") // << huh, should never happen
	}
	
	int LSetHeightmapModifier(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		
		if (value < 0)
		{
			LUA_FAILURE("Invalid argument, below zero")
		}
		
		Game::Dimension::terrainHeight = value;
		LUA_SUCCESS
	}
	
	int LSetMaximumBuildingAltitude(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("Firsta argument no number")
		}
		
		GLfloat value = (GLfloat)lua_tonumber(pVM, 1);
		
		if (value > 1.0f || value <= -1.0f)
		{
			LUA_FAILURE("Invalid altitude. Using preset.")
		}
		
		Game::Dimension::unitBuildingMaximumAltitude = value;
		
		LUA_SUCCESS
	}
	
	int LLoadTerrainTexture(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			lua_pushlightuserdata(pVM, NULL);
			return 1;
		}
		
		if (!lua_isstring(pVM, 1))
		{
			lua_pushlightuserdata(pVM, NULL);
			return 1;
		}
	
		std::stringstream ss("");
		ss << "textures/";
		ss << lua_tostring(pVM, 1);
		
		SDL_Surface* img = Utilities::LoadImage(ss.str().c_str());
		
		if (img != NULL)
		{
			Game::Dimension::terraintexture = Utilities::CreateGLTexture(img);
		}
		
		lua_pushlightuserdata(pVM, (void*)img);
		return 1;
	}

#define ASSERT_PARAM_COUNT(count) \
	for (int lua_idx = 1; lua_idx <= (count); lua_idx++) \
	{ \
		if (lua_isnil(pVM, lua_idx)) \
			LUA_FAILURE("Incorrect argument count") \
	}

	int LSetTerrainAmbientDiffuse(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(6)

		if (!lua_isboolean(pVM, 1))
		{
			LUA_FAILURE("First argument no boolean")
		}

		if (!lua_isboolean(pVM, 2))
		{
			LUA_FAILURE("Second argument no boolean")
		}

		for (int i = 3; i <= 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values.")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		GLfloat* ptr;
		int seen    = lua_toboolean(pVM, 1);
		int lighted = lua_toboolean(pVM, 2);

		if (!seen && !lighted)
		{
			ptr = Game::Dimension::terrainMaterialAmbientDiffuse[0][0];
		}
		else if (seen && !lighted)
		{
			ptr = Game::Dimension::terrainMaterialAmbientDiffuse[0][1];
		}
		else if (!seen && lighted)
		{
			ptr = Game::Dimension::terrainMaterialAmbientDiffuse[1][0];
		}
		else
		{
			ptr = Game::Dimension::terrainMaterialAmbientDiffuse[1][1];
		}

		for (int i = 3; i <= 6; i++)
		{
			ptr[i-3] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetTerrainSpecular(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::terrainMaterialSpecular[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetTerrainEmission(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::terrainMaterialEmission[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetTerrainShininess(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect argument count")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}

		Game::Dimension::terrainMaterialShininess = (GLfloat)lua_tonumber(pVM, 1);

		LUA_SUCCESS
	}
	
	int LSetWaterLevel(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no string")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		Game::Dimension::waterLevel = value;
		
		LUA_SUCCESS
	}
	
	int LSetWaterHeight(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no string")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		Game::Dimension::waterHeight = value;
		
		LUA_SUCCESS
	}

	int LSetWaterAmbientDiffuse(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(6)

		if (!lua_isboolean(pVM, 1))
		{
			LUA_FAILURE("First argument no boolean")
		}

		if (!lua_isboolean(pVM, 2))
		{
			LUA_FAILURE("Second argument no boolean")
		}

		for (int i = 3; i <= 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values.")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		GLfloat* ptr;
		int seen    = lua_toboolean(pVM, 1);
		int lighted = lua_toboolean(pVM, 2);

		if (!seen && !lighted)
		{
			ptr = Game::Dimension::waterMaterialAmbientDiffuse[0][0];
		}
		else if (seen && !lighted)
		{
			ptr = Game::Dimension::waterMaterialAmbientDiffuse[0][1];
		}
		else if (!seen && lighted)
		{
			ptr = Game::Dimension::waterMaterialAmbientDiffuse[1][0];
		}
		else
		{
			ptr = Game::Dimension::waterMaterialAmbientDiffuse[1][1];
		}

		for (int i = 3; i <= 6; i++)
		{
			ptr[i-3] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetWaterSpecular(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::waterMaterialSpecular[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetWaterEmission(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::waterMaterialEmission[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetWaterShininess(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect argument count")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}

		Game::Dimension::waterMaterialShininess = (GLfloat)lua_tonumber(pVM, 1);

		LUA_SUCCESS
	}
	
	int LSetWaterColor(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(3)
		
		GLfloat values[] = { 0, 0, 0 };
		int i = 0;
		for (i = 1; i < 4; i++ )
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("non-numeric values was provided")
			}
			
			values[i - 1] = (GLfloat)lua_tonumber(pVM, i);
			
			if (values[i - 1] > 1.0f || values[ i - 1 ] < 0)
			{
				LUA_FAILURE("invalid value (too high/low)")
			}
		}
		
		for (i = 0; i < 3; i++)
			Game::Dimension::waterColor[i] = values[i];
		
		LUA_SUCCESS
	}
	
	int LPrepareGUI(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		SDL_Surface* p = (SDL_Surface*) lua_touserdata(pVM, 1);
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		Game::Rules::GameWindow::Instance()->InitGUI(p);
		
		p = NULL;
		
		LUA_SUCCESS
	}
	
	int LFreeSurface(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		SDL_Surface* p = (SDL_Surface*) lua_touserdata(pVM, 1);
		
		if (p != NULL)
		{
			SDL_FreeSurface(p);
		}
		
		p = NULL;
		
		LUA_SUCCESS
	}
	
	int LAllocEnvironmentalCondition(lua_State* pVM)
	{
		Game::Dimension::Environment::EnvironmentalCondition* env = new
			Game::Dimension::Environment::EnvironmentalCondition;
			
		lua_pushlightuserdata(pVM, (void*)env);	
		
		return 1;
	}
		
	#define GET_ENVIRONMENTAL_CONDITION(index, p)\
		Game::Dimension::Environment::EnvironmentalCondition* (p) = \
		(Game::Dimension::Environment::EnvironmentalCondition*) lua_touserdata(pVM, (index) );
	
	int LSetHours(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(3)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		if (!lua_isnumber(pVM, 2) ||
			!lua_isnumber(pVM, 3))
		{
			LUA_FAILURE("Second or third argument no number")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		p->hourBegin = (float) lua_tonumber(pVM, 2);
		p->hourEnd   = (float) lua_tonumber(pVM, 3);
		
		LUA_SUCCESS
	}
	
	int LSetType(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		const char* type = lua_tostring(pVM, 2);
		
		if (strcmp(type, "night") == 0)
			p->isNight = true;
		else if (strcmp(type, "day") == 0)
			p->isDay = true;
		else if (strcmp(type, "dawn") == 0)
			p->isDawn = true;
		else if (strcmp(type, "dusk") == 0)
			p->isDusk = true;
		else
		{
			LUA_FAILURE("Invalid environmental condition type")
		}
		
		LUA_SUCCESS
	}
	
	int LSetMusicList(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("Second argument no string")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		p->musicListTag = lua_tostring(pVM, 2);
		
		LUA_SUCCESS
	}
	
	int LSetSkybox(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("Second argument no string")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		int idx = Game::Dimension::Environment::FourthDimension::Instance()->GetSkybox(lua_tostring(pVM, 2));
		
		if (idx == -1)
		{
			LUA_FAILURE("Invalid skybox tag")
		}
		
		p->skybox = idx;
		
		LUA_SUCCESS
	}
	
	int LSetSunPos(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->sunPos[i] = (float) lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LSetDiffuse(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->diffuse[i] = (float) lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS	
	}
	
	int LSetAmbient(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->ambient[i] = (float) lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LSetFogParams(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(4)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		for (int i = 2; i < 5; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		p->fogBegin     = (float) lua_tonumber(pVM, 2);
		p->fogEnd       = (float) lua_tonumber(pVM, 3);
		p->fogIntensity = (float) lua_tonumber(pVM, 4);
		
		LUA_SUCCESS
	}
	
	int LSetFogColor(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->fogColor[i] = (float) lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LAddEnvironmentalCondition(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("Null pointer")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->AddCondition(p);
		
		LUA_SUCCESS
	}
	
	int LValidateEnvironmentalConditions(lua_State* pVM)
	{
		if (!Game::Dimension::Environment::FourthDimension::Instance()->ValidateConditions())
			LUA_FAIL
			
		LUA_SUCCESS
	}
	
	int LInitSkybox(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isnumber(pVM, 1) ||
		    !lua_isnumber(pVM, 2))
		{
			LUA_FAILURE("Invalid arguments")
		}
	
		int detail  = (int)lua_tonumber(pVM, 1),
		    hdetail = (int)lua_tonumber(pVM, 2);
			
		if (detail > 100 || hdetail > 50)
		{
			LUA_FAILURE("Too detailed skybox")
		}
	
		Game::Dimension::Environment::FourthDimension::Instance()->InitSkyBox(detail, hdetail);
	
		LUA_SUCCESS
	}
	
	int LSetDayLength(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Invalid arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}
		
		double length = lua_tonumber(pVM, 1);
		
		if (length < 0 || length > 1000)
		{
			LUA_FAILURE("Invalid value")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->SetDayLength((int)length);
		
		LUA_SUCCESS
	}
	
	int LSetHourLength(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Invalid arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}
		
		double length = lua_tonumber(pVM, 1);
		
		if (length < 0 || length > 1000)
		{
			LUA_FAILURE("Invalid value")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->SetHourLength((float)length);
		
		LUA_SUCCESS
	}
	
	int LSetCurrentHour(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}
		
		int n = (int)lua_tonumber(pVM, 1);
		Game::Dimension::Environment::FourthDimension* p = 
		    Game::Dimension::Environment::FourthDimension::Instance();
		
		if (n < 0 || n > p->GetDayLength())
		{
			LUA_FAILURE("Invalid value")
		}
	
		p->SetCurrentHour((float)n);
		
		LUA_SUCCESS
	}

	int LFocusCameraOnUnit(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}

		if (!lua_islightuserdata(pVM, 1))
		{
			LUA_FAILURE("First argument no unit pointer")
		}

		Game::Dimension::Unit* unit = (Game::Dimension::Unit*)lua_touserdata(pVM, 1);

		if (unit == NULL)
		{
			LUA_FAILURE("Null pointer")
		}

		Game::Dimension::Camera* camera = Game::Rules::GameWindow::Instance()->GetCamera();
		GLfloat zoom = camera->GetZoom();
		GLfloat rotation = camera->GetRotation();

		if (!lua_isnil(pVM, 2))
		{
			if (lua_isnumber(pVM, 2))
			{
				if (lua_tonumber(pVM, 2) != -1)
					zoom = (GLfloat)lua_tonumber(pVM, 2);
			}
		}

		if (!lua_isnil(pVM, 3))
		{
			if (lua_isnumber(pVM, 3))
			{
				if (lua_tonumber(pVM, 3) != -1)
					rotation = (GLfloat)lua_tonumber(pVM, 3);
			}
		}

		camera->SetCamera(unit, zoom, rotation);

		LUA_SUCCESS
	}

	int LFocusCameraOnCoord(lua_State* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}

		if (!lua_isnumber(pVM, 2))
		{
			LUA_FAILURE("Second argument no number")
		}

		GLfloat x = (GLfloat)lua_tonumber(pVM, 1);
		GLfloat y = (GLfloat)lua_tonumber(pVM, 2);

		if (x < 0 || x > Game::Dimension::pWorld->width)
		{
			LUA_FAILURE("Invalid X - value")
		}

		if (y < 0 || y > Game::Dimension::pWorld->height)
		{
			LUA_FAILURE("Invalid Y - value")
		}

		Game::Dimension::Camera* camera = Game::Rules::GameWindow::Instance()->GetCamera();
		GLfloat zoom = camera->GetZoom();
		GLfloat rotation = camera->GetRotation();

		if (!lua_isnil(pVM, 3))
		{
			if (lua_isnumber(pVM, 3))
			{
				if (lua_tonumber(pVM, 3) != -1)
					zoom = (GLfloat)lua_tonumber(pVM, 3);
			}
		}

		if (!lua_isnil(pVM, 4))
		{
			if (lua_isnumber(pVM, 4))
			{
				if (lua_tonumber(pVM, 4) != -1)
					rotation = (GLfloat)lua_tonumber(pVM, 4);
			}
		}

		camera->SetFocus(x, y);
		camera->Rotate(rotation);
		camera->Zoom(zoom);

		LUA_SUCCESS
	}

	int LRotateCamera(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argumetn no number")
		}

		GLfloat rotation = (GLfloat)lua_tonumber(pVM, 1);

		if (rotation <= -360.0f || rotation >= 360.0f || rotation == 0)
		{
			LUA_FAILURE("Invalid rotation value. -360 < r < 360  r != 1")
		}

		Game::Rules::GameWindow::Instance()->GetCamera()->Rotate(rotation);

		LUA_SUCCESS
	}

	int LZoomCamera(lua_State* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("Incorrect arguments")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("First argument no number")
		}

		GLfloat zoom = (GLfloat)lua_tonumber(pVM, 1);

		Game::Rules::GameWindow::Instance()->GetCamera()->Zoom(zoom);

		LUA_SUCCESS
	}

	int LGetLUAScript(lua_State* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		if (!filename)
		{
			LUA_FAILURE("Invalid String")
		}
		std::string filepath = Utilities::GetDataFile("scripts/" + (std::string) filename);

		if (!filepath.length())
		{
			LUA_FAILURE("Script not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetDataFile(lua_State* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		std::string filepath = Utilities::GetDataFile(filename);

		if (!filepath.length())
		{
			LUA_FAILURE("File not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetConfigFile(lua_State* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		std::string filepath = Utilities::GetConfigFile(filename);

		if (!filepath.length())
		{
			LUA_FAILURE("File not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetWritableConfigFile(lua_State* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		bool exists;
		std::string filepath = Utilities::GetWritableConfigFile(filename, exists);

		if (!filepath.length())
		{
			LUA_FAILURE("Could not find a directory to write in")
		}

		lua_pushstring(pVM, filepath.c_str());
		lua_pushboolean(pVM, exists);
		return 2;
	}

	int LLoadLuaScript(lua_State* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
	
		if (!filename)
			LUA_FAILURE("Invalid filename - null pointer")

		Utilities::Scripting::LuaVMState* pVM_object = GetObjectByVMstate(pVM);

		if (pVM_object->DoFile(filename))
		{
			LUA_FAILURE("Error loading LUA script " << filename)
		}

		LUA_SUCCESS
	}

	int LUnloadAllUnitTypes(lua_State* pVM)
	{
		UnloadAllUnitTypes();
		LUA_SUCCESS
	}

	int LUnloadUnitType(lua_State* pVM)
	{
		// TODO
/*		const char *sz_name = lua_tostring(pVM, 1);

		if (!sz_name)
			LUA_FAILURE("Invalid unittype string - null pointer")

		const string name = sz_name;

		UnitType* pUnitType = unitTypeMap[name];
	
		if (!pUnitType)
			LUA_FAILURE("No such unittype found: " << name)
		
		UnloadUnitType(pUnitType);*/
		LUA_SUCCESS
	}

#define	GET_INT_FIELD(dest, field) \
lua_getfield(pVM, -1, field); \
if (!lua_isnumber(pVM, -1)) \
{ \
	lua_pop(pVM, 1); \
	LUA_FAILURE("Required field \"" field  "\" not found in unit description or is not a number") \
} \
dest = lua_tointeger(pVM, -1); \
lua_pop(pVM, 1);

#define	GET_INT_FIELD_OPTIONAL(dest, field, def) \
lua_getfield(pVM, -1, field); \
if (!lua_isnumber(pVM, -1)) \
{ \
	dest = def; \
	lua_pop(pVM, 1); \
} \
else \
{ \
	dest = lua_tointeger(pVM, -1); \
	lua_pop(pVM, 1); \
}

#define	GET_ENUM_FIELD_OPTIONAL(dest, field, def, type) \
lua_getfield(pVM, -1, field); \
if (!lua_isnumber(pVM, -1)) \
{ \
	dest = def; \
	lua_pop(pVM, 1); \
} \
else \
{ \
	dest = (type) lua_tointeger(pVM, -1); \
	lua_pop(pVM, 1); \
}

#define	GET_BOOL_FIELD(dest, field) \
lua_getfield(pVM, -1, field); \
if (!lua_isboolean(pVM, -1)) \
{ \
	lua_pop(pVM, 1); \
	LUA_FAILURE("Required field \""field"\" not found in unit description or is not a boolean") \
} \
dest = lua_toboolean(pVM, -1); \
lua_pop(pVM, 1);

#define	GET_BOOL_FIELD_OPTIONAL(dest, field, def) \
lua_getfield(pVM, -1, field); \
if (!lua_isboolean(pVM, -1)) \
{ \
	dest = def; \
	lua_pop(pVM, 1); \
} \
else \
{ \
	dest = lua_toboolean(pVM, -1); \
	lua_pop(pVM, 1); \
}

#define	GET_FLOAT_FIELD(dest, field) \
lua_getfield(pVM, -1, field); \
if (!lua_isnumber(pVM, -1)) \
{ \
	lua_pop(pVM, 1); \
	LUA_FAILURE("Required field \""field"\" not found in unit description or is not a number") \
} \
dest = (float) lua_tonumber(pVM, -1); \
lua_pop(pVM, 1);

#define	GET_FLOAT_FIELD_OPTIONAL(dest, field, def) \
lua_getfield(pVM, -1, field); \
if (!lua_isnumber(pVM, -1)) \
{ \
	dest = def; \
	lua_pop(pVM, 1); \
} \
else \
{ \
	dest = (float) lua_tonumber(pVM, -1); \
	lua_pop(pVM, 1); \
}

#define	GET_STDSTRING_FIELD(dest, field) \
lua_getfield(pVM, -1, field); \
if (!lua_isstring(pVM, -1)) \
{ \
	lua_pop(pVM, 1); \
	LUA_FAILURE("Required field \""field"\" not found in unit description or is not a string") \
} \
dest = std::string(lua_tostring(pVM, -1)); \
lua_pop(pVM, 1);

#define	GET_STDSTRING_FIELD_OPTIONAL(dest, field, def) \
lua_getfield(pVM, -1, field); \
if (!lua_isstring(pVM, -1)) \
{ \
	dest = def; \
	lua_pop(pVM, 1); \
} \
else \
{ \
	dest = std::string(lua_tostring(pVM, -1)); \
	lua_pop(pVM, 1); \
}

	void InterpretStringTableField(lua_State* pVM, const char *field, vector<std::string>& datadump)
	{
		lua_getfield(pVM, 1, field);
		if (lua_istable(pVM, 2))
		{
			int i = 1;
			int num = 0;
			while (1)
			{
				lua_pushinteger(pVM, i++);
				lua_gettable(pVM, 2);
				if (lua_isnil(pVM, 3) || !lua_isstring(pVM, 3))
				{
					lua_pop(pVM, 1);
					break;
				}
				lua_pop(pVM, 1);
				num++;
			}
			for (i = 1; i <= num; i++)
			{
				lua_pushinteger(pVM, i);
				lua_gettable(pVM, 2);
				datadump.push_back(std::string(lua_tostring(pVM, 3)));
				lua_pop(pVM, 1);
			}
		}
		lua_pop(pVM, 1);
	}

	map<UnitType*, bool> validUnitTypePointers;

	bool IsValidUnitTypePointer(UnitType* unittype)
	{
		return validUnitTypePointers[unittype];
	}

	UnitType *GetUnitTypeByID(Player* owner, std::string str)
	{
		return owner->unitTypeMap[str];
	}

	Research *GetResearchByID(Player* owner, std::string str)
	{
		return owner->researchMap[str];
	}

	int LCreateUnitType(lua_State* pVM)
	{
		UnitType *pUnitType;
		const char* model;
		Player *player = GetPlayerByVMstate(pVM);

		if (!lua_istable(pVM, 1))
			LUA_FAILURE("Invalid argument, must be a table")

		pUnitType = new UnitType;

		pUnitType->player = player;
		pUnitType->numBuilt = 0;
		pUnitType->numExisting = 0;

		GET_STDSTRING_FIELD(pUnitType->id, "id")

		GET_STDSTRING_FIELD_OPTIONAL(pUnitType->name, "name", pUnitType->id)

		GET_INT_FIELD(pUnitType->maxHealth, "maxHealth")
		GET_INT_FIELD(pUnitType->heightOnMap, "heightOnMap")
		GET_INT_FIELD(pUnitType->widthOnMap, "widthOnMap")
		
		GET_FLOAT_FIELD(pUnitType->size, "size")
		GET_FLOAT_FIELD(pUnitType->height, "height")

		GET_INT_FIELD_OPTIONAL(pUnitType->maxPower, "maxPower", 0)
		GET_INT_FIELD_OPTIONAL(pUnitType->minAttack, "minAttack", 0)
		GET_INT_FIELD_OPTIONAL(pUnitType->maxAttack, "maxAttack", 0)
		GET_INT_FIELD_OPTIONAL(pUnitType->requirements.money, "buildCost", 0)
		GET_INT_FIELD_OPTIONAL(pUnitType->requirements.time, "buildTime", 0)
		GET_INT_FIELD_OPTIONAL(pUnitType->requirements.power, "buildPower", 0)

		GET_ENUM_FIELD_OPTIONAL(pUnitType->powerType, "powerType", POWERTYPE_TWENTYFOURSEVEN, PowerType)
		GET_ENUM_FIELD_OPTIONAL(pUnitType->movementType, "movementType", MOVEMENT_SMALLVEHICLE, MovementType)

		GET_BOOL_FIELD_OPTIONAL(pUnitType->canAttack, "canAttack", false)
		GET_BOOL_FIELD_OPTIONAL(pUnitType->isMobile, "isMobile", false)
		GET_BOOL_FIELD_OPTIONAL(pUnitType->hasAI, "hasAI", false)
		GET_BOOL_FIELD_OPTIONAL(pUnitType->hasLuaAI, "hasLuaAI", false)

		GET_FLOAT_FIELD_OPTIONAL(pUnitType->armor, "armor", 100.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->regenHealth, "regenHealth", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->regenPower, "regenPower", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->attackAccuracy, "attackAccuracy", 100.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->attackMinRange, "attackMinRange", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->attackMaxRange, "attackMaxRange", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->sightRange, "sightRange", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->lightRange, "lightRange", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->powerIncrement, "powerIncrement", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->powerUsage, "powerUsage", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->lightPowerUsage, "lightPowerUsage", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->attackPowerUsage, "attackPowerUsage", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->movePowerUsage, "movePowerUsage", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->buildPowerUsage, "buildPowerUsage", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->movementSpeed, "movementSpeed", 0.0)
		GET_FLOAT_FIELD_OPTIONAL(pUnitType->attackSpeed, "attackSpeed", 0.0)

		lua_getfield(pVM, 1, "symbol");
		if (lua_isstring(pVM, 2))
		{
			const char* symbol = lua_tostring(pVM, 2);
			pUnitType->Symbol = LoadTexture(symbol);
		}
		else
		{
			pUnitType->Symbol = 0;
		}
		lua_pop(pVM, 1);

		lua_getfield(pVM, 1, "model");
		if (!lua_isstring(pVM, 2))
		{
			lua_pop(pVM, 1);
			LUA_FAILURE("Required field \"model\" not found in unit description or is not a string")
		}
		model = lua_tostring(pVM, 2);
		lua_pop(pVM, 1);

		pUnitType->model = LoadModel(model);
					
		if(pUnitType->model == NULL)
		{
			LUA_FAILURE("Model could not be found or loaded")
		}
		else
		{
			for (int i = 0; i < Game::AI::ACTION_NUM; i++)
			{
				pUnitType->animations[i] = CreateAnimation(CreateTransAnim(CreateMorphAnim(1.0, 1, model, 0.0), NULL, 0, 1.0, 1, CreateTransformData(Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(1.0, 1.0, 1.0)), 0.0));
			}
		}

		InterpretStringTableField(pVM, "canBuild", pUnitType->canBuildIDs);
		InterpretStringTableField(pVM, "canResearch", pUnitType->canResearchIDs);

		bool isResearched = false;

		lua_getfield(pVM, 1, "isResearched");
		if (lua_isboolean(pVM, 2))
		{
			isResearched = lua_toboolean(pVM, 2);
		}
		lua_pop(pVM, 1);

		if (!isResearched)
		{
			Research *research = new Research;
			ResearchRequirement res_req;
			ConjunctiveRequirements creq;

			research->id = "Research" + std::string(pUnitType->id);
			research->name = "Research " + std::string(pUnitType->name);
			research->description = "Research " + std::string(pUnitType->name);
			research->isResearched = false;
			research->luaEffectObj = "";
			research->icon = pUnitType->Symbol;
			research->player = player;

			GET_INT_FIELD(research->requirements.money, "researchCost")
			GET_INT_FIELD(research->requirements.time, "researchTime")
			GET_INT_FIELD_OPTIONAL(research->requirements.power, "researchPower", 0)

			res_req.research = research;
			res_req.desiredState = true;

			creq.researchs.push_back(res_req);

			pUnitType->requirements.creation.dreqs.push_back(creq);
			research->index = player->vResearchs.size();
			player->vResearchs.push_back(research);
			player->researchMap[research->id] = research;

		}
		else
		{
			GET_STDSTRING_FIELD_OPTIONAL(pUnitType->requirements.creation.dReqString, "crequirements", "")
		}

		GET_STDSTRING_FIELD_OPTIONAL(pUnitType->requirements.existance.dReqString, "erequirements", "")

		pUnitType->index = player->vUnitTypes.size();
		player->vUnitTypes.push_back(pUnitType);

		player->unitTypeMap[pUnitType->id] = pUnitType;

		lua_getfield(pVM, 1, "projectileType");
		if (lua_istable(pVM, 2))
		{
			pUnitType->projectileType = new ProjectileType;
		
			lua_getfield(pVM, -1, "model");
			if (!lua_isstring(pVM, -1))
			{
				lua_pop(pVM, 1);
				LUA_FAILURE("Required field \"model\" not found in projectiletype table or is not a string")
			}
			pUnitType->projectileType->model = LoadModel(lua_tostring(pVM, -1));
			lua_pop(pVM, 1);

			GET_FLOAT_FIELD_OPTIONAL(pUnitType->projectileType->size, "size", 0.0)
			GET_FLOAT_FIELD_OPTIONAL(pUnitType->projectileType->speed, "speed", 0.0)
			GET_FLOAT_FIELD_OPTIONAL(pUnitType->projectileType->areaOfEffect, "areaOfEffect", 0.0)
			GET_BOOL_FIELD_OPTIONAL(pUnitType->projectileType->isHoming, "isHoming", false)
			
			lua_getfield(pVM, -1, "startPos");
			if (lua_isstring(pVM, -1))
			{
				lua_pushinteger(pVM, 1);
				lua_gettable(pVM, -2);
				if (lua_isnumber(pVM, -1))
				{
					pUnitType->projectileType->startPos.x = (float) lua_tonumber(pVM, -1);
				}
				lua_pop(pVM, 1);
				
				lua_pushinteger(pVM, 2);
				lua_gettable(pVM, -2);
				if (lua_isnumber(pVM, -1))
				{
					pUnitType->projectileType->startPos.y = (float) lua_tonumber(pVM, -1);
				}
				lua_pop(pVM, 1);
				
				lua_pushinteger(pVM, 3);
				lua_gettable(pVM, -2);
				if (lua_isnumber(pVM, -1))
				{
					pUnitType->projectileType->startPos.z = (float) lua_tonumber(pVM, -1);
				}
				lua_pop(pVM, 1);
			}
			lua_pop(pVM, 1);

		}
		else
		{
			pUnitType->projectileType = NULL;
		}
		lua_pop(pVM, 1);

		for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
		{
			pUnitType->actionSounds[i] = NULL;
		}

		lua_pushlightuserdata(pVM, pUnitType);
		lua_setfield(pVM, 1, "pointer");

		lua_pushvalue(pVM, 1); // Create a copy of the unittype table
		lua_setglobal(pVM, pUnitType->id.c_str()); // Set it as a new global

		lua_getglobal(pVM, "UnitTypes");
		if (lua_isnil(pVM, 2))
		{
			lua_pop(pVM, 1);
			lua_newtable(pVM);
			lua_setglobal(pVM, "UnitTypes");
		}

		lua_pushvalue(pVM, 1); // Create a copy of the unittype table
		lua_setfield(pVM, 2, pUnitType->id.c_str());
		lua_pop(pVM, 1);

		pUnitType->playerAIFuncs = pWorld->vPlayers[player->index]->playerAIFuncs;

		pUnitType->unitAIFuncs = pWorld->vPlayers[player->index]->unitAIFuncs;

		pUnitType->GenerateRanges();

		validUnitTypePointers[pUnitType] = true;

		LUA_SUCCESS
	}

	int LCreateResearch(lua_State* pVM)
	{
		Research *pResearch;
		Player *player = GetPlayerByVMstate(pVM);

		if (!lua_istable(pVM, 1))
			LUA_FAILURE("Invalid argument, must be a table")

		pResearch = new Research;

		pResearch->player = player;

		GET_STDSTRING_FIELD(pResearch->id, "id")
		GET_STDSTRING_FIELD(pResearch->name, "name")
		GET_STDSTRING_FIELD(pResearch->description, "description")
		GET_STDSTRING_FIELD_OPTIONAL(pResearch->luaEffectObj, "luaEffectObj", "")
		GET_INT_FIELD_OPTIONAL(pResearch->requirements.money, "cost", 0)
		GET_INT_FIELD_OPTIONAL(pResearch->requirements.time, "time", 0)
		GET_INT_FIELD_OPTIONAL(pResearch->requirements.power, "power", 0)

		pResearch->icon = 0;
		pResearch->isResearched = false;

		GET_STDSTRING_FIELD_OPTIONAL(pResearch->requirements.creation.dReqString, "crequirements", "")
		GET_STDSTRING_FIELD_OPTIONAL(pResearch->requirements.existance.dReqString, "erequirements", "")
			
		pResearch->index = player->vResearchs.size();
		player->vResearchs.push_back(pResearch);
		player->researchMap[pResearch->id] = pResearch;

		LUA_SUCCESS
	}

	std::string GetReqStringSymbol(const char *&reqstring)
	{
		std::string symbol;
		int lastAddedChar = 0;
		bool hasCharsBefore = false;
		while (1)
		{
			char c = *reqstring;
			if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != ' ')
			{
				return symbol;
			}
			if (c == ' ')
			{
				
			}
			else
			{
				if (hasCharsBefore && lastAddedChar > 0)
				{
					symbol += std::string(reqstring-lastAddedChar, lastAddedChar+1);
				}
				else
				{
					symbol += c;
				}
				lastAddedChar = -1;
				hasCharsBefore = true;
			}
			reqstring++;
			lastAddedChar++;
		}
	}

	void InterpretRequirementsString(Player *player, const char *reqstring, DisjunctiveRequirements &requirements)
	{
		if (!reqstring || *reqstring == 0)
		{
			return;
		}

		while (1)
		{
			ConjunctiveRequirements creqs;
			while (1)
			{
				bool negated = false;
				while (*reqstring == ' ') reqstring++;
				if (*reqstring == '!')
				{
					negated = true;
					reqstring++;
				}
				std::string symbol = GetReqStringSymbol(reqstring);
				UnitType* unitType = GetUnitTypeByID(player, symbol);
				Research* research = GetResearchByID(player, symbol);
				UnitRequirement unit_req;
				ResearchRequirement res_req;
	
				if (unitType && research)
				{
					std::cout << "Ambiguous symbol '" << symbol << "', can both be unittype and research" << std::endl;
					return;
				}
				else if (unitType)
				{
					unit_req.type = unitType;
					unit_req.minBuilt = 0;
					unit_req.maxBuilt = INT_MAX;
					unit_req.minExisting = 1;
					unit_req.maxExisting = INT_MAX;
					if (negated)
					{
						std::cout << "Unit requirements cannot be negated" << std::endl;
						return;
					}
				}
				else if (research)
				{
					res_req.research = research;
					res_req.desiredState = !negated;
				}
				else
				{
					std::cout << "Symbol '" << symbol << "' could not be resolved" << std::endl;
					return;
				}

				if (unitType && *reqstring == '(')
				{
					unit_req.minExisting = 0;
					unit_req.maxExisting = INT_MAX;
					reqstring++;
					while (1)
					{

						while (*reqstring == ' ') reqstring++;

						int type;

						if (*reqstring == 'b')
						{
							type = 1;
						}
						else if (*reqstring == 'e')
						{
							type = 2;
						}
						else
						{
							std::cout << "Error detected while parsing requirements string, 'b' or 'e' expected; '" << *reqstring << "' found" << std::endl;
							return;
						}
						
						reqstring++;

						while (*reqstring == ' ') reqstring++;

						int ctype = 0;

						switch (*reqstring)
						{
							case '>':
								reqstring++;
								if (*reqstring == '=')
								{
									reqstring++;
									ctype = 2;
								}
								else
								{
									ctype = 1;
								}
								break;
							case '<':
								reqstring++;
								if (*reqstring == '=')
								{
									reqstring++;
									ctype = 4;
								}
								else
								{
									ctype = 5;
								}
								break;
							case '=':
								ctype = 3;
								reqstring++;
								break;
							default:
								std::cout << "Error detected while parsing requirements string, '>', '=' or '<' expected; '" << *reqstring << "' found" << std::endl;
								return;
						}

						while (*reqstring == ' ') reqstring++;

						int value = 0;

						while (*reqstring >= '0' && *reqstring <= '9')
						{
							value = value * 10 + *reqstring - '0';
							reqstring++;
						}

						switch (ctype)
						{
							case 1:
								if (type == 1)
								{
									unit_req.minBuilt = value+1;
								}
								else if (type == 2)
								{
									unit_req.minExisting = value+1;
								}
								break;
							case 2:
								if (type == 1)
								{
									unit_req.minBuilt = value;
								}
								else if (type == 2)
								{
									unit_req.minExisting = value;
								}
								break;
							case 3:
								if (type == 1)
								{
									unit_req.minBuilt = value;
									unit_req.maxBuilt = value;
								}
								else if (type == 2)
								{
									unit_req.minExisting = value;
									unit_req.maxExisting = value;
								}
								break;
							case 4:
								if (type == 1)
								{
									unit_req.maxBuilt = value;
								}
								else if (type == 2)
								{
									unit_req.maxExisting = value;
								}
								break;
							case 5:
								if (type == 1)
								{
									unit_req.maxBuilt = value-1;
								}
								else if (type == 2)
								{
									unit_req.maxExisting = value-1;
								}
								break;
						}
						
						while (*reqstring == ' ') reqstring++;

						char c = *reqstring;
						if (c == ')')
						{
							reqstring++;
							break;
						}

						if (c == ',')
						{
							reqstring++;
						}
						else
						{
							std::cout << "Error detected while parsing requirements string, ',' or ')' expected; '" << c << "' found" << std::endl;
						}
					}

				}

				if (unitType)
				{
					creqs.units.push_back(unit_req);
				}
				else if (research)
				{
					creqs.researchs.push_back(res_req);
				}

				char c = *reqstring;
				if (c == '|' || c == 0)
				{
					break;
				}

				if (c == ',')
				{
					reqstring++;
				}
				else
				{
					std::cout << "Error detected while parsing requirements string, ',', '|' or end-of-line expected; '" << c << "' found" << std::endl;
					return;
				}
			}

			requirements.dreqs.push_back(creqs);

			char c = *reqstring;
			if (c == 0)
			{
				break;
			}
			if (c == '|')
			{
				reqstring++;
			}
			else
			{
				std::cout << "Error detected while parsing requirements string, '|' or end-of-line expected; '" << c << "' found" << std::endl;
				return;
			}
		}
	}

	void PostProcessReqStrings(Player *player, ObjectRequirements &requirements)
	{
		InterpretRequirementsString(player, requirements.creation.dReqString.c_str(), requirements.creation);
		InterpretRequirementsString(player, requirements.existance.dReqString.c_str(), requirements.existance);
	}

	void PostProcessBuildResearch(UnitType* unitType)
	{
		for (std::vector<std::string>::iterator it_bld = unitType->canBuildIDs.begin(); it_bld != unitType->canBuildIDs.end(); it_bld++)
		{
			std::string id = *it_bld;
			if (GetUnitTypeByID(unitType->player, id))
			{
				unitType->canBuild.push_back(GetUnitTypeByID(unitType->player, id));
			}
			else
			{
				std::cout << "Cannot resolve unit type with id '" << id << "'" << std::endl;
			}
		}
		for (std::vector<std::string>::iterator it_rch = unitType->canResearchIDs.begin(); it_rch != unitType->canResearchIDs.end(); it_rch++)
		{
			std::string id = *it_rch;
			Research* res1 = GetResearchByID(unitType->player, id);
			Research* res2 = GetResearchByID(unitType->player, "Research" + id);
			if (res1 || res2)
			{
				if (res1)
				{
					unitType->canResearch.push_back(res1);
				}
				else
				{
					unitType->canResearch.push_back(res2);
				}
			}
			else
			{
				std::cout << "Cannot resolve research with id '" << id << "'" << std::endl;
			}
		}
	}

	void PostProcessStrings()
	{
		for (std::vector<Player*>::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
		{
			Player *player = *it;
			for (std::vector<Research*>::iterator it_res = player->vResearchs.begin(); it_res != player->vResearchs.end(); it_res++)
			{
				Research *research = *it_res;
				PostProcessReqStrings(player, research->requirements);
			}
			for (std::vector<UnitType*>::iterator it_unt = player->vUnitTypes.begin(); it_unt != player->vUnitTypes.end(); it_unt++)
			{
				UnitType *unitType = *it_unt;
				PostProcessReqStrings(player, unitType->requirements);
				PostProcessBuildResearch(unitType);
			}
		}
	}

	void Init(Scripting::LuaVMState* pVM)
	{
		pVM->RegisterFunction("GetUnitHealth", LGetUnitHealth);
		pVM->RegisterFunction("GetUnitPower", LGetUnitPower);
		pVM->RegisterFunction("GetUnitMaxHealth", LGetUnitMaxHealth);
		pVM->RegisterFunction("GetUnitMaxPower", LGetUnitMaxPower);
		pVM->RegisterFunction("GetUnitAction", LGetUnitAction);
		pVM->RegisterFunction("GetUnitActionArg", LGetUnitActionArg);
		pVM->RegisterFunction("GetUnitPosition", LGetUnitPosition);
		pVM->RegisterFunction("GetUnitRotation", LGetUnitRotation);
		pVM->RegisterFunction("GetUnitType", LGetUnitType);
		pVM->RegisterFunction("GetUnitOwner", LGetUnitOwner);
		pVM->RegisterFunction("GetUnitIsMobile", LGetUnitIsMobile);
		pVM->RegisterFunction("GetUnitLightAmount", LGetUnitLightAmount);
		pVM->RegisterFunction("GetUnitCanAttack", LGetUnitCanAttack);
		pVM->RegisterFunction("GetUnitCanBuild", LGetUnitCanBuild);
		pVM->RegisterFunction("GetUnitLastAttack", LGetUnitLastAttack);
		pVM->RegisterFunction("GetUnitLastAttacked", LGetUnitLastAttacked);
		pVM->RegisterFunction("GetUnitTargetUnit", LGetUnitTargetUnit);
		pVM->RegisterFunction("GetUnitTargetPos", LGetUnitTargetPos);
		
		pVM->RegisterFunction("GetNearestUnitInRange", LGetNearestUnitInRange);
		pVM->RegisterFunction("GetNearestSuitableAndLightedPosition", LGetNearestSuitableAndLightedPosition);
		pVM->RegisterFunction("GetSuitablePositionForLightTower", LGetSuitablePositionForLightTower);
		
		pVM->RegisterFunction("SquaresAreLighted", LSquaresAreLighted);
		pVM->RegisterFunction("SquaresAreLightedAround", LSquaresAreLightedAround);

		pVM->RegisterFunction("SetUnitHealth", LSetUnitHealth);
		pVM->RegisterFunction("SetUnitPower", LSetUnitPower);
		
		pVM->RegisterFunction("IsResearched", LIsResearched);
		pVM->RegisterFunction("GetResearcher", LGetResearcher);
		pVM->RegisterFunction("GetBuilder", LGetBuilder);

		pVM->RegisterFunction("IsWithinRangeForBuilding", LIsWithinRangeForBuilding);

		pVM->RegisterFunction("Attack", LAttack);
		pVM->RegisterFunction("CanReach", LCanReach);
		
		pVM->RegisterFunction("CommandUnit_TargetUnit", LCommandUnit_TargetUnit);
		pVM->RegisterFunction("CommandUnit_TargetPos", LCommandUnit_TargetPos);
		pVM->RegisterFunction("CommandGoto", LCommandGoto);
		pVM->RegisterFunction("CommandMoveAttack", LCommandMoveAttack);
		pVM->RegisterFunction("CommandBuild", LCommandBuild);
		pVM->RegisterFunction("CommandResearch", LCommandResearch);
		pVM->RegisterFunction("CommandFollow", LCommandFollow);
		pVM->RegisterFunction("CommandAttack", LCommandAttack);
		pVM->RegisterFunction("CommandCollect", LCommandCollect);
		pVM->RegisterFunction("CommandRepair", LCommandRepair);
		pVM->RegisterFunction("CommandMoveAttackUnit", LCommandMoveAttackUnit);
		
		pVM->RegisterFunction("ClearAllActions", LClearAllActions);

		pVM->RegisterFunction("CreateUnit", LCreateUnit);
		pVM->RegisterFunction("CanCreateUnitAt", LCanCreateUnitAt);

		pVM->RegisterFunction("GetUnitTypeIncomeAtNoon", LGetUnitTypeIncomeAtNoon);
		pVM->RegisterFunction("GetUnitTypeIncomeAtNight", LGetUnitTypeIncomeAtNight);
		
		pVM->RegisterFunction("GetUnitTypeBuildCost", LGetUnitTypeBuildCost);
		pVM->RegisterFunction("GetUnitTypeFromString", LGetUnitTypeFromString);
		pVM->RegisterFunction("GetUnitTypeIsMobile", LGetUnitTypeIsMobile);

		pVM->RegisterFunction("GetPlayerByIndex", LGetPlayerByIndex);
		pVM->RegisterFunction("GetPlayerByName", LGetPlayerByName);
		pVM->RegisterFunction("AddPlayer", LAddPlayer);
		pVM->RegisterFunction("SetCurrentPlayer", LSetCurrentPlayer);
		pVM->RegisterFunction("SetCurrentPlayerView", LSetCurrentPlayerView);
		
		pVM->RegisterFunction("GetCurrentFrame", LGetCurrentFrame);
		pVM->RegisterFunction("GetAIFPS", LGetAIFPS);
		pVM->RegisterFunction("GetTime", LGetTime);
		pVM->RegisterFunction("GetPower", LGetPower);
		pVM->RegisterFunction("GetMoney", LGetMoney);
		pVM->RegisterFunction("GetIncomeAtNoon", LGetIncomeAtNoon);
		pVM->RegisterFunction("GetIncomeAtNight", LGetIncomeAtNight);
		pVM->RegisterFunction("GetPowerAtDawn", LGetPowerAtDawn);
		pVM->RegisterFunction("GetPowerAtDusk", LGetPowerAtDusk);
		pVM->RegisterFunction("GetDayLength", LGetDayLength);
		pVM->RegisterFunction("GetNightLength", LGetNightLength);
		pVM->RegisterFunction("SellPower", LSellPower);

		pVM->RegisterFunction("GetMapDimensions", LGetMapDimensions);
		
		pVM->RegisterFunction("IsNonNull", LIsNonNull);
		pVM->RegisterFunction("Null", LNull);
		pVM->RegisterFunction("IsValidUnit", LIsValidUnit);
		
		pVM->RegisterFunction("GetUnitTypeName", LGetUnitTypeName);
		pVM->RegisterFunction("Output", LOutput);
		
		pVM->RegisterFunction("LoadHeightmap", LLoadHeightmap);
		pVM->RegisterFunction("SetHeightmapModifier", LSetHeightmapModifier);
		pVM->RegisterFunction("SetMaximumBuildingAltitude", LSetMaximumBuildingAltitude);
		pVM->RegisterFunction("LoadTerrainTexture", LLoadTerrainTexture);
		pVM->RegisterFunction("SetTerrainAmbientDiffuse", LSetTerrainAmbientDiffuse);
		pVM->RegisterFunction("SetTerrainSpecular", LSetTerrainSpecular);
		pVM->RegisterFunction("SetTerrainEmission", LSetTerrainEmission);
		pVM->RegisterFunction("SetTerrainShininess", LSetTerrainShininess);
		pVM->RegisterFunction("SetWaterLevel", LSetWaterLevel);
		pVM->RegisterFunction("SetWaterHeight", LSetWaterHeight);
		pVM->RegisterFunction("SetWaterAmbientDiffuse", LSetWaterAmbientDiffuse);
		pVM->RegisterFunction("SetWaterSpecular", LSetWaterSpecular);
		pVM->RegisterFunction("SetWaterEmission", LSetWaterEmission);
		pVM->RegisterFunction("SetWaterShininess", LSetWaterShininess);
		pVM->RegisterFunction("SetWaterColor", LSetWaterColor);
		pVM->RegisterFunction("PrepareGUI", LPrepareGUI);
		pVM->RegisterFunction("FreeSurface", LFreeSurface);
		pVM->RegisterFunction("AllocEnvironmentalCondition", LAllocEnvironmentalCondition);
		pVM->RegisterFunction("SetHours", LSetHours);
		pVM->RegisterFunction("SetType", LSetType);
		pVM->RegisterFunction("SetMusicList", LSetMusicList);
		pVM->RegisterFunction("SetSkybox", LSetSkybox);
		pVM->RegisterFunction("SetSunPos", LSetSunPos);
		pVM->RegisterFunction("SetDiffuse", LSetDiffuse);
		pVM->RegisterFunction("SetAmbient", LSetAmbient);
		pVM->RegisterFunction("SetFogParams", LSetFogParams);
		pVM->RegisterFunction("SetFogColor", LSetFogColor);
		pVM->RegisterFunction("AddEnvironmentalCondition", LAddEnvironmentalCondition);
		pVM->RegisterFunction("ValidateEnvironmentalConditions", LValidateEnvironmentalConditions);
		pVM->RegisterFunction("InitSkybox", LInitSkybox);
		pVM->RegisterFunction("SetDayLength", LSetDayLength);
		pVM->RegisterFunction("SetHourLength", LSetHourLength);
		pVM->RegisterFunction("SetCurrentHour", LSetCurrentHour);

		pVM->RegisterFunction("FocusCameraOnUnit", LFocusCameraOnUnit);
		pVM->RegisterFunction("FocusCameraOnCoord", LFocusCameraOnCoord);
		pVM->RegisterFunction("RotateCamera", LRotateCamera);
		pVM->RegisterFunction("ZoomCamera", LZoomCamera);

		pVM->RegisterFunction("GetLUAScript", LGetLUAScript);
		pVM->RegisterFunction("GetDataFile", LGetDataFile);
		pVM->RegisterFunction("GetConfigFile", LGetConfigFile);
		pVM->RegisterFunction("GetWritableConfigFile", LGetWritableConfigFile);
		
		pVM->RegisterFunction("LoadLuaScript", LLoadLuaScript);
		pVM->RegisterFunction("UnloadUnitType", LUnloadUnitType);
		pVM->RegisterFunction("UnloadAllUnitTypes", LUnloadAllUnitTypes);
		
		pVM->RegisterFunction("CreateUnitType", LCreateUnitType);
		pVM->RegisterFunction("CreateResearch", LCreateResearch);
		
		pVM->RegisterFunction("SetEventHandler", LSetEventHandler);
		pVM->RegisterFunction("SetRegularAIDelay", LSetRegularAIDelay);
		pVM->RegisterFunction("SetRegularAIEnabled", LSetRegularAIEnabled);
	}
}
