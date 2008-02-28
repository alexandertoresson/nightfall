#ifndef __AIBASE_H_PRE__
#define __AIBASE_H_PRE__

#ifdef DEBUG_DEP
#warning "aibase.h-pre"
#endif

#include "sdlheader.h"
#include <string>

namespace Game
{
	namespace AI
	{
		extern Uint32 currentFrame; // for tracking the number of the current frame
		extern int action_changes;
		extern int pathnodes;
		extern int paths;
		extern int aiFramesPerformedSinceLastRender;
		
		extern int numLuaAIThreads;

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

		struct EventAIFunc
		{
			std::string func;
		};

		struct RegularAIFunc
		{
			std::string func;
			bool enabled;
			int delay;
		};

		struct UnitAIFuncs
		{
			RegularAIFunc performUnitAI;
			EventAIFunc commandCompleted;
			EventAIFunc commandCancelled;
			EventAIFunc newCommand;
			EventAIFunc becomeIdle;
			EventAIFunc isAttacked;
			EventAIFunc unitKilled;
		};

		struct PlayerAIFuncs
		{
			RegularAIFunc performPlayerAI;
			EventAIFunc unitCreation;
			EventAIFunc commandUnitTargetUnit;
			EventAIFunc commandUnitTargetPos;
		};

		void InitAIThreads();
		void SendScheduledUnitEvents();
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
		void PerformAI(Dimension::Unit* unit);     // Execute AI stuff for a unit
		void PerformAI(Dimension::Player* player); // Execute player AI (things that have nothing to do with the units, that is)
		void PerformAIFrame(); // Perform a full AI frame, if possible
		void CommandUnit(Dimension::Unit* unit, int x, int y, UnitAction action, void* argument, bool queue, bool insert);
		// command/instruct a unit to do something at (x, y)
		void CommandUnit(Dimension::Unit* unit, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert); 
		// command/instruct the unit to do something with another unit
		void CommandUnits(vector<Dimension::Unit*> units, int x, int y, UnitAction action, void* argument, bool queue, bool insert);
		// command/instruct several units to do something at (x, y)
		void CommandUnits(vector<Dimension::Unit*> units, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert); 
		// command/instruct several units to do something with another unit
		// note: argument is an optional argument that is needed for for example the ACTION_BUILD action to say what should be built...
		
		void CancelAction(Dimension::Unit* pUnit, unsigned int num);
		void CancelAction(Dimension::Unit* pUnit);
		void CompleteAction(Dimension::Unit* pUnit);
		void CancelAllActions(Dimension::Unit* pUnit);
		void IssueNextAction(Dimension::Unit* pUnit);
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, int goal_x, int goal_y, Dimension::Unit* target, void* arg);
		
		void SendUnitEventToLua_IsAttacked(Dimension::Unit* pUnit, Dimension::Unit* attacker);
		void SendUnitEventToLua_UnitCreation(Dimension::Unit* pUnit);
		void SendUnitEventToLua_UnitKilled(Dimension::Unit* pUnit);
		void SendUnitEventToLua_BecomeIdle(Dimension::Unit* pUnit);

		extern unsigned aiFps;
		extern int aiFrame;
	
	}
}

#ifdef DEBUG_DEP
#warning "aibase.h-end"
#endif

#define __AIBASE_H_END__

#endif
#endif

