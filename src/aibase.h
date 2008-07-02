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
		void PerformAI(Dimension::Unit* unit);     // Execute AI stuff for a unit
		void PerformAI(Dimension::Player* player); // Execute player AI (things that have nothing to do with the units, that is)
		void PerformAIFrame(); // Perform a full AI frame, if possible
		void CommandUnit(Dimension::Unit* unit, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert);
		// command/instruct a unit to do something at (x, y)
		void CommandUnit(Dimension::Unit* unit, Dimension::Unit* destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert); 
		// command/instruct the unit to do something with another unit
		void CommandUnits(const std::vector<Dimension::Unit*>& units, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert);
		// command/instruct several units to do something at (x, y)
		void CommandUnits(const std::vector<Dimension::Unit*>& units, Dimension::Unit* destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert); 
		// command/instruct several units to do something with another unit
		// note: argument is an optional argument that is needed for for example the ACTION_BUILD action to say what should be built...
		
		void CancelAction(Dimension::Unit* pUnit, unsigned int num);
		void CancelAction(Dimension::Unit* pUnit);
		void CompleteAction(Dimension::Unit* pUnit);
		void CancelAllActions(Dimension::Unit* pUnit);
		void IssueNextAction(Dimension::Unit* pUnit);
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, int goal_x, int goal_y, Dimension::Unit* target, const Dimension::ActionArguments& args, float rotation);
		
		void SendUnitEventToLua_IsAttacked(Dimension::Unit* pUnit, Dimension::Unit* attacker);
		void SendUnitEventToLua_UnitCreation(Dimension::Unit* pUnit);
		void SendUnitEventToLua_UnitKilled(Dimension::Unit* pUnit);
		void SendUnitEventToLua_BecomeIdle(Dimension::Unit* pUnit);

		extern unsigned aiFps;
	
	}
}

#ifdef DEBUG_DEP
#warning "aibase.h-end"
#endif

#endif
