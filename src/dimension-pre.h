#ifndef __DIMENSION_H_PRE__
#define __DIMENSION_H_PRE__

#ifdef DEBUG_DEP
#warning "dimension.h-pre"
#endif

#include "ref_ptr.h"

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
		
		extern ref_ptr<Player> currentPlayerView;
		extern ref_ptr<Player> currentPlayer;
		
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
		
		struct Position
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
		
		struct IntPosition
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
		ref_ptr<Research> GetResearchByID(unsigned i);
	}
}

#endif

