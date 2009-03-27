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
#ifndef NETWORKING_H_PRE
#define NETWORKING_H_PRE

#ifdef DEBUG_DEP
#warning "networking.h-pre"
#endif

//#define CHECKSUM_DEBUG
//#define CHECKSUM_DEBUG_HIGH

#include "sdlheader.h"
#include <vector>
#include <string>

namespace Game
{
	namespace Networking
	{
		extern bool isNetworked;
		extern bool isReady;
		extern bool isReadyToLoad;
		extern bool isReadyToStart;

		extern int playerCounter;
		extern int numReady;
		extern Uint32 netDelay;
		extern int netPort;
		extern Uint32 attempted_frame_count;
		extern Uint32 attempted_frames_waited;
		extern Uint32 bytes_sent;
		extern std::string nickname;

		class BitStream
		{
			private:
				BitStream(); // we don't want a constructur that takes no arguments;
				             // disallow the compiler from creating a default by
					     // creating a private dummy one.
			protected:
				short bitnum;
				Uint8 *data;
				Uint8 *lower_boundary, *upper_boundary;
			public:
				BitStream(Uint8* data);
				BitStream(Uint8* data, int max_size);
				BitStream(Uint8* data, Uint8* upper_boundary);
				BitStream(Uint8* data, Uint8* lower_boundary, Uint8* upper_boundary);
				int Seek(int numbits);
				int Seek(Uint8* data);
				int BytesUsed();
				int ReadBit();
				int ReadInteger(int num_bits);
				int WriteBit(int val);
				int WriteInteger(int num_bits, int val);
		};

		enum JoinStatus
		{
			JOIN_NOTSTARTED, // no join attempted yet
			JOIN_WAITING,    // waiting for server to answer
			JOIN_ACCEPTED,   // accepted onto server
			JOIN_REJECTED,   // rejected from game
			JOIN_TIMEOUT,    // timeout on waiting for response
			JOIN_CANCELLED,  // game cancelled before started
			JOIN_FAILED      // error while processing.
		};

		enum NETWORKTYPE
		{
			CLIENT,
			SERVER
		};

		bool PerformIngameNetworking();
		void PerformPregameNetworking();
		void InitIngameNetworking();
		void ShutdownNetwork();
		
		// CLIENT INTERFACE
		void JoinGame();
		JoinStatus GetJoinStatus();
		void ReadyToStart();
		int SetClientConnect(std::string host);
		bool isClientConnected();
		// SERVER INTERFACE
		void StartGame();
		void CancelGame();
		// CLIENT & SERVER INTERFACE
		std::vector<std::string> GetPlayerNames();
		bool AllPlayersReady();
		void SendReadyMessage();
		void CheckNodesReady();
	}
}

#endif

