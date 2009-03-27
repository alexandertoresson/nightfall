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
#ifndef UNITINTERFACE_H
#define UNITINTERFACE_H

#ifdef DEBUG_DEP
#warning "unitinterface.h"
#endif

#include "sdlheader.h"
#include "unitinterface-pre.h"

#include "luawrapper-pre.h"
#include "unit-pre.h"
#include "dimension-pre.h"
#include <string>

namespace UnitLuaInterface
{
	void Init(Utilities::Scripting::LuaVMState* pVM);
	gc_ptr<Game::Dimension::UnitType>& GetUnitTypeByID(const gc_ptr<Game::Dimension::Player>& owner, std::string str);
	gc_ptr<Game::Dimension::Research>& GetResearchByID(const gc_ptr<Game::Dimension::Player>& owner, std::string str);
}

#ifdef DEBUG_DEP
#warning "unitinterface.h-end"
#endif

#endif
