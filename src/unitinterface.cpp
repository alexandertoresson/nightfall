#include "unitinterface.h"

#include "terrain.h"
#include "unit.h"
#include "aipathfinding.h"
#include "environment.h"
#include "networking.h"
#include "paths.h"

namespace UnitLuaInterface
{
	using namespace Game::Dimension;
	using namespace Game::AI;
	using namespace Utilities;

	Unit* _GetUnit(void* ptr)
	{
		if (ptr == NULL)
			return NULL;

		return reinterpret_cast<Unit*>(ptr);
	}

	int LGetUnitHealth(LuaVM* pVM)
	{
		float health = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			health = pUnit->health;

		lua_pushnumber(pVM, health);
		return 1;
	}

	int LGetUnitMaxHealth(LuaVM* pVM)
	{
		float health = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			health = pUnit->type->maxHealth;

		lua_pushnumber(pVM, health);
		return 1;
	}

	int LGetUnitPower(LuaVM* pVM)
	{
		float power = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			power = pUnit->power;

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetUnitMaxPower(LuaVM* pVM)
	{
		float power = -1.0f;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			power = pUnit->type->maxPower;

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetUnitAction(LuaVM* pVM)
	{
		UnitAction action = ACTION_NONE;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			action = pUnit->action;

		lua_pushinteger(pVM, action);
		return 1;
	}

	int LGetUnitActionArg(LuaVM* pVM)
	{
		void* arg = NULL;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			arg = pUnit->pMovementData->action.arg;

		lua_pushlightuserdata(pVM, arg);
		return 1;
	}

	int LGetUnitPosition(LuaVM* pVM)
	{
		float position[2] = { 0, 0 };
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
		{
			position[0] = pUnit->pos.x;
			position[1] = pUnit->pos.y;
		}

		lua_pushnumber(pVM, position[0]);
		lua_pushnumber(pVM, position[1]);
		return 2;
	}

	int LGetUnitRotation(LuaVM* pVM)
	{
		float rotation = 0;
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		if (pUnit != NULL)
			rotation = pUnit->rotation;

		lua_pushnumber(pVM, rotation);
		return 1;
	}

	int LGetUnitType(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		UnitType* pUType = NULL;

		if (pUnit != NULL)
		{
			pUType = pUnit->type;
		}

		lua_pushlightuserdata(pVM, static_cast<void*>(pUType));
		return 1;
	}

	int LGetUnitOwner(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Player* pPlayer = NULL;
		int result = 0;

		if (pUnit != NULL)
		{
			result = 1;
			pPlayer = pUnit->owner;
		}

//		lua_pushnumber(pVM, result);
		lua_pushlightuserdata(pVM, (void*) pPlayer);
		return 1;
	}

	int LGetUnitIsMobile(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool isMobile = false;

		if (pUnit != NULL)
			isMobile = pUnit->type->isMobile;

		lua_pushboolean(pVM, isMobile);
		return 1;
	}

	int LGetUnitTypeIsMobile(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		bool isMobile = false;

		if (pUnitType != NULL)
			isMobile = pUnitType->isMobile;

		lua_pushboolean(pVM, isMobile);
		return 1;
	}

	int LGetUnitIsHurtByLight(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool hurtByLight = false;

		if (pUnit != NULL)
			hurtByLight = pUnit->type->hurtByLight;

		lua_pushboolean(pVM, hurtByLight);
		return 1;
	}

	int LGetUnitLightAmount(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		float lightAmount = 0.0;

		if (pUnit != NULL)
			lightAmount = GetLightAmountOnUnit(pUnit);

		lua_pushnumber(pVM, lightAmount);
		return 1;
	}

	int LGetUnitCanAttack(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool canAttack = false;

		if (pUnit != NULL)
			canAttack = pUnit->type->canAttack;

		lua_pushboolean(pVM, canAttack);
		return 1;
	}

	int LGetUnitCanAttackWhileMoving(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool canAttackWhileMoving = false;

		if (pUnit != NULL)
			canAttackWhileMoving = pUnit->type->canAttackWhileMoving;

		lua_pushboolean(pVM, canAttackWhileMoving);
		return 1;
	}

	int LGetUnitLastAttack(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Uint32 time = 0;

		if (pUnit != NULL)
			time = pUnit->lastAttack;
		
		lua_pushnumber(pVM, time);
		return 1;
	}

	int LGetUnitLastAttacked(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Uint32 time = 0;

		if (pUnit != NULL)
			time = pUnit->lastAttacked;
		
		lua_pushnumber(pVM, time);
		return 1;
	}

	int LGetUnitTargetUnit(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Unit* target = NULL;

		if (pUnit != NULL)
			if (pUnit->pMovementData != NULL)
				target = pUnit->pMovementData->action.goal.unit;
		
		lua_pushlightuserdata(pVM, (void*) target);
		return 1;
	}

	int LGetUnitTargetPos(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		Position target;

		target.x = 0;
		target.y = 0;

		if (pUnit != NULL)
			if (pUnit->pMovementData != NULL)
				target = pUnit->pMovementData->action.goal.pos;
		
		lua_pushnumber(pVM, target.x);
		lua_pushnumber(pVM, target.y);
		return 2;
	}

	int LGetNearestUnitInRange(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		RangeType rangeType = (RangeType) lua_tointeger(pVM, 2);
		PlayerState PSBitmask = lua_tointeger(pVM, 3);

		Unit* retUnit = NULL;

		if (pUnit != NULL)
		{
			retUnit = GetNearestUnitInRange(pUnit, rangeType, PSBitmask);
		}
		
		lua_pushlightuserdata(pVM, retUnit);
		return 1;
	}

	int LGetNearestSuitableAndLightedPosition(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player* player = (Player*) lua_touserdata(pVM, 2);
		int x = lua_tointeger(pVM, 3), y = lua_tointeger(pVM, 4);
		bool ret = false;

		if (pUnitType != NULL && player != NULL)
		{
			ret = GetNearestSuitableAndLightedPosition(pUnitType, player, x, y);
		}
		
		lua_pushinteger(pVM, x);
		lua_pushinteger(pVM, y);
		lua_pushboolean(pVM, ret);
		return 3;
	}

	int LGetSuitablePositionForLightTower(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player* player = (Player*) lua_touserdata(pVM, 2);
		int x = lua_tointeger(pVM, 3), y = lua_tointeger(pVM, 4);
		bool ret = false;

		if (pUnitType != NULL && player != NULL)
		{
			ret = GetSuitablePositionForLightTower(pUnitType, player, x, y);
		}
		
		lua_pushinteger(pVM, x);
		lua_pushinteger(pVM, y);
		lua_pushboolean(pVM, ret);
		return 3;
	}

	int LGetUnitProjectileNum(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		int num = 0;

		if (pUnit != NULL)
			num = pUnit->projectiles.size();
		
		lua_pushinteger(pVM, num);
		return 1;
	}

	int LGetUnitProjectileType(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		ProjectileType* projtype = NULL;

		if (pUnit != NULL)
			projtype = pUnit->type->projectileType;
		
		lua_pushlightuserdata(pVM, projtype);
		return 1;
	}

	int LGetUnitProjectile(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		unsigned int num = lua_tointeger(pVM, 2);
		Projectile* proj = NULL;

		if (pUnit != NULL)
			if (num < pUnit->projectiles.size())
				proj = pUnit->projectiles.at(num);
		
		lua_pushlightuserdata(pVM, proj);
		return 1;
	}

	int LGetProjectileType(LuaVM* pVM)
	{
		Projectile* proj = (Projectile*) lua_touserdata(pVM, 1);
		ProjectileType* projtype = NULL;

		if (proj != NULL)
			projtype = proj->type;
		
		lua_pushlightuserdata(pVM, projtype);
		return 1;
	}

	int LGetProjectilePos(LuaVM* pVM)
	{
		Projectile* proj = (Projectile*) lua_touserdata(pVM, 1);
		Vector3D pos;

		if (proj != NULL)
			pos = proj->pos;
		
		lua_pushnumber(pVM, pos.x);
		lua_pushnumber(pVM, pos.y);
		lua_pushnumber(pVM, pos.z);
		return 3;
	}

	int LGetProjectileDirection(LuaVM* pVM)
	{
		Projectile* proj = (Projectile*) lua_touserdata(pVM, 1);
		Vector3D direction;

		if (proj != NULL)
			direction = proj->direction;
		
		lua_pushnumber(pVM, direction.x);
		lua_pushnumber(pVM, direction.y);
		lua_pushnumber(pVM, direction.z);
		return 3;
	}

	int LGetProjectileGoalPos(LuaVM* pVM)
	{
		Projectile* proj = (Projectile*) lua_touserdata(pVM, 1);
		Vector3D goal_pos;

		if (proj != NULL)
			goal_pos = proj->goalPos;
		
		lua_pushnumber(pVM, goal_pos.x);
		lua_pushnumber(pVM, goal_pos.y);
		lua_pushnumber(pVM, goal_pos.z);
		return 3;
	}

	int LGetProjectileGoalUnit(LuaVM* pVM)
	{
		Projectile* proj = (Projectile*) lua_touserdata(pVM, 1);
		Unit* goal_unit = NULL;

		if (proj != NULL)
			goal_unit = proj->goalUnit;
		
		lua_pushlightuserdata(pVM, goal_unit);
		return 1;
	}

	int LGetProjectileTypeStartPos(LuaVM* pVM)
	{
		ProjectileType* projtype = (ProjectileType*) lua_touserdata(pVM, 1);
		Vector3D start_pos;

		if (projtype != NULL)
			start_pos = projtype->startPos;
		
		lua_pushnumber(pVM, start_pos.x);
		lua_pushnumber(pVM, start_pos.y);
		lua_pushnumber(pVM, start_pos.z);
		return 3;
	}

	int LGetProjectileTypeAOE(LuaVM* pVM)
	{
		ProjectileType* projtype = (ProjectileType*) lua_touserdata(pVM, 1);
		float aoe = 0.0;

		if (projtype != NULL)
			aoe = projtype->areaOfEffect;
		
		lua_pushnumber(pVM, aoe);
		return 1;
	}

	int LGetProjectileTypeSpeed(LuaVM* pVM)
	{
		ProjectileType* projtype = (ProjectileType*) lua_touserdata(pVM, 1);
		float speed = 0.0;

		if (projtype != NULL)
			speed = projtype->speed;
		
		lua_pushnumber(pVM, speed);
		return 1;
	}

	int LGetTime(LuaVM* pVM)
	{
		Environment::FourthDimension* pDimension = Environment::FourthDimension::Instance();
		lua_pushnumber(pVM, pDimension->GetCurrentHour());
		return 1;
	}

	int LGetPower(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float power = 0;

		if (player)
			power = GetPower(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetMoney(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float money = 0;

		if (player)
			money = GetMoney(player);

		lua_pushnumber(pVM, money);
		return 1;
	}

	int LGetIncomeAtNoon(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float income = 0;

		if (player)
			income = GetIncomeAtNoon(player);

		lua_pushnumber(pVM, income);
		return 1;
	}
	
	int LGetIncomeAtNight(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float income = 0;

		if (player)
			income = GetIncomeAtNight(player);

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetUnitTypeIncomeAtNoon(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float income;

		income = 0;

		if (pUnitType != NULL)
		{
			
			income += pUnitType->powerIncrement;
			income -= pUnitType->powerUsage + pUnitType->lightPowerUsage + (pUnitType->attackPowerUsage + pUnitType->movePowerUsage + pUnitType->buildPowerUsage) * 0.1;
		}

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetUnitTypeIncomeAtNight(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float income;

		income = 0;

		if (pUnitType != NULL)
		{
			
			if (pUnitType->powerType == POWERTYPE_TWENTYFOURSEVEN)
			{
				income += pUnitType->powerIncrement;
			}
			income -= pUnitType->powerUsage + pUnitType->lightPowerUsage + (pUnitType->attackPowerUsage + pUnitType->movePowerUsage + pUnitType->buildPowerUsage) * 0.1;
		}

		lua_pushnumber(pVM, income);
		return 1;
	}

	int LGetPowerAtDawn(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float power = 0;

		if (player)
			power = GetPowerAtDawn(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LGetPowerAtDusk(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float power = 0;

		if (player)
			power = GetPowerAtDusk(player);

		lua_pushnumber(pVM, power);
		return 1;
	}

	int LSellPower(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		float power = lua_tonumber(pVM, 2);

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

	int LGetDayLength(LuaVM* pVM)
	{
		lua_pushnumber(pVM, GetDayLength());
		return 1;
	}

	int LGetNightLength(LuaVM* pVM)
	{
		lua_pushnumber(pVM, GetNightLength());
		return 1;
	}

	int LGetMapDimensions(LuaVM* pVM)
	{
		lua_pushinteger(pVM, pWorld->width);
		lua_pushinteger(pVM, pWorld->height);
		return 2;
	}

#define CHECK_UNIT_PTR(x) \
	if ((x) == NULL) \
	{ \
		lua_pushboolean(pVM, 0); \
		std::cout << "[LUA SET] Failure: invalid unit - null pointer" << std::endl; \
		return 0; \
	} 

#define LUA_FAILURE(x) \
	{ \
		lua_pushboolean(pVM, 0); \
		std::cout << "[LUA] Failure: " #x << std::endl; \
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

	int LSetUnitHealth(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float health = lua_tonumber(pVM, 2);
		if (health < 0)
			LUA_FAILURE("Health value lower than zero.")

		if (health > pUnit->type->maxHealth)
			health = pUnit->type->maxHealth;

		pUnit->health = health;

		LUA_SUCCESS
	}

	int LSetUnitPower(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float power = lua_tonumber(pVM, 2);
		if (power < 0)
			LUA_FAILURE("Power value lower than zero")

		if (power > pUnit->type->maxPower)
			power = pUnit->type->maxPower;

		pUnit->power = power;

		LUA_SUCCESS
	}

	int LSetUnitAction(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		UnitAction action = (UnitAction) lua_tointeger(pVM, 2);
		
		pUnit->action = action;

		LUA_SUCCESS
	}

	int LSetUnitActionArg(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		pUnit->pMovementData->action.arg = lua_touserdata(pVM, 2);
		
		LUA_SUCCESS
	}

	int LSetUnitPosition(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float position[2] = { lua_tonumber(pVM, 2), lua_tonumber(pVM, 3) };

		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		if (SquaresAreWalkable(pUnit, (int)position[0], (int)position[1]) == false)
			LUA_FAILURE("Designated goal isn't walkable")

		pUnit->pos.x = position[0];
		pUnit->pos.y = position[1];

		LUA_SUCCESS
	}

	int LSetUnitRotation(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float rotation = lua_tonumber(pVM, 2);

		while (rotation >= 360.0f)
			rotation -= 360.0f;
		
		while (rotation < 0.0f)
			rotation += 360.0f;
		
		pUnit->rotation = rotation;

		LUA_SUCCESS
	}

	int LSetUnitType(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		
		CHECK_UNIT_PTR(pUnit)

		UnitType* pUnitType = static_cast<UnitType*>(lua_touserdata(pVM, 2));
		if (pUnitType == NULL)
			LUA_FAILURE("Invalid unit type - null pointer")

		pUnit->type = pUnitType;

		LUA_SUCCESS
	}

	int LSetUnitOwner(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		Player* pPlayer = static_cast<Player*>(lua_touserdata(pVM, 2));
		if (pPlayer == NULL)
			LUA_FAILURE("Invalid player type - null pointer")

		pUnit->owner = pPlayer;

		LUA_SUCCESS
	}

	int LSetUnitTargetUnit(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
//		CHECK_UNIT_PTR(pUnit02)
		// the above line was commented because a target unit of NULL is indeed legal, as it signifies no target.

		pUnit01->pMovementData->action.goal.unit = pUnit02;

		LUA_SUCCESS
	}

	int LSetUnitTargetPos(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		float x = lua_tonumber(pVM, 2);
		float y = lua_tonumber(pVM, 3);

		CHECK_UNIT_PTR(pUnit)

		pUnit->pMovementData->action.goal.pos.x = x;
		pUnit->pMovementData->action.goal.pos.y = y;
		if (pUnit->type->isMobile)
		{
			pUnit->isMoving = true;
		}

		LUA_SUCCESS
	}

	int LSetUnitIsMoving(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));
		bool isMoving = lua_toboolean(pVM, 2);

		CHECK_UNIT_PTR(pUnit)
		
		pUnit->isMoving = isMoving;

		LUA_SUCCESS
	}

	int LClearProjectiles(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		std::vector<Projectile*>::iterator it;
		for (it = pUnit->projectiles.begin(); it != pUnit->projectiles.end(); it++)
			delete *it;
		pUnit->projectiles.clear();

		LUA_SUCCESS
	}

	int LFireProjectileAtLocation(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		Vector3D location;
		float x, y;

		x = lua_tonumber(pVM, 2);
		y = lua_tonumber(pVM, 3);

		if (x < 0 || y < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		location = GetTerrainCoord(x, y);

		location.z += 0.5;

		Projectile* p = CreateProjectile(pUnit->type->projectileType, 
			                             GetTerrainCoord(pUnit->pos.x, pUnit->pos.y), 
		                                 location);

		pUnit->projectiles.push_back(p);

		lua_pushlightuserdata(pVM, static_cast<void*>(p));
		return 1;
	}

	int LFireProjectileAtTarget(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		Projectile* p = CreateProjectile(pUnit01->type->projectileType, 
			                             GetTerrainCoord(pUnit01->pos.x, pUnit01->pos.y), 
		                                 pUnit02);

		pUnit01->projectiles.push_back(p);

		lua_pushlightuserdata(pVM, static_cast<void*>(p));
		return 1;
	}

	int LHandleProjectiles(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		HandleProjectiles(pUnit);

		LUA_SUCCESS
	}

	int LGetUnitTypeBuildCost(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float cost = 0;

		if (pUnitType)
			cost = pUnitType->buildCost;

		lua_pushnumber(pVM, cost);
		return 1;
	}
	
	int LGetUnitTypeResearchCost(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		float cost = 0;

		if (pUnitType)
			cost = pUnitType->researchCost;

		lua_pushnumber(pVM, cost);
		return 1;
	}
	
	int LIsResearched(LuaVM* pVM)
	{
		Player* player = (Player*) lua_touserdata(pVM, 1);
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 2);
		if (pUnitType == NULL)
		{
			lua_pushboolean(pVM, false);
			return 1;
		}
		lua_pushboolean(pVM, pUnitType->isResearched[player->index]);
		return 1;
	}
	
	int LGetResearcher(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		for (vector<UnitType*>::iterator it = pWorld->vUnitTypes.begin(); it != pWorld->vUnitTypes.end(); it++)
		{
			for (vector<UnitType*>::iterator it2 = (*it)->canResearch.begin(); it2 != (*it)->canResearch.end(); it2++)
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
	
	int LGetBuilder(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		for (vector<UnitType*>::iterator it = pWorld->vUnitTypes.begin(); it != pWorld->vUnitTypes.end(); it++)
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
	
	int LSquaresAreLightedAround(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player* player = (Player*) lua_touserdata(pVM, 2);
		int lighted = false;
		if (pUnitType && player)
		{
			lighted = SquaresAreLightedAround(pUnitType, player, lua_tointeger(pVM, 3), lua_tointeger(pVM, 4));
		}
		lua_pushboolean(pVM, lighted);
		return 1;
	}
	
	int LSquaresAreLighted(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		Player* player = (Player*) lua_touserdata(pVM, 2);
		int lighted = false;
		if (pUnitType && player)
		{
			lighted = SquaresAreLighted(pUnitType, player, lua_tointeger(pVM, 3), lua_tointeger(pVM, 4));
		}
		lua_pushboolean(pVM, lighted);
		return 1;
	}
	
	int LIsWithinRangeForBuilding(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		lua_pushboolean(pVM, IsWithinRangeForBuilding(pUnit));
		return 1;
	}

	int LBuild(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		Build(pUnit);

		LUA_SUCCESS
	}

	int LInitiateAttack(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		InitiateAttack(pUnit01, pUnit02);

		LUA_SUCCESS
	}

	int LAttack(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareDamaging(pUnit, lua_tonumber(pVM, 2));
		}
		else
		{
			Attack(pUnit, lua_tonumber(pVM, 2));
		}
		return 0;
	}

	int LCanAttack(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		lua_pushboolean(pVM, CanAttack(pUnit));
		return 1;
	}

	int LCalculateUnitDamage(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		lua_pushnumber(pVM, CalcUnitDamage(pUnit));
		return 1;
	}

	int LCanReach(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		lua_pushboolean(pVM, CanReach(pUnit01, pUnit02));
		return 1;
	}

	int LChangePath(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float position[2] = { lua_tonumber(pVM, 2), 
		                      lua_tonumber(pVM, 3) };

		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		ChangePath(pUnit, position[0], position[1], pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
		LUA_SUCCESS
	}

	void CommandUnit_TargetUnit(Unit* unit, Unit* target, UnitAction action, void* arg)
	{
		Game::AI::action_changes++;
		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareAction(unit, target, target->pos.x, target->pos.y, action, arg);
		}
		else
		{
			unit->action = action;
			unit->action_completeness = 0.0;
			unit->pMovementData->action.goal.unit = target;
			unit->pMovementData->action.arg = arg;
			unit->pMovementData->action.goal.pos = target->pos;
			if (unit->type->isMobile)
			{
				unit->isMoving = true;
			}
		}
		Game::Dimension::ChangePath(unit, target->pos.x, target->pos.y, action, target, arg);
	}

	void CommandUnit_TargetPos(Unit* unit, float x, float y, UnitAction action, void* arg)
	{
		if (!unit->type->isMobile && action == ACTION_GOTO)
			return;
		
		Game::AI::action_changes++;
		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareAction(unit, NULL, x, y, action, arg);
		}
		else
		{
			unit->action = action;
			unit->action_completeness = 0.0;
			unit->pMovementData->action.goal.unit = NULL;
			unit->pMovementData->action.arg = arg;
			unit->pMovementData->action.goal.pos.x = x;
			unit->pMovementData->action.goal.pos.y = y;
			if (unit->type->isMobile)
			{
				unit->isMoving = true;
			}
		}
		Game::Dimension::ChangePath(unit, x, y, action, NULL, arg);
	}

	int LCommandUnit_TargetUnit(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, (UnitAction) lua_tointeger(pVM, 3), lua_touserdata(pVM, 4));

		LUA_SUCCESS
	}

	int LCommandUnit_TargetPos(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		CommandUnit_TargetPos(pUnit, lua_tonumber(pVM, 2), lua_tonumber(pVM, 3), (UnitAction) lua_tointeger(pVM, 4), lua_touserdata(pVM, 5));

		LUA_SUCCESS
	}

	int LCommandGoto(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float position[2] = { lua_tonumber(pVM, 2), 
		                      lua_tonumber(pVM, 3) };

		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_GOTO, NULL);
		LUA_SUCCESS
	}

	int LCommandMoveAttack(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float position[2] = { lua_tonumber(pVM, 2), 
		                      lua_tonumber(pVM, 3) };

		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_MOVE_ATTACK, NULL);
		LUA_SUCCESS
	}

	int LCommandBuild(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		float position[2] = { lua_tonumber(pVM, 2), 
		                      lua_tonumber(pVM, 3) };

		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 4);

		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		CommandUnit_TargetPos(pUnit, position[0], position[1], ACTION_BUILD, pUnitType);
		LUA_SUCCESS
	}

	int LCommandResearch(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 2);

		CommandUnit_TargetPos(pUnit, 0, 0, ACTION_RESEARCH, pUnitType);
		LUA_SUCCESS
	}

	int LCommandFollow(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_FOLLOW, NULL);
		LUA_SUCCESS
	}

	int LCommandAttack(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_ATTACK, NULL);
		LUA_SUCCESS
	}

	int LCommandCollect(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_COLLECT, NULL);
		LUA_SUCCESS
	}

	int LCommandRepair(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_BUILD, NULL);
		LUA_SUCCESS
	}

