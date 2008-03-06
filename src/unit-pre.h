#ifndef __UNIT_H_PRE__
#define __UNIT_H_PRE__

#ifdef DEBUG_DEP
#warning "unit.h-pre"
#endif

namespace Game
{
	namespace Dimension
	{
		struct Unit;
		struct ActionData;
		struct UnitType;
		struct Model;
		struct ProjectileType;
		struct Projectile;
		struct MorphAnim;
		struct TransformAnim;
		struct TransData;
		struct Animation;
		
		Unit *GetUnitByID(unsigned id);
		bool IsValidUnitPointer(Unit* unit); // May not be called from another thread
		bool IsDisplayedUnitPointer(Unit* unit); // May be calle from synced threads

		inline int GetTraversalTime(Unit *unit, int x, int y, int dx, int dy);
		int GetTraversalTimeAdjusted(Unit *unit, int x, int y, int dx, int dy);
		bool MoveUnit(Unit* unit);
		void ApplyScheduledBigSquareUpdates();
		bool SquareIsGoal(Unit *unit, int x, int y, bool use_internal = false);
		
		bool SquareIsWalkable(Unit *unit, int x, int y, int flags);
		bool SquaresAreWalkable(Unit *unit, int x, int y, int flags);
		inline bool SquareIsWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(UnitType *type, int x, int y, int flags);
		
		struct TransformData;
		struct TransformAnim;
		struct Animation;
		extern int** numUnitsPerAreaMap;
		extern int numSentCommands;
		
		void GenerateUnitTypeRanges(UnitType* type);
	}
}

#endif

