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
			Unit* unit;
			IntPosition pos;

			UnitGoal() : unit(NULL)
			{
				
			}

			UnitGoal(Unit* unit, int pos_x, int pos_y) : unit(unit), pos(pos_x, pos_y)
			{
				
			}
		};
		
		struct ActionArguments
		{
			ref_ptr<UnitType> unitType;
			ref_ptr<Research> research;
			int argHandle;

			ActionArguments() : argHandle(-1)
			{
				
			}
			
			ActionArguments(const ref_ptr<UnitType>& unitType) : unitType(unitType), argHandle(unitType->GetHandle())
			{
				
			}
			
			ActionArguments(const ref_ptr<Research>& research) : research(research), argHandle(research->GetHandle())
			{
				
			}
			
			ActionArguments(unsigned i) : argHandle(i)
			{
				
				if (i >= 65536)
				{
					if (i >= 131072)
					{
						research = GetResearchByID(i-131072);
					}
					else
					{
						unitType = GetUnitTypeByID(i-65536);
					}
				}
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
		};

	}
}

#ifdef DEBUG_DEP
#warning "action.h-end"
#endif

#endif
