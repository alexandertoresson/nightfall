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
#ifndef ACTION_H
#define ACTION_H

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
