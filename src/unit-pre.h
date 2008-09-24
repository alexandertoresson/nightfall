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
		
		gc_ptr<Unit> GetUnitByID(int id);
		bool IsDisplayedUnitPointer(const gc_ptr<Unit>& unit); // May be calle from synced threads

		bool MoveUnit(const gc_ptr<Unit>& unit);
		
		extern int** numUnitsPerAreaMap;
		extern int numSentCommands;
	}
}

#endif

