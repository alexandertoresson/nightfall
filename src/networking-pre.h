#ifndef __NETWORKING_H_PRE__
#define __NETWORKING_H_PRE__

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
		int SetClientConnect(char* host);
		bool isClientConnected();
		// SERVER INTERFACE
		bool CreateGame(char* player_name);
		void StartGame();
		void CancelGame();
		// CLIENT & SERVER INTERFACE
		std::vector<char*> GetPlayerNames();
		bool AllPlayersReady();
		void SendReadyMessage();
		void CheckNodesReady();
	}
}

#endif

