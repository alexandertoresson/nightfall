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
#ifndef TRACKER_H_PRE
#define TRACKER_H_PRE

#ifdef DEBUG_DEP
#warning "tracker.h-pre"
#endif

#include <string>
#include <map>

namespace Utilities
{
	class BeginRequest;

	class Tracker
	{
		private:
			std::string secretgameid;
			std::map<std::string, std::string> unsentUpdate;
			bool needShutdown;
			int lastUpdateID;
			void SetSecretGameID(std::string gameid);
		public:
			Tracker() : lastUpdateID(0) {}

			void BeginGame(int maxplayers, int freeplayerslots, int freespectatorslots);
			void UpdateGame(int maxplayers, int freeplayerslots, int freespectatorslots, bool started);
			void KeepAlive();
			void EndGame();
			bool IsPrefixOfSecretGameID(std::string prefix);
			std::string GetLastPartOfSecretGameId();

			friend class BeginRequest;
	};

	extern Tracker gameTracker;
}

#endif
