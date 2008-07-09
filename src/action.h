#ifndef __ACTION_H__
#define __ACTION_H__

#ifdef DEBUG_DEP
#warning "action.h"
#endif

#include "unit-pre.h"
#include "unittype.h"
#include "aibase-pre.h"
#include "dimension.h"

namespace Game
{
	namespace Dimension
	{
		
		struct UnitGoal
		{
			gc_ptr<Unit> unit;
			IntPosition pos;

			UnitGoal() : unit(NULL)
			{
				
			}

			UnitGoal(const gc_ptr<Unit>& unit, int pos_x, int pos_y) : unit(unit), pos(pos_x, pos_y)
			{
				
			}

			void shade()
			{
				unit.shade();
			}
		};
		
		struct ActionArguments
		{
			gc_ptr<UnitType> unitType;
			gc_ptr<Research> research;
			int argHandle;

			ActionArguments() : argHandle(-1)
			{
				
			}
			
			ActionArguments(const gc_root_ptr<UnitType>& unitType) : unitType(unitType), argHandle(unitType->GetHandle())
			{
				
			}
			
			ActionArguments(const gc_root_ptr<Research>& research) : research(research), argHandle(research->GetHandle())
			{
				
			}
			
			ActionArguments(const gc_ptr<UnitType>& unitType) : unitType(unitType), argHandle(unitType->GetHandle())
			{
				
			}
			
			ActionArguments(const gc_ptr<Research>& research) : research(research), argHandle(research->GetHandle())
			{
				
			}
			
			ActionArguments(int i) : argHandle(i)
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

			void shade()
			{
				research.shade();
				unitType.shade();
			}
			
		};

		struct BaseActionData
		{
			UnitGoal goal;
			AI::UnitAction action;
			float rotation;
			ActionArguments args;

			BaseActionData()
			{
				
			}

			BaseActionData(UnitGoal goal, AI::UnitAction action, float rotation, ActionArguments args) : goal(goal), action(action), rotation(rotation), args(args)
			{
				
			}
			
/*			BaseActionData(const ActionData& a) : goal(a.goal), action(a.action), rotation(a.rotation), args(a.args)
			{
				
			}*/
			
			void shade()
			{
				goal.shade();
				args.shade();
			}
			
		};

	}
}

#ifdef DEBUG_DEP
#warning "action.h-end"
#endif

#endif
