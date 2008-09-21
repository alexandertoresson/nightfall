#ifndef __TRACKER_H_PRE__
#define __TRACKER_H_PRE__

#ifdef DEBUG_DEP
#warning "tracker.h-pre"
#endif

#include <string>
#include <map>

namespace Utilities
{
	class Tracker
	{
		private:
			std::string secretgameid;
			std::map<std::string, std::string> unsentUpdate;
			bool needShutdown;
		public:
			void BeginGame(int maxplayers, int freeplayerslots, int freespectatorslots);
			void UpdateGame(int maxplayers, int freeplayerslots, int freespectatorslots, bool started);
			void EndGame();
			void SetSecretGameID(std::string gameid);
	};

	extern Tracker gameTracker;
}

#endif
