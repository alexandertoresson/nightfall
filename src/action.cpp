/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
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