	int LCommandMoveAttackUnit(LuaVM* pVM)
	{
		Unit* pUnit01 = _GetUnit(lua_touserdata(pVM, 1));
		Unit* pUnit02 = _GetUnit(lua_touserdata(pVM, 2));

		CHECK_UNIT_PTR(pUnit01)
		CHECK_UNIT_PTR(pUnit02)

		CommandUnit_TargetUnit(pUnit01, pUnit02, ACTION_MOVE_ATTACK_UNIT, NULL);
		LUA_SUCCESS
	}

	int LMove(LuaVM* pVM)
	{
		Unit* pUnit = _GetUnit(lua_touserdata(pVM, 1));

		CHECK_UNIT_PTR(pUnit)

		MoveUnit(pUnit);
		LUA_SUCCESS
	}

	int LClearAllActions(LuaVM* pVM)
	{
		LUA_FAILURE("Not yet implemented")
	}

	int LCreateUnit(LuaVM* pVM)
	{
		UnitType* pUnitType = static_cast<UnitType*>(lua_touserdata(pVM, 1));
		if (pUnitType == NULL)
			LUA_FAILURE("Invalid unit type - null pointer")

		Player* pOwner = static_cast<Player*>(lua_touserdata(pVM, 2));
		if (pOwner == NULL)
			LUA_FAILURE("Invalid owner - null pointer")

		float position[2] = { lua_tonumber(pVM, 3), lua_tonumber(pVM, 4) };
		if (position[0] < 0 || position[1] < 0)
			LUA_FAILURE("Invalid position, x or y values lower than zero")

		if (SquaresAreWalkable_AllKnowing(pUnitType, (int)position[0], (int)position[1]) == false)
		{
			LUA_FAILURE("Designated goal isn't walkable (or is preoccupied by another unit).")
		}

		float rotation = rand()/((double)RAND_MAX + 1) * 360;

		if (lua_isnumber(pVM, 5))
		{
			rotation = lua_tonumber(pVM, 5);

			while (rotation >= 360.0f)
				rotation -= 360.0f;

			while (rotation < 0.0f)
				rotation += 360.0f;

		}

		if (Game::Networking::isNetworked)
		{
			Game::Networking::PrepareCreation(pUnitType, pOwner, position[0], position[1], rotation);
		}
		else
		{
			Unit* pUnit = CreateUnit(pUnitType, pOwner, position[0], position[1]);
			if (pUnit)
				pUnit->rotation = rotation;
		}

		return 0;
	}

