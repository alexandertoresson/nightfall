#ifndef __UNIT_H_PRE__
#define __UNIT_H_PRE__

#ifdef DEBUG_DEP
#warning "unit.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		enum MovementType
		{
			MOVEMENT_HUMAN = 0,
			MOVEMENT_SMALLVEHICLE,
			MOVEMENT_MEDIUMVEHICLE,
			MOVEMENT_LARGEVEHICLE,
			MOVEMENT_BUILDING,
			MOVEMENT_AIRBORNE,
			MOVEMENT_SEA,
			MOVEMENT_TYPES_NUM
		};

		enum PowerType
		{
			POWERTYPE_DAYLIGHT,
			POWERTYPE_TWENTYFOURSEVEN
		};

		enum LightState
		{
			LIGHT_OFF,
			LIGHT_AUTO,
			LIGHT_ON
		};

		struct Unit;
		struct ActionQueueItem;
		struct UnitType;
		struct Model;
		struct ProjectileType;
		struct Projectile;
		struct MorphAnim;
		struct TransformAnim;
		struct TransData;
		struct Animation;
		
		gc_ptr<Unit> GetUnitByID(unsigned id);
		bool IsDisplayedUnitPointer(const gc_ptr<Unit>& unit); // May be calle from synced threads

		bool MoveUnit(const gc_ptr<Unit>& unit);
		
		extern int** numUnitsPerAreaMap;
		extern int numSentCommands;
	}
}

#endif

