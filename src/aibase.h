#include "unit-pre.h"

#ifndef __AIBASE_H__
#define __AIBASE_H__

#ifdef DEBUG_DEP
#warning "aibase.h"
#endif

#include "aibase-pre.h"
#include "dimension-pre.h"
#include "action-pre.h"

#include <vector>

namespace Game
{
	namespace AI
	{
		void PerformAI(const gc_ptr<Dimension::Unit>& unit);     // Execute AI stuff for a unit
		void PerformAI(const gc_ptr<Dimension::Player>& player); // Execute player AI (things that have nothing to do with the units, that is)
		void PerformAIFrame(); // Perform a full AI frame, if possible
		void CommandUnit(const gc_ptr<Dimension::Unit>& unit, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert);
		// command/instruct a unit to do something at (x, y)
		void CommandUnit(const gc_ptr<Dimension::Unit>& unit, const gc_ptr<Dimension::Unit>& destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert); 
		// command/instruct the unit to do something with another unit
		void CommandUnits(const std::vector<gc_ptr<Dimension::Unit> >& units, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert);
		// command/instruct several units to do something at (x, y)
		void CommandUnits(const std::vector<gc_ptr<Dimension::Unit> >& units, const gc_ptr<Dimension::Unit>& destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert); 
		// command/instruct several units to do something with another unit
		// note: argument is an optional argument that is needed for for example the ACTION_BUILD action to say what should be built...
		
		void CancelAction(const gc_ptr<Dimension::Unit>& pUnit, unsigned int num);
		void CancelAction(const gc_ptr<Dimension::Unit>& pUnit);
		void CompleteAction(const gc_ptr<Dimension::Unit>& pUnit);
		void CancelAllActions(const gc_ptr<Dimension::Unit>& pUnit);
		void IssueNextAction(const gc_ptr<Dimension::Unit>& pUnit);
		void ApplyAction(const gc_ptr<Dimension::Unit>& pUnit, UnitAction action, int goal_x, int goal_y, const gc_ptr<Dimension::Unit>& target, const Dimension::ActionArguments& args, float rotation);
		
		void SendUnitEventToLua_IsAttacked(const gc_ptr<Dimension::Unit>& pUnit, const gc_ptr<Dimension::Unit>& attacker);
		void SendUnitEventToLua_UnitCreation(const gc_ptr<Dimension::Unit>& pUnit);
		void SendUnitEventToLua_UnitKilled(const gc_ptr<Dimension::Unit>& pUnit);
		void SendUnitEventToLua_BecomeIdle(const gc_ptr<Dimension::Unit>& pUnit);

		extern unsigned aiFps;
		extern SDL_mutex* updateMutex;
	
	}
}

#ifdef DEBUG_DEP
#warning "aibase.h-end"
#endif

#endif