	int LCanCreateUnitAt(LuaVM* pVM)
	{
		UnitType* pUnitType = static_cast<UnitType*>(lua_touserdata(pVM, 1));
		if (pUnitType == NULL)
			LUA_FAIL

		Player* pOwner = static_cast<Player*>(lua_touserdata(pVM, 2));
		if (pOwner == NULL)
			LUA_FAIL

		float position[2] = { lua_tonumber(pVM, 3), lua_tonumber(pVM, 4) };
		if (position[0] < 0 || position[1] < 0)
			LUA_FAIL

		if (SquaresAreWalkable_AllKnowing(pUnitType, (int)position[0], (int)position[1]) == false)
			LUA_FAIL

		LUA_SUCCESS
	}

	int LGetUnitTypeFromString(LuaVM* pVM)
	{
		const char *sz_name = lua_tostring(pVM, 1);
		const string name = sz_name;
		
		lua_pushlightuserdata(pVM, unitTypeMap[name]);
		return 1;
	}

	int LGetPlayerByIndex(LuaVM* pVM)
	{
		const Uint32 index = static_cast<const Uint32>(lua_tonumber(pVM, 1));

		if (index > pWorld->vPlayers.size())
			LUA_FAILURE("Player index out of vector bounds")

		lua_pushlightuserdata(pVM, static_cast<void*>(pWorld->vPlayers.at(index)));
		return 1;
	}

