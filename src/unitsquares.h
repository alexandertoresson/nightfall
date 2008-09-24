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
#ifndef __UNITSQUARES_H__
#define __UNITSQUARES_H__

#ifdef DEBUG_DEP
#warning "unitsquares.h"
#endif

#include "unitsquares-pre.h"

#include "unit-pre.h"
#include "dimension-pre.h"
#include "sdlheader.h"
#include <vector>

namespace Game
{
	namespace Dimension
	{
		extern std::vector<gc_ptr<Unit> >*** unitsInBigSquares;
		extern int PositionSearch_NumStepsTaken;
		extern GLfloat unitBuildingMaximumAltitude;
		extern int bigSquareHeight, bigSquareWidth;
		extern int bigSquareRightShift;
		
		enum RangeType
		{
			RANGE_ATTACK,
			RANGE_SIGHT
		};

		const int SIW_DEFAULT = 0,
		          SIW_ALLKNOWING = 1,
		          SIW_IGNORE_MOVING = 2,
/*		          SIW_CONSIDER_WAITING = 4,
		          SIW_CONSIDER_PUSHED = 8,*/
		          SIW_IGNORE_OWN_MOBILE_UNITS = 16;

		bool IsWithinRangeForBuilding(const gc_ptr<Unit>& unit);
		
		void GetTypeUpperLeftCorner(const gc_ptr<UnitType>& type, int mx, int my, int& lx, int& uy);
		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, int& lx, int& uy);
		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, float mx, float my, int& lx, int& uy);
		void GetUnitUpperLeftCorner(const gc_ptr<Unit>& unit, int mx, int my, int& lx, int& uy);
		bool DoesNotBlock(const gc_ptr<Unit>& unit, const gc_ptr<UnitType>& build_type, int build_x, int build_y, int x, int y);

		void NearestSquareFromBuildingPlace(const gc_ptr<Unit>& unit, const gc_ptr<UnitType>& build_type, int build_x, int build_y, int &x, int &y);
		void GetNearestUnoccupiedPosition(const gc_ptr<UnitType>& type, int& x, int& y);
		bool IsSuitableForBuilding(const gc_ptr<UnitType>& type, int build_x, int build_y);

		void Complete(const gc_ptr<Unit>& unit);
		void Incomplete(const gc_ptr<Unit>& unit);

		bool MovementTypeCanWalkOnSquare(MovementType mType, int x, int y);
		inline bool MovementTypeCanWalkOnSquare_UnGuarded(MovementType mType, int x, int y);
		bool MovementTypeCanWalkOnSquare_Pathfinding(MovementType mType, int size, int pos_x, int pos_y);

		bool SquareIsWalkable(const gc_ptr<UnitType>& type, int x, int y, int flags);
		bool SquaresAreWalkable(const gc_ptr<UnitType>& type, int x, int y, int flags);
		inline bool SquareIsWalkable(const gc_ptr<UnitType>& type, int x, int y);
		bool SquaresAreWalkable(const gc_ptr<UnitType>& type, int x, int y);

		bool SquareIsLighted(const gc_ptr<Player>& player, int x, int y);
		bool SquareIsVisible(const gc_ptr<Player>& player, int x, int y);
		inline bool SquareIsVisible_UnGuarded(const gc_ptr<Player>& player, int x, int y);
		bool SquaresAreLighted(const gc_ptr<UnitType>& type, int x, int y);
		bool SquaresAreLightedAround(const gc_ptr<UnitType>& type, int x, int y);
		
		bool GetNearestSuitableAndLightedPosition(const gc_ptr<UnitType>& type, int& x, int& y);
		bool GetSuitablePositionForLightTower(const gc_ptr<UnitType>& type, int& x, int& y, bool needLighted);
		
		gc_ptr<Unit> GetNearestUnitInRange(const gc_ptr<Unit>& unit, RangeType rangeType, PlayerState state);
		bool UnitIsVisible(const gc_ptr<Unit>& unit, const gc_ptr<Player>& player);
		
		bool UpdateAssociatedSquares(const gc_ptr<Unit>& unit, int new_x, int new_y, int old_x, int old_y);
		bool SetAssociatedSquares(const gc_ptr<Unit>& unit, int new_x, int new_y);
		void DeleteAssociatedSquares(const gc_ptr<Unit>& unit, int old_x, int old_y);
		
		float GetLightAmountOnUnit(const gc_ptr<Unit>& unit);
		void UpdateLightedSquares(const gc_ptr<Unit>& unit, int x, int y, int operation);
		void SetLightState(const gc_ptr<Unit>& unit, LightState lightState);
		
		bool CanSee(const gc_ptr<Unit>& attacker, const gc_ptr<Unit>& target);

		bool UnitIsRendered(const gc_ptr<Unit>& unit, const gc_ptr<Player>& player);

		void CheckPrecomputedArrays(const gc_ptr<UnitType>& type);

		void InitUnitSquares();
		
		int GetTraversalTime(const gc_ptr<Unit>& unit, int x, int y, int dx, int dy);
		int GetTraversalTimeAdjusted(const gc_ptr<Unit>& unit, int x, int y, int dx, int dy);
		
		bool SquareIsGoal(const gc_ptr<Unit>& unit, int x, int y, bool use_internal = false);
		
		bool SquareIsWalkable(const gc_ptr<Unit>& unit, int x, int y, int flags);
		bool SquaresAreWalkable(const gc_ptr<Unit>& unit, int x, int y, int flags);
		inline bool SquareIsWalkable(const gc_ptr<Unit>& unit, int x, int y);
		bool SquaresAreWalkable(const gc_ptr<Unit>& unit, int x, int y);
		bool SquaresAreWalkable(const gc_ptr<UnitType>& type, int x, int y, int flags);

		void RemoveUnitFromBigSquare(const gc_ptr<Unit>& unit);
	}
}

#ifdef DEBUG_DEP
#warning "unitsquares.h-end"
#endif

#endif
