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
#ifndef DIMENSION_H_PRE
#define DIMENSION_H_PRE

#ifdef DEBUG_DEP
#warning "dimension.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		struct World;
		struct Player;
		struct Position;
		struct Resources;
		struct ObjectRequirements;
		struct Research;
		
		extern gc_ptr<Player> currentPlayerView;
		extern gc_ptr<Player> currentPlayer;
		
		class InputController;
		
		enum PlayerType
		{
			PLAYER_TYPE_HUMAN,
			PLAYER_TYPE_AI
		};

		struct Resources
		{
			double power;
			double money;
			Resources()
			{
				money = 1000.0;
				power = 1000.0;
			}
		};
		
		struct Position : gc_null_shader<Position>
		{
			float x;
			float y;
			
			Position()
			{
				x = y = 0;
			}

			Position(float nx, float ny) : x(nx), y(nx)
			{

			}
		};
		
		struct IntPosition : gc_null_shader<IntPosition>
		{
			int x;
			int y;

			IntPosition() : x(-1), y(-1)
			{
				
			}

			IntPosition(int x, int y) : x(x), y(y)
			{
				
			}
		};

		typedef int PlayerState;

		const int PLAYER_STATE_ALLY = 1,
		          PLAYER_STATE_NEUTRAL = 2,
		          PLAYER_STATE_ENEMY = 4;
		
		void UnloadAllUnitTypes();
		void EnforceMinimumExistanceRequirements();
		
		// Stänger ned världen
		void UnloadWorld(void);
		gc_ptr<Research> GetResearchByID(unsigned i);
	}
}

#endif