	int LGetPlayerByName(LuaVM* pVM)
	{
		const char* name = lua_tostring(pVM, 1);

		if (sizeof(name)/sizeof(char) <= 0)
			LUA_FAILURE("Player name too short")

		std::vector<Player*>::const_iterator it;
		Player* pPlayer = NULL;
		for (it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
		{
			if (strcmp((*it)->name, name) == 0)
			{
				pPlayer = *it;
				break;
			}
		}

		lua_pushlightuserdata(pVM, static_cast<void*>(pPlayer));
		return 1;
	}
	
	int LInitPlayers(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LInitPlayers: Invalid arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LInitPlayers: First argument no number")
		}
		
		int value = (int)lua_tonumber(pVM, 1);
		if (value < 1 || value > 4)
		{
			LUA_FAILURE("LInitPlayers: Invalid amount of players. Reasonable amounts: 1-4")
		}
		
		Game::Dimension::InitPlayers(value);
		LUA_SUCCESS
	}

	int LIsNonNull(LuaVM* pVM)
	{
		void* val = lua_touserdata(pVM, 1);
		lua_pushboolean(pVM, val != NULL);
		return 1;
	}

	int LNull(LuaVM* pVM)
	{
		lua_pushlightuserdata(pVM, NULL);
		return 1;
	}

