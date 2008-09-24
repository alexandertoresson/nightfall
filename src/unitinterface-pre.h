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
#ifndef __UNITINTERFACE_H_PRE__
#define __UNITINTERFACE_H_PRE__

#ifdef DEBUG_DEP
#warning "unitinterface.h-pre"
#endif

namespace UnitLuaInterface
{
	enum EventType
	{
		EVENTTYPE_COMMANDCOMPLETED,
		EVENTTYPE_COMMANDCANCELLED,
		EVENTTYPE_NEWCOMMAND,
		EVENTTYPE_BECOMEIDLE,
		EVENTTYPE_ISATTACKED,
		EVENTTYPE_PERFORMUNITAI,
		EVENTTYPE_UNITCREATION,
		EVENTTYPE_UNITKILLED,
		EVENTTYPE_PERFORMPLAYERAI
	};
	
	enum AIContexts
	{
		AI_CONTEXT_UNIT,
		AI_CONTEXT_UNITTYPE,
		AI_CONTEXT_PLAYER
	};
	
	void ApplyScheduledActions();
	void ApplyScheduledDamagings();
	void PostProcessStrings();
}

#endif

