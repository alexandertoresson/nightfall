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
#ifndef AIBASE_H_PRE
#define AIBASE_H_PRE

#ifdef DEBUG_DEP
#warning "aibase.h-pre"
#endif

#include "sdlheader.h"
#include <string>

namespace Game
{
	namespace AI
	{
		extern Uint32 currentFrame; // for tracking the number of the current frame
		extern int action_changes;
		extern int pathnodes;
		extern int paths;
		extern int aiFramesPerformedSinceLastRender;
		
		extern int numLuaAIThreads;

		enum UnitAction // different actions
		{
			ACTION_NONE = 0,
			ACTION_GOTO,
			ACTION_ATTACK,
			ACTION_COLLECT,
			ACTION_BUILD,
			ACTION_RESEARCH,
			ACTION_REPAIR,
			ACTION_FOLLOW,
			ACTION_MOVE_ATTACK,
			ACTION_MOVE_ATTACK_UNIT,
			ACTION_DIE,
			ACTION_BEING_CREATED,
			ACTION_NETWORK_AWAITING_SYNC,
			ACTION_NUM         // only here to check the number of actions, not used as an action
		};

		struct EventAIFunc
		{
			std::string func;
		};

		struct RegularAIFunc
		{
			std::string func;
			bool enabled;
			int delay;
		};

		struct UnitAIFuncs
		{
			RegularAIFunc performUnitAI;
			EventAIFunc commandCompleted;
			EventAIFunc commandCancelled;
			EventAIFunc newCommand;
			EventAIFunc becomeIdle;
			EventAIFunc isAttacked;
			EventAIFunc unitKilled;
		};

		struct PlayerAIFuncs
		{
			RegularAIFunc performPlayerAI;
			EventAIFunc unitCreation;
			EventAIFunc commandUnit;
		};

		void InitAIThreads();
		void InitAIMiscMutexes();
	}
}

#endif

