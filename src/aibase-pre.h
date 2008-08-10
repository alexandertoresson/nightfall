#ifndef __AIBASE_H_PRE__
#define __AIBASE_H_PRE__

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

