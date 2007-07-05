#ifndef __AIBASE_H_PRE__
#define __AIBASE_H_PRE__

#ifdef DEBUG_DEP
#warning "aibase.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace AI
	{
		struct UnitAIData;
		struct PlayerAIData;
		
		extern Uint32 currentFrame; // for tracking the number of the current frame
		extern int action_changes;
		extern int pathnodes;
		extern int paths;
		
		enum UnitAction // different actions
		{
			ACTION_NONE = 0,
			ACTION_GOTO,
			ACTION_ATTACK,
			ACTION_COLLECT,
			ACTION_BUILD,
			ACTION_RESEARCH,
			ACTION_REPAIR,
			ACTION_FOLLOW,
			ACTION_MOVE_ATTACK,
			ACTION_MOVE_ATTACK_UNIT,
			ACTION_DIE,
			ACTION_BEING_CREATED,
			ACTION_NETWORK_AWAITING_SYNC,
			ACTION_NUM         // only here to check the number of actions, not used as an action
		};

	}
}

#define __AIBASE_H_PRE_END__

#include "unit.h"

#endif

#ifdef __UNIT_H_PRE_END__

#ifndef __AIBASE_H__
#define __AIBASE_H__

#ifdef DEBUG_DEP
#warning "aibase.h"
#endif

#include <vector>
#include <queue>
#include <set>
#include <algorithm>

using namespace std;

namespace Game
{
	namespace AI
	{
		struct UnitAIData
		{
			// any data the computer player might want to store for this unit
			int dummy;
		};

		struct PlayerAIData
		{
			// any data the computer player might want to store for this player
			int dummy;
		};
		
		void PerformAI(Dimension::Unit* unit);     // Execute AI stuff for a unit
		void PerformAI(Dimension::Player* player); // Execute player AI (things that have nothing to do with the units, that is)
		void PerformAIFrame(); // Perform a full AI frame, if possible
		void CommandUnit(Dimension::Unit* unit, float x, float y, UnitAction action, void* argument, bool queue, bool insert);
		// command/instruct a unit to do something at (x, y)
		void CommandUnit(Dimension::Unit* unit, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert); 
		// command/instruct the unit to do something with another unit
		void CommandUnits(vector<Dimension::Unit*> units, float x, float y, UnitAction action, void* argument, bool queue, bool insert);
		// command/instruct several units to do something at (x, y)
		void CommandUnits(vector<Dimension::Unit*> units, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert); 
		// command/instruct several units to do something with another unit
		// note: argument is an optional argument that is needed for for example the ACTION_BUILD action to say what should be built...
		
		void CancelAction(Dimension::Unit* pUnit, unsigned int num);
		void CancelAction(Dimension::Unit* pUnit);
		void CompleteAction(Dimension::Unit* pUnit);
		void CancelAllActions(Dimension::Unit* pUnit);
		void ScheduleNextAction(Dimension::Unit* pUnit);
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, float goal_x, float goal_y, Dimension::Unit* target, void* arg);
		
		void SendUnitEventToLua_BuildComplete(Dimension::Unit* pUnit);
		void SendUnitEventToLua_BuildCancelled(Dimension::Unit* pUnit);
		void SendUnitEventToLua_ResearchComplete(Dimension::Unit* pUnit);
		void SendUnitEventToLua_ResearchCancelled(Dimension::Unit* pUnit);

		extern int aiFps;
		extern int aiFrame;
	
	}
}

#ifdef DEBUG_DEP
#warning "aibase.h-end"
#endif

#define __AIBASE_H_END__

#endif
#endif

