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
		extern std::vector<Unit*>*** unitsInBigSquares;
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
		          SIW_CONSIDER_WAITING = 4,
		          SIW_CONSIDER_PUSHED = 8,
		          SIW_IGNORE_OWN_MOBILE_UNITS = 16;

		bool IsWithinRangeForBuilding(Unit* unit);
		
		void GetTypeUpperLeftCorner(UnitType* type, int mx, int my, int& lx, int& uy);
		void GetUnitUpperLeftCorner(Unit* unit, int& lx, int& uy);
		void GetUnitUpperLeftCorner(Unit* unit, float mx, float my, int& lx, int& uy);
		void GetUnitUpperLeftCorner(Unit* unit, int mx, int my, int& lx, int& uy);
		bool DoesNotBlock(Unit* unit, UnitType* build_type, int build_x, int build_y, int x, int y);

		void NearestSquareFromBuildingPlace(Unit* unit, UnitType* build_type, int build_x, int build_y, int &x, int &y);
		void GetNearestUnoccupiedPosition(UnitType* type, int& x, int& y);
		bool IsSuitableForBuilding(UnitType* type, Player* player, int build_x, int build_y);

		void Complete(Unit* unit);
		void Incomplete(Unit* unit);

		bool MovementTypeCanWalkOnSquare(MovementType mType, int x, int y);
		inline bool MovementTypeCanWalkOnSquare_UnGuarded(MovementType mType, int x, int y);
		bool MovementTypeCanWalkOnSquare_Pathfinding(MovementType mType, int size, int pos_x, int pos_y);

		bool SquareIsWalkable(UnitType *type, Player *player, int x, int y, int flags);
		bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y, int flags);
		inline bool SquareIsWalkable(UnitType *type, Player *player, int x, int y);
		bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y);

		bool SquareIsLighted(Player *player, int x, int y);
		bool SquareIsVisible(Player *player, int x, int y);
		inline bool SquareIsVisible_UnGuarded(Player *player, int x, int y);
		bool SquaresAreLighted(UnitType *type, Player *player, int x, int y);
		bool SquaresAreLightedAround(UnitType *type, Player *player, int x, int y);
		
		bool GetNearestSuitableAndLightedPosition(UnitType* type, Player* player, int& x, int& y);
		bool GetSuitablePositionForLightTower(UnitType* type, Player* player, int& x, int& y);
		
		Unit* GetNearestUnitInRange(Unit* unit, RangeType rangeType, PlayerState state);
		bool UnitIsVisible(Unit *unit, Player *player);
		
		bool UpdateAssociatedSquares(Unit* unit, int new_x, int new_y, int old_x, int old_y);
		bool SetAssociatedSquares(Unit* unit, int new_x, int new_y);
		void DeleteAssociatedSquares(Unit* unit, int old_x, int old_y);
		
		float GetLightAmountOnUnit(Unit* unit);
		void UpdateLightedSquares(Unit* unit, int x, int y, int operation);
		void SetLightState(Unit* unit, LightState lightState);
		
		bool CanSee(Unit* attacker, Unit* target);

		bool UnitIsRendered(Unit *unit, Player *player);

		void CheckPrecomputedArrays(UnitType* type);

		void InitUnitSquares();
		
		int GetTraversalTime(Unit *unit, int x, int y, int dx, int dy);
		int GetTraversalTimeAdjusted(Unit *unit, int x, int y, int dx, int dy);
		
		bool SquareIsGoal(Unit *unit, int x, int y, bool use_internal = false);
		
		bool SquareIsWalkable(Unit *unit, int x, int y, int flags);
		bool SquaresAreWalkable(Unit *unit, int x, int y, int flags);
		inline bool SquareIsWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(UnitType *type, int x, int y, int flags);

		void RemoveUnitFromBigSquare(Unit* unit);
	}
}

#ifdef DEBUG_DEP
#warning "unitsquares.h-end"
#endif

#endif
