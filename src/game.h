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
#ifndef GAME_H
#define GAME_H

#ifdef DEBUG_DEP
#warning "game.h"
#endif

#include "game-pre.h"

#include "dimension-pre.h"
#include "unit-pre.h"
#include "gui.h"
#include "gamegui.h"
#include "effect-pre.h"
#include "camera-pre.h"
#include "gamegui-pre.h"
#include "networking-pre.h"

#include "sdlheader.h"

#include <vector>
#include <queue>
#include <sstream>
#include <iomanip>
#include <string>

namespace Game
{
	namespace Rules
	{

		class CurGame
		{
			private:
				Uint32 last_frame, this_frame;
				float time_passed_since_last_ai_pass;
				float time_since_last_frame;
				bool atLeastOneFrameCalculated;

				std::string levelHash;

				volatile bool gameRunning;

				SDL_Thread* gameLogicThread;

				CurGame();
				~CurGame();

				int InitGame(bool is_new_game = true, bool isNetworked = false, Networking::NETWORKTYPE ntype = Networking::CLIENT);
				void PerformPreFrame();

				static CurGame* instance;
		
			public:
				int StartGame(std::string saveGame = "", bool isNetworked = false, Networking::NETWORKTYPE ntype = Networking::CLIENT);
				void EndGame();

				void StartGameLogicThread();
				
				bool RunGameLogicFrame();

				bool GameRunning();

				bool AtLeastOneFrameCalculated()
				{
					return atLeastOneFrameCalculated;
				}

				const std::string& GetLevelHash();

				static CurGame* Instance();
				static CurGame* New();
		};
	}
}

#ifdef DEBUG_DEP
#warning "game.h-end"
#endif

#endif