	int LGetUnitTypeName(LuaVM* pVM)
	{
		UnitType* pUnitType = (UnitType*) lua_touserdata(pVM, 1);
		char* name = (char*) "";
		if (pUnitType)
			name = pUnitType->name;

		lua_pushstring(pVM, name);
		return 1;
	}

	int LOutput(LuaVM* pVM)
	{
		const char *str = lua_tostring(pVM, 1);
		cout << str;
		return 0;
	}
	
	int LLoadHeightmap(LuaVM* pVM)
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
			LUA_FAILURE("LLoadHeightmap: Error during world load")
		}
		
		LUA_FAILURE("LLoadHeightmap: Should never happen!") // << huh, should never happen
	}
	
	int LSetHeightmapModifier(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetHeightmapModifier: Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetHeightmapModifier: First argument no number")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		
		if (value < 0)
		{
			LUA_FAILURE("LSetHeightmapModifier: Invalid argument, below zero")
		}
		
		Game::Dimension::terrainHeight = value;
		LUA_SUCCESS
	}
	
	int LSetMaximumBuildingAltitude(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetMaximumBuildingAltitude: Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetMaxiumBuildingAltitude: Firsta rgument no number")
		}
		
		GLfloat value = (GLfloat)lua_tonumber(pVM, 1);
		
		if (value > 0 || value <= -1.0f)
		{
			LUA_FAILURE("LSetMaximumBuildingAltitude: Invalid altitude. Using preset.")
		}
		
		Game::Dimension::unitBuildingMaximumAltitude = value;
		
		LUA_SUCCESS
	}
	
	int LLoadTerrainTexture(LuaVM* pVM)
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

	int LSetTerrainAmbientDiffuse(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(6)

		if (!lua_isboolean(pVM, 1))
		{
			LUA_FAILURE("LSetTerrainAmbientDiffuse: First argument no boolean")
		}

		if (!lua_isboolean(pVM, 2))
		{
			LUA_FAILURE("LSetTerrainAmbientDiffuse: Second argument no boolean")
		}

		for (int i = 3; i <= 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetTerrainAmbientDiffuse: Invalid arguments. No numeric values.")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetTerrainAmbientDiffuse: Invalid colour range. Valid range: 0 - 1.0")
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

	int LSetTerrainSpecular(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetTerrainSpecular: Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetTerrainSpecular: Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::terrainMaterialSpecular[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetTerrainEmission(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetTerrainEmission: Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetTerrainEmission: Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::terrainMaterialEmission[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetTerrainShininess(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetTerrainShininess: Incorrect argument count")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetTerrainShininess: First argument no number")
		}

		Game::Dimension::terrainMaterialShininess = (GLfloat)lua_tonumber(pVM, 1);

		LUA_SUCCESS
	}
	
	int LSetWaterLevel(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetWaterLevel: Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetWaterLevel: First argument no string")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		Game::Dimension::waterLevel = value;
		
		LUA_SUCCESS
	}
	
	int LSetWaterHeight(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetWaterHeight: Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetWaterHeight: First argument no string")
		}
		
		float value = (float)lua_tonumber(pVM, 1);
		Game::Dimension::waterHeight = value;
		
		LUA_SUCCESS
	}

	int LSetWaterAmbientDiffuse(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(6)

		if (!lua_isboolean(pVM, 1))
		{
			LUA_FAILURE("LSetWaterAmbientDiffuse: First argument no boolean")
		}

		if (!lua_isboolean(pVM, 2))
		{
			LUA_FAILURE("LSetWaterAmbientDiffuse: Second argument no boolean")
		}

		for (int i = 3; i <= 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetWaterAmbientDiffuse: Invalid arguments. No numeric values.")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetWaterAmbientDiffuse: Invalid colour range. Valid range: 0 - 1.0")
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

	int LSetWaterSpecular(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetWaterSpecular: Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetWaterSpecular: Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::waterMaterialSpecular[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetWaterEmission(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(4)

		for (int i = 1; i <= 4; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetWaterEmission: Invalid arguments. No numeric values")
			}

			GLfloat tmp = (GLfloat)lua_tonumber(pVM, i);

			if (tmp < 0 || tmp > 1.0f)
			{
				LUA_FAILURE("LSetWaterEmission: Invalid colour range. Valid range: 0 - 1.0")
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			Game::Dimension::waterMaterialEmission[i-1] = (GLfloat)lua_tonumber(pVM, i);
		}

		LUA_SUCCESS
	}

	int LSetWaterShininess(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetWaterShininess: Incorrect argument count")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetWaterShininess: First argument no number")
		}

		Game::Dimension::waterMaterialShininess = (GLfloat)lua_tonumber(pVM, 1);

		LUA_SUCCESS
	}
	
	int LSetWaterColor(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(3)
		
		GLfloat values[] = { 0, 0, 0 };
		int i = 0;
		for (i = 1; i < 4; i++ )
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetWaterColor: non-numeric values was provided")
			}
			
			values[i - 1] = (GLfloat)lua_tonumber(pVM, i);
			
			if (values[i - 1] > 1.0f || values[ i - 1 ] < 0)
			{
				LUA_FAILURE("LSetWaterColor: invalid value (too high/low)")
			}
		}
		
		for (i = 0; i < 3; i++)
			Game::Dimension::waterColor[i] = values[i];
		
		LUA_SUCCESS
	}
	
	int LPrepareGUI(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LPrepareGUI: Incorrect arguments")
		}
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LPrepareGUI: First argument no pointer")
		}
		
		SDL_Surface* p = (SDL_Surface*) lua_touserdata(pVM, 1);
		
		if (p == NULL)
		{
			LUA_FAILURE("LPrepareGUI: Null pointer")
		}
		
		Game::Rules::GameWindow::Instance()->InitGUI(p);
		
		p = NULL;
		
		LUA_SUCCESS
	}
	
	int LFreeSurface(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LFreeSurface: Incorrect arguments")
		}
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LFreeSurface: First argument no pointer")
		}
		
		SDL_Surface* p = (SDL_Surface*) lua_touserdata(pVM, 1);
		
		if (p != NULL)
		{
			SDL_FreeSurface(p);
		}
		
		p = NULL;
		
		LUA_SUCCESS
	}
	
	int LAllocEnvironmentalCondition(LuaVM* pVM)
	{
		Game::Dimension::Environment::EnvironmentalCondition* env = new
			Game::Dimension::Environment::EnvironmentalCondition;
			
		lua_pushlightuserdata(pVM, (void*)env);	
		
		return 1;
	}
		
	#define GET_ENVIRONMENTAL_CONDITION(index, p)\
		Game::Dimension::Environment::EnvironmentalCondition* (p) = \
		(Game::Dimension::Environment::EnvironmentalCondition*) lua_touserdata(pVM, (index) );
	
	int LSetHours(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(3)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetHours: First argument no pointer")
		}
		
		if (!lua_isnumber(pVM, 2) ||
			!lua_isnumber(pVM, 3))
		{
			LUA_FAILURE("LSetHours: Second or third argument no number")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetHours: Null pointer")
		}
		
		p->hourBegin = lua_tonumber(pVM, 2);
		p->hourEnd   = lua_tonumber(pVM, 3);
		
		LUA_SUCCESS
	}
	
	int LSetType(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetType: First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("LSetType: Incorrect arguments")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetType: Null pointer")
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
			LUA_FAILURE("LSetType: Invalid environmental condition type")
		}
		
		LUA_SUCCESS
	}
	
	int LSetMusicList(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetMusicList: First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("LSetMusicList: Second argument no string")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetMusicList: Null pointer")
		}
		
		p->musicListTag = lua_tostring(pVM, 2);
		
		LUA_SUCCESS
	}
	
	int LSetSkybox(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetSkybox: First argument no pointer")
		}
		
		if (!lua_isstring(pVM, 2))
		{
			LUA_FAILURE("LSetSkybox: Second argument no string")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetSkybox: Null pointer")
		}
		
		int idx = Game::Dimension::Environment::FourthDimension::Instance()->GetSkybox(lua_tostring(pVM, 2));
		
		if (idx == -1)
		{
			LUA_FAILURE("LSetSkybox: Invalid skybox tag")
		}
		
		p->skybox = idx;
		
		LUA_SUCCESS
	}
	
	int LSetSunPos(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetSunPos: First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetSunPos: Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetSunPos: Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->sunPos[i] = lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LSetDiffuse(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetDiffuse: First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetDiffuse: Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetDiffuse: Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->diffuse[i] = lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS	
	}
	
	int LSetAmbient(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetAmbient: First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetAmbient: Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetAmbient: Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->ambient[i] = lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LSetFogParams(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(4)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetFogParams: First argument no pointer")
		}
		
		for (int i = 2; i < 5; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetFogParams: Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetFogParams: Null pointer")
		}
		
		p->fogBegin     = lua_tonumber(pVM, 2);
		p->fogEnd       = lua_tonumber(pVM, 3);
		p->fogIntensity = lua_tonumber(pVM, 4);
		
		LUA_SUCCESS
	}
	
	int LSetFogColor(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(5)
		
		if (!lua_isuserdata(pVM, 1))
		{
			LUA_FAILURE("LSetFogColor: First argument no pointer")
		}
		
		for (int i = 2; i < 6; i++)
		{
			if (!lua_isnumber(pVM, i))
			{
				LUA_FAILURE("LSetFogColor: Invalid arguments")
			}
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LSetFogColor: Null pointer")
		}
		
		for (int i = 0; i < 4; i++)
		{
			p->fogColor[i] = lua_tonumber(pVM, i + 2);
		}
		
		LUA_SUCCESS
	}
	
	int LAddEnvironmentalCondition(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LAddEnvironmentalCondition: Incorrect arguments")
		}
		
		GET_ENVIRONMENTAL_CONDITION(1, p)
		
		if (p == NULL)
		{
			LUA_FAILURE("LAddEnvironmentalCondition: Null pointer")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->AddCondition(p);
		
		LUA_SUCCESS
	}
	
	int LValidateEnvironmentalConditions(LuaVM* pVM)
	{
		if (!Game::Dimension::Environment::FourthDimension::Instance()->ValidateConditions())
			LUA_FAIL
			
		LUA_SUCCESS
	}
	
	int LInitSkybox(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isnumber(pVM, 1) ||
		    !lua_isnumber(pVM, 2))
		{
			LUA_FAILURE("LInitSkybox: Invalid arguments")
		}
	
		int detail  = (int)lua_tonumber(pVM, 1),
		    hdetail = (int)lua_tonumber(pVM, 2);
			
		if (detail > 100 || hdetail > 50)
		{
			LUA_FAILURE("LInitSkybox: Too detailed skybox")
		}
	
		Game::Dimension::Environment::FourthDimension::Instance()->InitSkyBox(detail, hdetail);
	
		LUA_SUCCESS
	}
	
	int LSetDayLength(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetDayLength: Invalid arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetDayLength: First argument no number")
		}
		
		double length = lua_tonumber(pVM, 1);
		
		if (length < 0 || length > 1000)
		{
			LUA_FAILURE("LSetDayLength: Invalid value")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->SetDayLength((int)length);
		
		LUA_SUCCESS
	}
	
	int LSetHourLength(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetHourLength: Invalid arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetHourLength: First argument no number")
		}
		
		double length = lua_tonumber(pVM, 1);
		
		if (length < 0 || length > 1000)
		{
			LUA_FAILURE("LSetHourLength: Invalid value")
		}
		
		Game::Dimension::Environment::FourthDimension::Instance()->SetHourLength((int)length);
		
		LUA_SUCCESS
	}
	
	int LSetCurrentHour(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LSetCurrentHour: Incorrect arguments")
		}
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LSetCurrentHour: Incorrect arguments")
		}
		
		int n = (int)lua_tonumber(pVM, 1);
		Game::Dimension::Environment::FourthDimension* p = 
		    Game::Dimension::Environment::FourthDimension::Instance();
		
		if (n < 0 || n > p->GetDayLength())
		{
			LUA_FAILURE("LSetCurrentHour: Invalid value")
		}
	
		p->SetCurrentHour(n);
		
		LUA_SUCCESS
	}

	int LFocusCameraOnUnit(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LFocusCameraOnUnit: Incorrect arguments")
		}

		if (!lua_islightuserdata(pVM, 1))
		{
			LUA_FAILURE("LFocusCameraOnUnit: First argument no unit pointer")
		}

		Game::Dimension::Unit* unit = (Game::Dimension::Unit*)lua_touserdata(pVM, 1);

		if (unit == NULL)
		{
			LUA_FAILURE("LFocusCameraOnUnit: Null pointer")
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

	int LFocusCameraOnCoord(LuaVM* pVM)
	{
		ASSERT_PARAM_COUNT(2)
		
		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LFocusCameraOnCoord: First argument no number")
		}

		if (!lua_isnumber(pVM, 2))
		{
			LUA_FAILURE("LFocusCameraOnCoord: Second argument no number")
		}

		GLfloat x = (GLfloat)lua_tonumber(pVM, 1);
		GLfloat y = (GLfloat)lua_tonumber(pVM, 2);

		if (x < 0 || x > Game::Dimension::pWorld->width)
		{
			LUA_FAILURE("LFocusCameraOnCoord: Invalid X - value")
		}

		if (y < 0 || y > Game::Dimension::pWorld->height)
		{
			LUA_FAILURE("LFocusCameraOnCoord: Invalid Y - value")
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

	int LRotateCamera(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LRotateCamera: Incorrect arguments")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LRotateCamera: First argumetn no number")
		}

		GLfloat rotation = (GLfloat)lua_tonumber(pVM, 1);

		if (rotation <= -360.0f || rotation >= 360.0f || rotation == 0)
		{
			LUA_FAILURE("LRotateCamera: Invalid rotation value. -360 < r < 360  r != 1")
		}

		Game::Rules::GameWindow::Instance()->GetCamera()->Rotate(rotation);

		LUA_SUCCESS
	}

	int LZoomCamera(LuaVM* pVM)
	{
		if (lua_isnil(pVM, 1))
		{
			LUA_FAILURE("LZoomCamera: Incorrect arguments")
		}

		if (!lua_isnumber(pVM, 1))
		{
			LUA_FAILURE("LZoomCamera: First argument no number")
		}

		GLfloat zoom = (GLfloat)lua_tonumber(pVM, 1);

		Game::Rules::GameWindow::Instance()->GetCamera()->Zoom(zoom);

		LUA_SUCCESS
	}

	int LGetLUAScript(LuaVM* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		if (!filename)
		{
			LUA_FAILURE("LGetLUAScript: Invalid String")
		}
		std::string filepath = Utilities::GetDataFile("scripts/" + (std::string) filename);

		if (!filepath.length())
		{
			LUA_FAILURE("LGetLUAScript: Script not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetDataFile(LuaVM* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		std::string filepath = Utilities::GetDataFile(filename);

		if (!filepath.length())
		{
			LUA_FAILURE("LGetDataFile: File not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetConfigFile(LuaVM* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		std::string filepath = Utilities::GetConfigFile(filename);

		if (!filepath.length())
		{
			LUA_FAILURE("LGetConfigFile: File not found")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	int LGetWritableConfigFile(LuaVM* pVM)
	{
		const char* filename = lua_tostring(pVM, 1);
		std::string filepath = Utilities::GetWritableConfigFile(filename);

		if (!filepath.length())
		{
			LUA_FAILURE("LGetWritableConfigFile: Could not find a directory to write in")
		}

		lua_pushstring(pVM, filepath.c_str());
		return 1;
	}

	void Init(void)
	{
		Scripting::LuaVirtualMachine* const pVM = Scripting::LuaVirtualMachine::Instance();

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
		pVM->RegisterFunction("GetUnitIsHurtByLight", LGetUnitIsHurtByLight);
		pVM->RegisterFunction("GetUnitLightAmount", LGetUnitLightAmount);
		pVM->RegisterFunction("GetUnitCanAttack", LGetUnitCanAttack);
		pVM->RegisterFunction("GetUnitCanAttackWhileMoving", LGetUnitCanAttackWhileMoving);
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
		pVM->RegisterFunction("SetUnitAction", LSetUnitAction);
		pVM->RegisterFunction("SetUnitActionArg", LSetUnitActionArg);
		pVM->RegisterFunction("SetUnitPosition", LSetUnitPosition);
		pVM->RegisterFunction("SetUnitRotation", LSetUnitRotation);
		pVM->RegisterFunction("SetUnitType", LSetUnitType);
		pVM->RegisterFunction("SetUnitOwner", LSetUnitOwner);
		pVM->RegisterFunction("SetUnitTargetUnit", LSetUnitTargetUnit);
		pVM->RegisterFunction("SetUnitTargetPos", LSetUnitTargetPos);
		pVM->RegisterFunction("SetUnitIsMoving", LSetUnitIsMoving);

		pVM->RegisterFunction("ClearProjectiles", LClearProjectiles);
		pVM->RegisterFunction("FireProjectileAtLocation", LFireProjectileAtLocation);
		pVM->RegisterFunction("FireProjectileAtTarget", LFireProjectileAtTarget);
		pVM->RegisterFunction("HandleProjectiles", LHandleProjectiles);
		
		pVM->RegisterFunction("IsResearched", LIsResearched);
		pVM->RegisterFunction("GetResearcher", LGetResearcher);
		pVM->RegisterFunction("GetBuilder", LGetBuilder);

		pVM->RegisterFunction("Build", LBuild);
		pVM->RegisterFunction("IsWithinRangeForBuilding", LIsWithinRangeForBuilding);

		pVM->RegisterFunction("InitiateAttack", LInitiateAttack);
		pVM->RegisterFunction("Attack", LAttack);
		pVM->RegisterFunction("CanAttack", LCanAttack);
		pVM->RegisterFunction("CalculateUnitDamage", LCalculateUnitDamage);
		pVM->RegisterFunction("CanReach", LCanReach);
		
		pVM->RegisterFunction("ChangePath", LChangePath);
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
		
		pVM->RegisterFunction("Move", LMove);
		pVM->RegisterFunction("ClearAllActions", LClearAllActions);

		pVM->RegisterFunction("CreateUnit", LCreateUnit);
		pVM->RegisterFunction("CanCreateUnitAt", LCanCreateUnitAt);

		pVM->RegisterFunction("GetUnitTypeIncomeAtNoon", LGetUnitTypeIncomeAtNoon);
		pVM->RegisterFunction("GetUnitTypeIncomeAtNight", LGetUnitTypeIncomeAtNight);
		
		pVM->RegisterFunction("GetUnitTypeBuildCost", LGetUnitTypeBuildCost);
		pVM->RegisterFunction("GetUnitTypeResearchCost", LGetUnitTypeResearchCost);
		pVM->RegisterFunction("GetUnitTypeFromString", LGetUnitTypeFromString);
		pVM->RegisterFunction("GetUnitTypeIsMobile", LGetUnitTypeIsMobile);

		pVM->RegisterFunction("GetPlayerByIndex", LGetPlayerByIndex);
		pVM->RegisterFunction("GetPlayerByName", LGetPlayerByName);
		pVM->RegisterFunction("InitPlayers", LInitPlayers);
		
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
	}
}
