#ifndef __DIMENSION_H_PRE__
#define __DIMENSION_H_PRE__

#ifdef DEBUG_DEP
#warning "dimension.h-pre"
#endif

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
		
		extern Player*       currentPlayerView;
		extern Player*       currentPlayer;
		
		class InputController;
		
		enum PlayerType
		{
			PLAYER_TYPE_HUMAN,
			PLAYER_TYPE_AI
		};

		typedef int PlayerState;

		const int PLAYER_STATE_ALLY = 1,
		          PLAYER_STATE_NEUTRAL = 2,
		          PLAYER_STATE_ENEMY = 4;
		
		void UnloadAllUnitTypes();
		void EnforceMinimumExistanceRequirements();
		
		// Stänger ned världen
		void UnloadWorld(void);
	}
}

#endif

