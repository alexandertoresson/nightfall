#ifndef __NETWORKING_H_PRE__
#define __NETWORKING_H_PRE__

#ifdef DEBUG_DEP
#warning "networking.h-pre"
#endif

#define CHECKSUM_DEBUG
//#define CHECKSUM_DEBUG_HIGH

#include "sdlheader.h"
#include <fstream>
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

#define __NETWORKING_H_PRE_END__

#include "unit.h"
#include "dimension.h"
#include "aibase.h"
#include "ainode.h"
#include "circularbuffer.h"

#endif

#ifdef __UNIT_H_PRE_END__
#ifdef __DIMENSION_H_END__
#ifdef __AIBASE_H_PRE_END__

#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#ifdef DEBUG_DEP
#warning "networking.h"
#endif

#include <queue>

namespace Game
{
	namespace Networking
	{
		struct NetActionData
		{
			Uint16 unit_id;
			Uint16 x;
			Uint16 y;
			Uint16 goalunit_id;
			AI::UnitAction action;
			void* arg;
			Uint32 valid_at_frame;
		};

		struct NetPath
		{
			Uint16 unit_id;
			AI::Node *pStart;
			AI::Node *pGoal;
			Uint32 valid_at_frame;
		};
		
		struct NetCreate
		{
			Uint16 unittype_id;
			Uint16 owner_id;
			Uint16 x;
			Uint16 y;
			Uint16 rot;
			Uint32 valid_at_frame;
		};
		
		struct NetDamage
		{
			Uint16 unit_id;
			Uint32 damage; // x100
			Uint32 valid_at_frame;
		};

		struct NetSell
		{
			Uint16 owner_id;
			Uint32 amount;
			Uint32 valid_at_frame;
		};
		
		void PrepareAction(Dimension::Unit* unit, Dimension::Unit* target, float x, float y, AI::UnitAction action, void* arg);
		void PreparePath(Dimension::Unit* unit, AI::Node* pStart, AI::Node* pGoal);
		void PrepareCreation(Dimension::UnitType* unittype, Dimension::Player* owner, float x, float y, float rot);
		void PrepareDamaging(Dimension::Unit* unit, float damage);
		void PrepareSell(Dimension::Player* owner, float amount);

		typedef Uint8 BUFFER;

		struct Chunk
		{
			Uint8 id[4];
			Uint16 length;
			BUFFER* data;
		};

		struct Packet
		{
			Uint8 id[4];
			BUFFER* frame;
			Uint16 frameLength;
			Uint16 numChunks;
			Chunk** chunks;
			int references;
			int node;

			Packet() { node = 0; }
		};

		int _networkServerSendThread(void* arg);
		int _networkServerRecvThread(void* arg);
		int _networkSendThread(void* arg);
		int _networkRecvThread(void* arg);

		struct NetworkSocket
		{
			Uint8 *pBufferIn;
			Uint8 *pBufferOut;
			TCPsocket socket; //Client or Server socket
			SDLNet_SocketSet set;
		};

		enum NETWORKTYPE
		{
			CLIENT,
			SERVER
		};

		extern NETWORKTYPE networkType;

#ifdef CHECKSUM_DEBUG
		extern CircularBuffer checksum_output;
#endif

		void InitNetwork();
		int StartNetwork(NETWORKTYPE type);
		
		Uint32 ChecksumPacket(BUFFER* rawdata, int len);
		Packet* ProcessPacket(BUFFER* rawdata, int rawlen);

		void AppendDataToPacket(BUFFER*& pDest, void* pSrc, int len);
		int CreatePacket(Uint8* packet, Packet* data);
		void DeletePacket(Packet* data);
		
		Packet *PopReceivedPacket();
		void PushPacketToSend(Packet *packet);
		unsigned int QueueSize();
	}
}

#ifdef DEBUG_DEP
#warning "networking.h-end"
#endif

#define __NETWORKING_H_END__

#endif
#endif
#endif
#endif
