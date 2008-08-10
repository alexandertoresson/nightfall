#include "action.h"
#include "unittype.h"
#include "unit.h"
#include "aipathfinding.h"
			
namespace Game
{
	namespace Dimension
	{
	
		UnitGoal::UnitGoal() : unit(NULL)
		{
			
		}

		UnitGoal::UnitGoal(const gc_ptr<Unit>& unit, int pos_x, int pos_y) : unit(unit), pos(pos_x, pos_y)
		{
			
		}

		void UnitGoal::shade()
		{
			unit.shade();
		}
		
		ActionArguments::ActionArguments() : argHandle(-1)
		{
			
		}
		
		ActionArguments::ActionArguments(const gc_root_ptr<UnitType>::type& unitType) : unitType(unitType), argHandle(unitType->GetHandle())
		{
			
		}
		
		ActionArguments::ActionArguments(const gc_root_ptr<Research>::type& research) : research(research), argHandle(research->GetHandle())
		{
			
		}
		
		ActionArguments::ActionArguments(const gc_ptr<UnitType>& unitType) : unitType(unitType), argHandle(unitType->GetHandle())
		{
			
		}
		
		ActionArguments::ActionArguments(const gc_ptr<Research>& research) : research(research), argHandle(research->GetHandle())
		{
			
		}
		
		ActionArguments::ActionArguments(int i) : argHandle(i)
		{
			if (HandleManager<UnitType>::IsCorrectHandle(i))
			{
				unitType = HandleManager<UnitType>::InterpretHandle(i);
			}
			else if (HandleManager<Research>::IsCorrectHandle(i))
			{
				research = HandleManager<Research>::InterpretHandle(i);
			}
		}

		void ActionArguments::shade()
		{
			research.shade();
			unitType.shade();
		}
		
		BaseActionData::BaseActionData()
		{
			
		}

		BaseActionData::BaseActionData(UnitGoal goal, AI::UnitAction action, float rotation, ActionArguments args) : goal(goal), action(action), rotation(rotation), args(args)
		{
			
		}
		
		void BaseActionData::shade()
		{
			goal.shade();
			args.shade();
		}
			
		UnitEvent::UnitEvent(const gc_ptr<Dimension::Unit>& pUnit, AI::EventAIFunc *aiEvent, UnitEventType eventType) : BaseActionData(pUnit->pMovementData->action), eventType(eventType), unit(pUnit), func(aiEvent->func)
		{
		}
		
		UnitEvent::UnitEvent(const gc_ptr<Dimension::Unit>& pUnit, const gc_ptr<Dimension::Unit>& target, AI::EventAIFunc *aiEvent) : BaseActionData(pUnit->pMovementData->action), eventType(UNITEVENTTYPE_ATTACK), unit(pUnit), func(aiEvent->func)
		{
			goal.unit = target;
		}
	}
}
