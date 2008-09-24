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
#ifndef __REQUIREMENTS_H__
#define __REQUIREMENTS_H__

#ifdef DEBUG_DEP
#warning "requirements.h"
#endif

#include "requirements-pre.h"

#include "dimension-pre.h"

namespace Game
{
	namespace Dimension
	{		

		struct UnitRequirement
		{
			gc_ptr<UnitType> type;
			int minExisting, maxExisting;
			int minBuilt, maxBuilt;
		};

		struct ResearchRequirement
		{
			gc_ptr<Research> research;
			bool desiredState;
		};

		struct ConjunctiveRequirements
		{
			std::vector<ResearchRequirement> researchs;
			std::vector<UnitRequirement> units;
			std::string cReqString;
			bool isSatisfied;
			
			ConjunctiveRequirements()
			{
				isSatisfied = false;
			}
		};

		struct ObjectRequirements
		{
			ConjunctiveRequirements creation;
			ConjunctiveRequirements existance;
			int time;
			int money;
			int power;
		};
	}
}

#ifdef DEBUG_DEP
#warning "requirements.h-end"
#endif

#endif
