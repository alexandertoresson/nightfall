#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef DEBUG_DEP
#warning "action.h"
#endif

#include "unit-pre.h"
#include "unittype-pre.h"
#include "aibase-pre.h"
#include "research.h"
#include "aipathfinding-pre.h"

namespace Game
{
	namespace Dimension
	{
		
		struct UnitGoal
		{
			gc_ptr<Unit> unit;
			IntPosition pos;

			UnitGoal();

			UnitGoal(const gc_ptr<Unit>& unit, int pos_x, int pos_y);

			void shade();
		};
		
		struct ActionArguments
		{
			gc_ptr<UnitType> unitType;
			gc_ptr<Research> research;
			int argHandle;

			ActionArguments();
			
			ActionArguments(const gc_root_ptr<UnitType>::type& unitType);
			
			ActionArguments(const gc_root_ptr<Research>::type& research);
			
			ActionArguments(const gc_ptr<UnitType>& unitType);
			
			ActionArguments(const gc_ptr<Research>& research);
			
			ActionArguments(int i);

			void shade();
			
		};

		struct BaseActionData
		{
			UnitGoal goal;
			AI::UnitAction action;
			float rotation;
			ActionArguments args;

			BaseActionData();

			BaseActionData(UnitGoal goal, AI::UnitAction action, float rotation, ActionArguments args);
			
			void shade();
			
		};

		enum UnitEventType
		{
			UNITEVENTTYPE_ACTION,
			UNITEVENTTYPE_SIMPLE,
			UNITEVENTTYPE_ATTACK
		};

		struct UnitEvent : public Dimension::BaseActionData
		{
			UnitEventType eventType;
			gc_ptr<Dimension::Unit> unit;
			std::string func;

			UnitEvent(const gc_ptr<Dimension::Unit>& pUnit, AI::EventAIFunc *aiEvent, UnitEventType eventType);
			
			UnitEvent(const gc_ptr<Dimension::Unit>& pUnit, const gc_ptr<Dimension::Unit>& target, AI::EventAIFunc *aiEvent);
		};

	}
}

#ifdef DEBUG_DEP
#warning "action.h-end"
#endif

#endif
