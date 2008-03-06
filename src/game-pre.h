#ifndef __GAME_H_PRE__
#define __GAME_H_PRE__

#ifdef DEBUG_DEP
#warning "game.h-pre"
#endif

#include <string>

namespace Game
{
	namespace Rules
	{

		extern float time_since_last_frame;
		extern std::string CurrentLevel;
		extern std::string CurrentLevelScript;

		extern bool noGraphics;
		extern bool graphicsLoaded;
		extern bool noSound;
		extern int numPlayersGoal;
		extern std::string host;
		extern std::string checksumLog;

		extern float time_passed_since_last_water_pass;

		//All game related run loop shall return a switchstate.
		enum SwitchState
		{
			QUIT = 0,
			SETTINGS,
			INGAMEMENU,
			GAME,
			NEWGAME,
			LOADGAME,
			ENDGAME,
			CREDITS,
			MENU,
			MULTIPLAYER,
			NETWORKCREATE,
			NETWORKJOIN
		};

		extern SwitchState startState;

		class GameWindow;
	}
}

#endif

