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
#include "networking.h"

#include "ainode.h"
#include "aipathfinding.h"
#include "unit.h"
#include "game.h"
#include "camera.h"
#include "action.h"
#include "handle.h"
#include "tracker.h"
#include "utilities.h"
#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>

//#define NET_DEBUG
//#define NET_DEBUG_CONNECTION

using namespace std;

namespace Game
{
	namespace Networking
	{

#define NETWORK_BUFFER 65536
#define NETWORK_RECEIVE_CHUNK 4096
#define NETWORK_MAX_CLIENTS 32
#define PACKETTYPE(val) memcmp(packet->id, val, 4) == 0

		bool isNetworked = false;
		bool isReady = false;
		bool isReadyToLoad = false;
		bool isReadyToStart = false;

		bool scheduleShutdown = false;
		Uint32 shutdownFrame;

		int numReady = 0;
		int playerCounter = 2;

		int netPort = 51500;
		Uint32 netDelay = 5;
		unsigned queueLimit = 40;
		NETWORKTYPE networkType;
		unsigned numClients = 1;

		vector<NetActionData*> waitingActions;
		vector<NetPath*> waitingPaths;
		vector<NetActionData*> unsentActions;
		vector<NetPath*> unsentPaths;

		vector<NetCreate*> waitingCreations;
		vector<NetDamage*> waitingDamagings;
		vector<NetCreate*> unsentCreations;
		vector<NetDamage*> unsentDamagings;
		
		vector<NetSell*> waitingSells;
		vector<NetSell*> unsentSells;

		struct Checksum
		{
			Uint32 checksum;
			Uint32 frame;
			std::string data;
		};

		vector<Checksum*> waitingChecksums;
		vector<Checksum*> unsentChecksums;

		queue<Packet*> packetInQueue; //From network
		queue<Packet*> packetOutQueue; //To network
		queue<Packet*> packetFrameOutQueue; //To network (special queue for frame packets)

		int netDestCount = 1; //indicates amout of server connections aswell
		int numConnected = 0;
		
		enum NetworkNodeType
		{
			NETWORKNODETYPE_PLAYER = 0,
			NETWORKNODETYPE_SPECTATOR,
			NETWORKNODETYPE_DISCONNECTED
		};

		struct NetworkSocket
		{
			Uint8 *pBufferIn;
			Uint8 *pBufferOut;
			TCPsocket socket; //Client or Server socket
			SDLNet_SocketSet set;
		};

		struct PendingConnection
		{
			TCPsocket socket;
			std::string received;
		};

		string netNickname[NETWORK_MAX_CLIENTS];
		TCPsocket netDest[NETWORK_MAX_CLIENTS]; //clients connected to server (players, spectators).
		NetworkNodeType nodeTypes[NETWORK_MAX_CLIENTS];
		
		std::vector<PendingConnection> pendingSockets; // sockets which haven't told us what they want to do yet

		Uint8 **netDestBufferIn;
		int *netDataLeft;
		int *netDataTotal;
		int *netID;

		int *netBufferStart;
		int *netBufferEnd;

		bool serverListening = true;

		SDL_Thread* pNetworkThread = NULL;
		SDL_mutex* mutPacketInQueue = NULL;
		SDL_mutex* mutPacketOutQueue = NULL;
		SDL_mutex* mutPacketFrameOutQueue = NULL;
		SDL_mutex* mutServerConnected = NULL;
		SDL_mutex* mutClientConnect = NULL;

		SDL_mutex* mutNetworkShutdown = NULL;
		bool terminateNetwork = false;

		SDL_Thread* thrServerRecv;
		SDL_Thread* thrServerSend;
		SDL_Thread* thrSend;
		SDL_Thread* thrRecv;

		bool connect = false;
		bool clientConnected = false;
		IPaddress clientConnect;
		unsigned clientID = 0;
		string clientError;
		string nickname;

		bool *framePacketsReceived;
		bool **frameFragmentsReceived;
		bool **individualFramePacketsReceived;
		bool ***individualFrameFragmentsReceived;
		Packet **framePacketsSent;
		Uint32 frameRFRSSentAt;
		bool *frameMayAdvance;

#ifdef CHECKSUM_DEBUG
		CircularBuffer checksum_output(100000, "");
#endif

		// CRC32 functions:

		/*
 		 * efone - Distributed internet phone system.
 		 *
 		 * (c) 1999,2000 Krzysztof Dabrowski
 		 * (c) 1999,2000 ElysiuM deeZine
 		 *
 		 * This program is free software; you can redistribute it and/or
 		 * modify it under the terms of the GNU General Public License
 		 * as published by the Free Software Foundation; either version
 		 * 2 of the License, or (at your option) any later version.
 		 *
 		 */

		// based on implementation by Finn Yannick Jacobs
		// modified by the nightfall team to fit into the nightfall project

		// crc_tab[] -- this crcTable is being build by crc32_init().
		Uint32 crc_tab[256];

		Uint32 CRC32 (Uint8 *block, Uint32 length)
		{
			Uint32 crc;
			Uint32 i;

			crc = 0xFFFFFFFF;
			for (i = 0; i < length; i++)
			{
				crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
			}
			return (crc ^ 0xFFFFFFFF);
		}

		/* crc32_init() -- to a global crc_tab[256], this one will calculate the crcTable for crc32-checksums.
		*				it is generated to the polynom [..]
		*/

		void CRC32_init ()
		{
			Uint32 crc, poly;
			int i, j;

			poly = 0xEDB88320L;
			for (i = 0; i < 256; i++)
			{
				crc = i;
				for (j = 8; j > 0; j--)
				{
					if (crc & 1)
					{
						crc = (crc >> 1) ^ poly;
					}
					else
					{
						crc >>= 1;
					}
				}
				crc_tab[i] = crc;
			}
		}

		// end CRC32 functions 

		BitStream::BitStream(Uint8* data)
		{
			this->data = data;
			this->lower_boundary = NULL;
			this->upper_boundary = NULL;
			this->bitnum = 0;
		}

		BitStream::BitStream(Uint8* data, int max_size)
		{
			this->data = data;
			this->lower_boundary = data;
			this->upper_boundary = data + max_size-1;
			this->bitnum = 0;
		}

		BitStream::BitStream(Uint8* data, Uint8* upper_boundary)
		{
			this->data = data;
			this->lower_boundary = NULL;
			this->upper_boundary = upper_boundary;
			this->bitnum = 0;
		}

		BitStream::BitStream(Uint8* data, Uint8* lower_boundary, Uint8* upper_boundary)
		{
			this->data = data;
			this->lower_boundary = lower_boundary;
			this->upper_boundary = upper_boundary;
			this->bitnum = 0;
		}

		int BitStream::Seek(int num_bits)
		{
			int new_bitnum = this->bitnum + num_bits;
			this->data += new_bitnum / 8;
			this->bitnum = new_bitnum % 8;
			if ((!lower_boundary || data >= lower_boundary) && (!upper_boundary || data <= upper_boundary))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		int BitStream::Seek(Uint8* data)
		{
			this->data = data;
			this->bitnum = 0;
			if ((!lower_boundary || data >= lower_boundary) && (!upper_boundary || data <= upper_boundary))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		int BitStream::BytesUsed()
		{
			if (lower_boundary)
			{
				return data - lower_boundary+1;
			}
			else
			{
				return -1; 
			}
		}

		int BitStream::ReadBit()
		{
			if ((!lower_boundary || data >= lower_boundary) && (!upper_boundary || data <= upper_boundary))
			{
				int ret = (*data >> bitnum) & 1;
				Seek(1);
				return ret;
			}
			else
			{
				Seek(1);
				return -1;
			}
		}

		int BitStream::ReadInteger(int num_bits)
		{
			int ret = 0;
			for (int i = 0; i < num_bits; i++)
			{
				int bit = ReadBit();
				if (bit == -1)
					return -1;
				ret = ret * 2 + bit;
			}
			return ret;
		}

		int BitStream::WriteBit(int val)
		{
			if ((!lower_boundary || data >= lower_boundary) && (!upper_boundary || data <= upper_boundary))
			{
				*data = (*data & (255 - (1 << bitnum))) | (val << bitnum);
				Seek(1);
				return 1;
			}
			else
			{
				Seek(1);
				return 0;
			}
		}

		int BitStream::WriteInteger(int num_bits, int val)
		{
			int ret = 1;
			for (int i = 0; i < num_bits; i++)
			{
				ret &= WriteBit((val >> (num_bits - i - 1)) & 1);
			}
			return ret;
		}

		int EncodePath(AI::Node* pGoal, Uint8* data, int max_size)
		{
			AI::Node* curnode = pGoal;
			BitStream bitstream(data, max_size);
			int len;
			int numsteps = 0;
			int stepcodes[3][3] = {{0, 1, 2},
			                       {7,-1, 3},
			                       {6, 5, 4}};

			bitstream.Seek(12);
			bitstream.WriteInteger(12, pGoal->x);
			bitstream.WriteInteger(12, pGoal->y);

			while (curnode->pParent)
			{
				int stepcode = -1;
				if (fabs((float)curnode->pParent->x - curnode->x) <= 1 && fabs((float)curnode->pParent->y - curnode->y) <= 1)
				{
					stepcode = stepcodes[(curnode->pParent->y - curnode->y)+1][(curnode->pParent->x - curnode->x)+1];
				}
				if (stepcode != -1)
				{
					bitstream.WriteInteger(3, stepcode);
				}
				else
				{
					return 0;
				}
				numsteps++;
				curnode = curnode->pParent;
			}

			len = bitstream.BytesUsed();
			bitstream.Seek(data);
			bitstream.WriteInteger(12, numsteps);

			return len;
		}

		void DeallocPath(AI::Node *pGoal)
		{
			delete[] pGoal;
		}

		void ClonePath(AI::Node *&pStart, AI::Node *&pGoal)
		{
			AI::Node *new_nodes, *new_start, *new_cur, *cur, *new_last;
			int num_nodes = 0, i;
			cur = pGoal;
			while (cur)
			{
				num_nodes++;
				cur = cur->pParent;
			}
			new_nodes = new AI::Node[num_nodes];

			new_cur = &new_nodes[0];
			new_start = new_cur;
			cur = pGoal;
			new_last = NULL;
			i = 1;
			while (cur)
			{
				new_cur->x = cur->x;
				new_cur->y = cur->y;
				cur = cur->pParent;
				new_last = new_cur;
				if (cur)
				{
					new_cur = &new_nodes[i++];
					new_cur->pChild = new_last;
					new_last->pParent = new_cur;
				}
			}
			pStart = new_cur;
			pGoal = new_start;
		}

		int DecodePath(AI::Node *&pStart, AI::Node *&pGoal, Uint8* data, int max_size)
		{
			AI::Node *new_nodes, *curnode, *lastnode;
			BitStream bitstream(data, max_size);
			int numsteps = bitstream.ReadInteger(12);
			int stepcodes[8][2] = {{-1, -1},
			                       { 0, -1},
			                       { 1, -1},
			                       { 1,  0},
			                       { 1,  1},
			                       { 0,  1},
			                       {-1,  1},
			                       {-1,  0}};

			new_nodes = new AI::Node[numsteps+1];

			curnode = &new_nodes[0];
			pGoal = curnode;

			curnode->x = bitstream.ReadInteger(12);
			curnode->y = bitstream.ReadInteger(12);

			if (curnode->x == -1 || curnode->y == -1 || numsteps == -1)
			{
				DeallocPath(pGoal);
				pStart = NULL;
				return ERROR_GENERAL;
			}

			for (int i = 0; i < numsteps; i++)
			{
				int stepcode = bitstream.ReadInteger(3);
				if (stepcode == -1)
				{
					DeallocPath(pGoal);
					pGoal = NULL;
					return ERROR_GENERAL;
				}
				lastnode = curnode;
				curnode = &new_nodes[i+1];
				curnode->x = lastnode->x + stepcodes[stepcode][0];
				curnode->y = lastnode->y + stepcodes[stepcode][1];
				lastnode->pParent = curnode;
				curnode->pChild = lastnode;
			}

			pStart = curnode;

			return SUCCESS;
		}

		Uint8 RotationToByte(float rotation)
		{
			return (Uint8) floor(rotation / 360 * 256);
		}

		float ByteToRotation(Uint8 rotation)
		{
			return (float) rotation / 256 * 360;
		}

		SDL_mutex* prepareActionMutex = NULL;

		void PrepareAction(const gc_ptr<Dimension::Unit>& unit, const gc_ptr<Dimension::Unit>& target, int x, int y, AI::UnitAction action, const Dimension::ActionArguments& args, float rotation)
		{
			NetActionData* actiondata = new NetActionData;
			actiondata->unit_id = unit->GetIndependentHandle();
			actiondata->action = action;
			actiondata->x = x;
			actiondata->y = y;
			actiondata->rot = RotationToByte(rotation);
			if (target)
				actiondata->goalunit_id = target->GetIndependentHandle();
			else
				actiondata->goalunit_id = 0xFFFF;
			actiondata->arg = args.argHandle != -1 ? args.argHandle - Dimension::HandleTraits<Dimension::UnitType>::base : 0xFFFF;
			actiondata->valid_at_frame = AI::currentFrame + netDelay;
			SDL_LockMutex(prepareActionMutex);
			if (networkType == SERVER)
			{
				NetActionData* actiondata_copy = new NetActionData;
				*actiondata_copy = *actiondata;
				waitingActions.push_back(actiondata_copy);
			}
			unsentActions.push_back(actiondata);
			if (unit->pMovementData->action.action == AI::ACTION_NONE)
				unit->pMovementData->action.action = AI::ACTION_NETWORK_AWAITING_SYNC;
			SDL_UnlockMutex(prepareActionMutex);
		}

		void PreparePath(const gc_ptr<Dimension::Unit>& unit, AI::Node* pStart, AI::Node* pGoal)
		{
			NetPath* path = new NetPath;
			ClonePath(pStart, pGoal);
			path->unit_id = unit->GetIndependentHandle();
			path->pStart = pStart;
			path->pGoal = pGoal;
			path->valid_at_frame = AI::currentFrame + netDelay;
			if (networkType == SERVER)
			{
				NetPath* path_copy = new NetPath;
				*path_copy = *path;
				ClonePath(path_copy->pStart, path_copy->pGoal);
				waitingPaths.push_back(path_copy);
			}
			unsentPaths.push_back(path);
		}

		SDL_mutex* prepareCreationMutex = NULL;

		void PrepareCreation(const gc_ptr<Dimension::UnitType>& unittype, int x, int y, float rot)
		{
			NetCreate* create = new NetCreate;
			create->unittype_id = unittype->GetIndependentHandle();
			create->owner_id = unittype->player->GetIndependentHandle();
			create->x = x;
			create->y = y;
			create->rot = RotationToByte(rot);
			create->valid_at_frame = AI::currentFrame + netDelay;
			SDL_LockMutex(prepareCreationMutex);
			if (networkType == SERVER)
			{
				NetCreate* create_copy = new NetCreate;
				*create_copy = *create;
				waitingCreations.push_back(create_copy);
			}
			unsentCreations.push_back(create);
			SDL_UnlockMutex(prepareCreationMutex);
		}

		SDL_mutex* prepareDamagingMutex = NULL;

		void PrepareDamaging(const gc_ptr<Dimension::Unit>& unit, float damage)
		{
			NetDamage* dmg = new NetDamage;
			dmg->unit_id = unit->GetIndependentHandle();
			dmg->damage = (int) (damage * 100);
			dmg->valid_at_frame = AI::currentFrame + netDelay;
			SDL_LockMutex(prepareDamagingMutex);
			if (networkType == SERVER)
			{
				NetDamage* damage_copy = new NetDamage;
				*damage_copy = *dmg;
				waitingDamagings.push_back(damage_copy);
			}
			unsentDamagings.push_back(dmg);
			SDL_UnlockMutex(prepareDamagingMutex);
		}

		SDL_mutex* prepareSellMutex = NULL;

		void PrepareSell(const gc_ptr<Dimension::Player>& owner, int amount)
		{
			NetSell* sell = new NetSell;
			sell->owner_id = owner->GetIndependentHandle();
			sell->amount = amount;
			sell->valid_at_frame = AI::currentFrame + netDelay;
			SDL_LockMutex(prepareSellMutex);
			if (networkType == SERVER)
			{
				NetSell* sell_copy = new NetSell;
				*sell_copy = *sell;
				waitingSells.push_back(sell_copy);
			}
			unsentSells.push_back(sell);
			SDL_UnlockMutex(prepareSellMutex);
		}

		gc_ptr<Dimension::Unit> DecodeUnitID(Uint16 id)
		{
			return Dimension::HandleManager<Dimension::Unit>::InterpretIndependentHandle(id);
		}

		Packet* ClonePacket(Packet* packet);
		vector<Packet*> SplitPacket(Packet* packet);
		void SendClientFramePacket();
		void SendServerFramePacket(Uint32 frame);
		void SendRFRSPacket(Uint32 frame, int node);
		Chunk *CreateActionChunk(NetActionData *actiondata);
		int InterpretActionChunk(Chunk *chunk);
		Chunk *CreatePathChunk(NetPath *path);
		int InterpretPathChunk(Chunk *chunk);
		Chunk *CreateCreationChunk(NetCreate *create);
		int InterpretCreationChunk(Chunk *chunk);
		Chunk *CreateDamagingChunk(NetDamage *damage);
		int InterpretDamagingChunk(Chunk *chunk);
		Chunk *CreateSellChunk(NetSell *sell);
		int InterpretSellChunk(Chunk *chunk);
		void CalculateChecksum();
		Chunk *CreateChecksumChunk(Checksum* checksum_struct);
		int InterpretChecksumChunk(Chunk *chunk);
			
		Uint32 attempted_frame_count = 0;
		Uint32 attempted_frames_waited = 0;
		Uint32 bytes_sent = 0;

		std::string playerName;
		
		vector<std::string> playerNames;

		

		// CLIENT INTERFACE
		int SetClientConnect(std::string host)
		{
			SDL_LockMutex(mutClientConnect);
			if(SDLNet_ResolveHost(&clientConnect, host.c_str(), netPort) == 0)
			{
				connect = true;
				SDL_UnlockMutex(mutClientConnect);
				return SUCCESS;
			}
			else
			{
				connect = false;
				SDL_UnlockMutex(mutClientConnect);
				return ERROR_GENERAL;
			}
		}

		void JoinGame()
		{
			// Send JOIN packet
			Packet *packet = new Packet();
			packet->id[0] = 'J';
			packet->id[1] = 'O';
			packet->id[2] = 'I';
			packet->id[3] = 'N';
			packet->frame = new Uint8[nickname.length()+1];
			memcpy(packet->frame, nickname.c_str(), nickname.length()+1);
			packet->frameLength = (Uint16)nickname.length()+1;
			packet->numChunks = 0;
			packet->chunks = NULL;
			packet->node = 0;
			packet->references = 0;
			PushPacketToSend(packet);
		}

		JoinStatus joinStatus = JOIN_NOTSTARTED;

		JoinStatus GetJoinStatus()
		{
			return joinStatus;
		}

		void SendSignalPacket(const char *id, int node)
		{
			Packet *packet = new Packet();
			packet->id[0] = id[0];
			packet->id[1] = id[1];
			packet->id[2] = id[2];
			packet->id[3] = id[3];
			packet->frame = NULL;
			packet->frameLength = 0;
			packet->numChunks = 0;
			packet->chunks = NULL;
			packet->node = node;
			packet->references = 0;
			PushPacketToSend(packet);
		}

		void StartGame()
		{
			// Send STRT packet
			SDL_LockMutex(mutServerConnected);
			numClients = numConnected;
			netDestCount = playerCounter - 2;
			SDL_UnlockMutex(mutServerConnected);
			SendSignalPacket("STRT", -1);
			isReadyToStart = true;
			InitIngameNetworking();
			
			Utilities::gameTracker.UpdateGame(playerCounter + 2, netDestCount - numConnected, 0, isReadyToStart);
		}

		void CancelGame()
		{
			// Send CNCL packet
			SendSignalPacket("CNCL", -1);
		}

		// CLIENT & SERVER INTERFACE

		vector<std::string> GetPlayerNames()
		{
			return playerNames;
		}

		bool nodesReady[NETWORK_MAX_CLIENTS]; // specifies for every node whether it has finished loading the game or not

		void SendReadyMessage()
		{
			// Client: Send REDY packet
			// Server: Set an internal flag stating that the server is ready,
			// but do not send out REDY packets until all nodes are ready
			if(networkType == CLIENT)
			{
				SendSignalPacket("REDY", 0);
			}
			else if(networkType == SERVER)
			{
				SendSignalPacket("REDY", -1);
			}
		}

		void DestroyServer();

		void ShutdownNetwork()
		{
			SDL_LockMutex(mutNetworkShutdown);
			terminateNetwork = true;
			SDL_UnlockMutex(mutNetworkShutdown);
			//WAIT!
			if(networkType == CLIENT)
			{
				SDL_WaitThread(thrRecv, NULL);
				SDL_WaitThread(thrSend, NULL);
				Networking::clientConnected = false;
				Networking::joinStatus = JOIN_WAITING;
				//Networking is off.
			}
			else if(networkType == SERVER)
			{
				SDL_WaitThread(thrServerRecv, NULL);
				SDL_WaitThread(thrServerSend, NULL);
				DestroyServer();
				Utilities::gameTracker.EndGame();
			}

			//Empty In Queue of residual packets
			Packet* packet;
			while ((packet = PopReceivedPacket()))
			{
				DeletePacket(packet);
			}

			//Empty Out Queue of residual packets
			while (packetFrameOutQueue.size())
			{
				Packet *packet = packetFrameOutQueue.front();
				packetFrameOutQueue.pop();
				DeletePacket(packet);
			}
		}

		void SendRejectPacket(string error, int node)
		{
			Packet *packet = new Packet();
			packet->id[0] = 'R';
			packet->id[1] = 'J';
			packet->id[2] = 'C';
			packet->id[3] = 'T';
			if(error.length() != 0)
			{
				packet->frame = new Uint8[error.length()];
				memcpy(packet->frame, error.c_str(), error.length()+1);
			}
			else
			{
				packet->frame = NULL;
			}
			packet->frameLength = (Uint16)error.length()+1;
			packet->numChunks = 0;
			packet->chunks = NULL;
			packet->node = node;
			packet->references = 0;
			PushPacketToSend(packet);
		}

		void SendAcceptPacket(int player_id, int node)
		{
			Packet *packet = new Packet();
			packet->id[0] = 'A';
			packet->id[1] = 'C';
			packet->id[2] = 'P';
			packet->id[3] = 'T';
			packet->frame = new Uint8[1];
			packet->frame[0] = (Uint8)player_id;
			packet->frameLength = 1;
			packet->numChunks = 0;
			packet->chunks = NULL;
			packet->node = node;
			packet->references = 0;
			PushPacketToSend(packet);
		}

		void CheckNodesReady()
		{
			SDL_LockMutex(mutServerConnected);
			if(numConnected == netDestCount && isReady)
			{
				for(int i = 0; i < numConnected; i++)
				{
					if(nodesReady[i] == false)
					{
						SDL_UnlockMutex(mutServerConnected);
						break;
					}
				}
				SDL_UnlockMutex(mutServerConnected);
				SendReadyMessage();
				isReadyToStart = true;
			}
			else
			{
				SDL_UnlockMutex(mutServerConnected);
			}
		}

		void ReadyToStart()
		{
			if(networkType == CLIENT)
			{
				SendReadyMessage();
				isReady = true;
			}
			else if(networkType == SERVER)
			{
				isReady = true;
			}
		}

		void PerformPregameNetworking()
		{
			Packet* packet;
			while ((packet = PopReceivedPacket()))
			{
				if(networkType == CLIENT)
				{
					// shut down networking for RJCT and CNCL
					// for ACPT, RJCT and CNCL packet: set joinStatus accordingly
					if(joinStatus == JOIN_WAITING)
					{
						if(PACKETTYPE("ACPT"))
						{
							joinStatus = JOIN_ACCEPTED;
							//excpects a clientID.
							if(packet->frameLength == 1)
							{
								clientID = (Uint8)packet->frame[0];
							}
							else
							{
								//ERROR
								joinStatus = JOIN_FAILED;
								ShutdownNetwork();
							}
						}
						else if(PACKETTYPE("RJCT"))
						{
							if(packet->frameLength > 0)
							{
								//May contain an message of why the user was rejected.
								clientError.copy((char*)packet->frame, packet->frameLength);
							}
							joinStatus = JOIN_REJECTED;
							ShutdownNetwork();
						}
						else if(PACKETTYPE("CNCL"))
						{
							joinStatus = JOIN_CANCELLED;
							ShutdownNetwork();
						}
					}
					else if(joinStatus == JOIN_ACCEPTED)
					{
						if(PACKETTYPE("STRT"))
						{
							// for STRT packet: start game, switch state to NewGame with isNetworked = true
							// When loaded: sendReadyMessage and wait for REDY from server.
							isReadyToStart = true;
							InitIngameNetworking();
						}
					}
				}
				else if(networkType == SERVER)
				{
					// for JOIN packet on server side: send ACPT or RJCT packet
					if(nodesReady[packet->node] == false)
					{
						if(PACKETTYPE("JOIN"))
						{
							//frame data cointains nickname
							if(packet->frameLength == 0)
							{
								SendRejectPacket("No nickname specified", packet->node);
							}
							else
							{
								if(netNickname[packet->node].length() != 0)
									netNickname[packet->node].clear();

								netNickname[packet->node].copy((char*)packet->frame, packet->frameLength);
								SendAcceptPacket(playerCounter,packet->node);
								playerCounter++;
							}
						}
					}
					else
					{
						//Handle client aborts
					}
				}
			}

			// Set joinStatus to JOIN_TIMEOUT if enough time has passed since join was initiated
		}

		// returns true if it manages to apply the changes for current frame;
		// if not, the routine is to be run regularly until it does so.
		bool PerformIngameNetworking()
		{
			Packet* packet;
			attempted_frame_count++;
			unsigned queueSize = QueueSize();

			if (scheduleShutdown)
			{
				if (AI::currentFrame == shutdownFrame)
				{
					Game::Rules::GameWindow::Instance()->Stop();
				}
			}
			
			if (queueSize <= queueLimit)
			{
				bool nopackets = true;
			
				while ((packet = PopReceivedPacket()))
				{
					nopackets = true;
					if (packet->id[0] == 'F' && packet->id[1] == 'R' && packet->id[2] == 'A' && packet->id[3] == 'M' && packet->frameLength == 6)
					{
						Uint32 frame = SDLNet_Read32(packet->frame);
						Uint8 fragment = packet->frame[4];
						Uint8 num_fragments = packet->frame[5];
						int index = frame-AI::currentFrame+(netDelay<<1)-1;
						bool accept = false;
						if (fragment > 15 || num_fragments > 16)
							break;
						if (networkType == CLIENT)
						{
							if (index >= 0 && index < (signed) (netDelay<<2) && !frameFragmentsReceived[index][fragment])
							{
								frameFragmentsReceived[index][fragment] = true;
								bool all_fragments_received = true;
								accept = true;
								for (unsigned i = 0; i < num_fragments; i++)
								{
									if (!frameFragmentsReceived[index][i])
									{
										all_fragments_received = false;
										break;
									}
								}
								if (all_fragments_received)
								{
									framePacketsReceived[index] = true;
									frameMayAdvance[index] = true;
								}
							}
						}
						else if (networkType == SERVER)
						{
							if (index >= 0 && index < (signed) (netDelay<<2))
							{
								if (!individualFrameFragmentsReceived[index][packet->node][fragment])
								{
									individualFrameFragmentsReceived[index][packet->node][fragment] = true;
									bool all_fragments_received = true;
									accept = true;
									for (unsigned i = 0; i < num_fragments; i++)
									{
										if (!individualFrameFragmentsReceived[index][packet->node][i])
										{
											all_fragments_received = false;
											break;
										}
									}
									if (all_fragments_received)
									{
										individualFramePacketsReceived[index][packet->node] = true;
										bool all_recved = true;
										for (unsigned i = 0; i < numClients; i++)
										{
											if (!individualFramePacketsReceived[index][i] && nodeTypes[i] == NETWORKNODETYPE_PLAYER)
											{
												all_recved = false;
												break;
											}
										}
										if (all_recved)
										{
											framePacketsReceived[index] = true;
											frameMayAdvance[index] = true;
	#ifdef NET_DEBUG
											cout << "index " << index << endl;
	#endif
										}
									}
								}
							}
						}
						if (accept)
						{
							for (int i = 0; i < packet->numChunks; i++)
							{
								Chunk* chunk = packet->chunks[i];
								if (chunk->id[0] == 'A' && chunk->id[1] == 'C' && chunk->id[2] == 'T' && chunk->id[3] == 'N')
								{
									InterpretActionChunk(chunk);
								}
								else if (chunk->id[0] == 'P' && chunk->id[1] == 'A' && chunk->id[2] == 'T' && chunk->id[3] == 'H')
								{
									InterpretPathChunk(chunk);
								}
								else if (chunk->id[0] == 'C' && chunk->id[1] == 'H' && chunk->id[2] == 'K' && chunk->id[3] == 'S')
								{
									InterpretChecksumChunk(chunk);
								}
								else if (chunk->id[0] == 'C' && chunk->id[1] == 'R' && chunk->id[2] == 'T' && chunk->id[3] == 'E')
								{
									InterpretCreationChunk(chunk);
								}
								else if (chunk->id[0] == 'D' && chunk->id[1] == 'M' && chunk->id[2] == 'G' && chunk->id[3] == 'E')
								{
									InterpretDamagingChunk(chunk);
								}
								else if (chunk->id[0] == 'S' && chunk->id[1] == 'E' && chunk->id[2] == 'L' && chunk->id[3] == 'L')
								{
									InterpretSellChunk(chunk);
								}
							}
						}
#ifdef NET_DEBUG
						else
						{
							cout << "throwaway packet " << frame << " at frame " << AI::currentFrame << endl;
						}
						cout << "RECV ACK " << frame << " " << index << endl;
#endif
					}
					else if (packet->id[0] == 'R' && packet->id[1] == 'F' && packet->id[2] == 'R' && packet->id[3] == 'S' && packet->frameLength == 4)
					{
						Uint32 frame = SDLNet_Read32(packet->frame);
						int index;
						if (networkType == SERVER)
						{
							index = frame-AI::currentFrame+(netDelay<<1)-1;
						}
						else
						{
							index = frame-AI::currentFrame+(netDelay<<1);
						}
#ifdef NET_DEBUG
						cout << "Receive RFRS" << endl;
#endif
						if (index >= 0 && index < (signed) (netDelay<<2))
						{
							if (framePacketsSent[index])
							{
#ifdef NET_DEBUG
								cout << "ACK RFRS " << frame << " " << SDLNet_Read32(framePacketsSent[index]->frame) << endl;
#endif
								Packet* temp_packet = ClonePacket(framePacketsSent[index]);
								temp_packet->node = packet->node;
								vector<Packet*> packets = SplitPacket(temp_packet);
								for (unsigned i = 0; i < packets.size(); i++)
									PushPacketToSend(packets.at(i));
								DeletePacket(temp_packet);
								packets.clear();
							}
#ifdef NET_DEBUG
							else
							{
								cout << "NACKUNSENT RFRS " << frame << endl;
							}
#endif
						}
#ifdef NET_DEBUG
						else
						{
							cout << "NACKOOB RFRS " << frame << endl;
						}
#endif
					}
					DeletePacket(packet);
				}

				if (nopackets && !frameMayAdvance[netDelay])
				{
					int no_connections = true;
					for (unsigned i = 0; i < numClients; i++)
					{
						if (nodeTypes[i] != NETWORKNODETYPE_DISCONNECTED)
						{
							no_connections = false;
						}
					}
					if (no_connections)
					{
						frameMayAdvance[netDelay] = true;
					}
				}

			}

			if (networkType == SERVER)
			{
				for (unsigned i = 0; i < (netDelay<<1); i++)
				{
					if (!framePacketsSent[i] && framePacketsReceived[i])
					{
						if (AI::currentFrame+i+1 >= (netDelay<<1))
						{
							Uint32 frame = AI::currentFrame+i+1-(netDelay<<1);
#ifdef NET_DEBUG
							cout << "index " << i << endl;
#endif
							SendServerFramePacket(frame);
						}
					}
				}
			}
			
			if (frameMayAdvance[netDelay])
			{
				for (unsigned i = 0; i < waitingActions.size(); i++)
				{
					NetActionData* actiondata = waitingActions.at(i);
					if (actiondata->valid_at_frame <= AI::currentFrame)
					{
#ifdef NET_DEBUG
						if (actiondata->valid_at_frame < AI::currentFrame)
						{
							cout << "Packet late by " << AI::currentFrame - actiondata->valid_at_frame << " frames" << endl;
						}
#endif
						const gc_ptr<Dimension::Unit>& unit = DecodeUnitID(actiondata->unit_id);
						const gc_ptr<Dimension::Unit>& target = actiondata->goalunit_id == 0xFFFF ? gc_ptr<Dimension::Unit>() : DecodeUnitID(actiondata->goalunit_id);
						waitingActions.erase(waitingActions.begin() + i--);
						
						if (unit && (actiondata->goalunit_id == 0xFFFF || target))
						{
							if (!(actiondata->action == AI::ACTION_ATTACK ||
							      actiondata->action == AI::ACTION_FOLLOW ||
							      actiondata->action == AI::ACTION_REPAIR ||
							      actiondata->action == AI::ACTION_MOVE_ATTACK_UNIT)
							      || target)
							{
								Dimension::ActionArguments args = actiondata->arg != 0xFFFF ? actiondata->arg + Dimension::HandleTraits<Dimension::UnitType>::base : -1;
								AI::ApplyAction(unit, actiondata->action, actiondata->x, actiondata->y, target, args, ByteToRotation(actiondata->rot));
#ifdef CHECKSUM_DEBUG_HIGH
								checksum_output << "ActionData chunk on frame " << AI::currentFrame << "\n";
								checksum_output << actiondata->unit_id << " " << actiondata->goalunit_id << " " << actiondata->action << " " << actiondata->x << " " << actiondata->y << " " << actiondata->arg << " " << unit << " " << target << " " << (target ? target->pMovementData->action.action : -1) << " " << "\n";
#endif
								goto correct_actiondata;
							}
						}
#ifdef CHECKSUM_DEBUG_HIGH
						checksum_output << "Discarded ActionData chunk on frame " << AI::currentFrame << "\n";
						checksum_output << actiondata->unit_id << " " << actiondata->goalunit_id << " " << actiondata->action << " " << actiondata->x << " " << actiondata->y << " " << actiondata->arg << " " << unit << " " << target << " " << (target ? target->pMovementData->action.action : -1) << " " << "\n";
#endif
						correct_actiondata:

						delete actiondata;
					}
				}
				for (unsigned i = 0; i < waitingPaths.size(); i++)
				{
					NetPath* path = waitingPaths.at(i);
					if (path->valid_at_frame <= AI::currentFrame)
					{
						const gc_ptr<Dimension::Unit>& unit = DecodeUnitID(path->unit_id);
						waitingPaths.erase(waitingPaths.begin() + i--);
						
						if (unit)
						{
							AI::DeallocPathfindingNodes(unit);
							unit->pMovementData->pStart = path->pStart;
							unit->pMovementData->pGoal = path->pGoal;
							unit->pMovementData->pCurGoalNode = NULL;
#ifdef CHECKSUM_DEBUG_HIGH
							checksum_output << "Path chunk on frame " << AI::currentFrame << "\n";
							checksum_output << path->unit_id << "\n";
#endif
						}
						delete path;
					}
				}

				for (unsigned i = 0; i < waitingCreations.size(); i++)
				{
					NetCreate* create = waitingCreations.at(i);
					if (create->valid_at_frame <= AI::currentFrame)
					{
						waitingCreations.erase(waitingCreations.begin() + i--);
						const gc_ptr<Dimension::Player>& owner = Dimension::HandleManager<Dimension::Player>::InterpretIndependentHandle(create->owner_id);
						const gc_ptr<Dimension::UnitType>& type = Dimension::HandleManager<Dimension::UnitType>::InterpretIndependentHandle(create->unittype_id);
						if (owner && type)
						{
							
							const gc_ptr<Dimension::Unit>& unit = Dimension::CreateUnit(type, create->x, create->y);
							if (unit)
							{
								unit->rotation = ByteToRotation(create->rot);
							}
#ifdef CHECKSUM_DEBUG_HIGH
							checksum_output << "Creation chunk on frame " << AI::currentFrame << "\n";
							checksum_output << create->unittype_id << " " << create->owner_id << " " << create->x << " " << create->y << "\n";
#endif
						}
						delete create;
					}
				}

				for (unsigned i = 0; i < waitingDamagings.size(); i++)
				{
					NetDamage* damage = waitingDamagings.at(i);
					if (damage->valid_at_frame <= AI::currentFrame)
					{
						gc_ptr<Dimension::Unit> unit = DecodeUnitID(damage->unit_id);
						waitingDamagings.erase(waitingDamagings.begin() + i--);
						
						if (unit)
						{
							Dimension::Attack(unit, (float) damage->damage / 100);
#ifdef CHECKSUM_DEBUG_HIGH
							checksum_output << "Damaging chunk on frame " << AI::currentFrame << "\n";
							checksum_output << damage->unit_id << " " << damage->damage << "\n";
#endif
						}
						delete damage;
					}
				}

				for (unsigned i = 0; i < waitingSells.size(); i++)
				{
					NetSell* sell = waitingSells.at(i);
					if (sell->valid_at_frame <= AI::currentFrame)
					{
						waitingSells.erase(waitingSells.begin() + i--);
						
						const gc_ptr<Dimension::Player>& owner = Dimension::HandleManager<Dimension::Player>::InterpretIndependentHandle(sell->owner_id);
						if (owner)
						{
							Dimension::SellPower(Dimension::pWorld->vPlayers.at(sell->owner_id), sell->amount);
#ifdef CHECKSUM_DEBUG_HIGH
							checksum_output << "Sell chunk on frame " << AI::currentFrame << "\n";
							checksum_output << sell->owner_id << " " << sell->amount << "\n";
#endif
						}
						delete sell;
					}
				}
				if (framePacketsSent[0])
				{
					framePacketsSent[0]->references--;
					if (!framePacketsSent[0]->references)
						DeletePacket(framePacketsSent[0]);
				}

				for (unsigned i = 0; i < (netDelay<<2)-1; i++)
				{
					framePacketsReceived[i] = framePacketsReceived[i+1];
					frameMayAdvance[i] = frameMayAdvance[i+1];
					framePacketsSent[i] = framePacketsSent[i+1];
					if (networkType == SERVER)
					{
						for (unsigned j = 0; j < numClients; j++)
						{
							individualFramePacketsReceived[i][j] = individualFramePacketsReceived[i+1][j];
							for (unsigned k = 0; k < 16; k++)
							{
								individualFrameFragmentsReceived[i][j][k] = individualFrameFragmentsReceived[i+1][j][k];
							}
						}
					}
					for (unsigned j = 0; j < 15; j++)
					{
						frameFragmentsReceived[i][j] = frameFragmentsReceived[i+1][j];
					}

				}
				
				framePacketsReceived[(netDelay<<2)-1] = false;
				frameMayAdvance[(netDelay<<2)-1] = false;
				framePacketsSent[(netDelay<<2)-1] = NULL;
				frameRFRSSentAt = SDL_GetTicks();
				
				if (networkType == SERVER)
				{
					for (unsigned j = 0; j < numClients; j++)
					{
						individualFramePacketsReceived[(netDelay<<2)-1][j] = false;
						for (unsigned k = 0; k < 16; k++)
						{
							individualFrameFragmentsReceived[(netDelay<<2)-1][j][k] = false;
						}
					}
				}
				for (unsigned j = 0; j < 16; j++)
				{
					frameFragmentsReceived[(netDelay<<2)-1][j] = false;
				}
#ifdef CHECKSUM_DEBUG_HIGH
				CalculateChecksum();
#else
				if (AI::currentFrame % 20 == 0)
				{
					CalculateChecksum();
				}
#endif

				if (networkType == CLIENT)
				{
					SendClientFramePacket();
				}

#ifdef NET_DEBUG
				cout << "ADVANCE " << AI::currentFrame+1 << endl;
#endif

				return true;
			}
			else if (queueSize <= queueLimit)
			{
				if (SDL_GetTicks() - frameRFRSSentAt >= 100)
				{
#ifdef NET_DEBUG
					cout << "SEND RFRS " << AI::currentFrame-netDelay+1 << endl;
#endif
					if (networkType == SERVER)
					{
						for (unsigned i = 0; i < numClients; i++)
						{
							if (nodeTypes[i] == NETWORKNODETYPE_PLAYER)
							{
								SendRFRSPacket(AI::currentFrame-netDelay+1, i);
							}
						}
					}
					else
					{
						SendRFRSPacket(AI::currentFrame-netDelay+1, -1);
					}
					frameRFRSSentAt = SDL_GetTicks();
				}
			}
			
#ifdef NET_DEBUG
			cout << "wait " << AI::currentFrame << endl;
#endif
			attempted_frames_waited++;

			return false;
		}

		vector<Packet*> SplitPacket(Packet* packet)
		{
			vector<Packet*> packets;
			vector<Chunk*> selected_chunks;
			Packet* cur_packet;
			Uint16 chunk_index = 0;
			while (chunk_index < packet->numChunks || packets.size() == 0)
			{
				int cur_size =   6 // checksum + packet length
				               + 4 // ID
				               + 2 // numChunks
					       + 2 // frameLength
					       + packet->frameLength // frame data
					       + 4; // extra frame data; numpacket
				cur_packet = new Packet;
				cur_packet->id[0] = packet->id[0];
				cur_packet->id[1] = packet->id[1];
				cur_packet->id[2] = packet->id[2];
				cur_packet->id[3] = packet->id[3];
				cur_packet->frameLength = packet->frameLength + 2; // original frame + packet id + total packets in frame
				cur_packet->frame = new BUFFER[packet->frameLength + 2];
				memcpy(cur_packet->frame, packet->frame, packet->frameLength);
				cur_packet->node = packet->node;
				cur_packet->references = 0;
				for (; chunk_index < packet->numChunks; chunk_index++)
				{
					int chunk_size =   4 // ID
					                 + 2 // length
							 + packet->chunks[chunk_index]->length; // chunk length
					if (chunk_size > 64000)
					{
						chunk_index++;
						break;
					}
					else if (cur_size + chunk_size > 65000)
					{
						break;
					}
					Chunk* chunk_copy = new Chunk;
					chunk_copy->id[0] = packet->chunks[chunk_index]->id[0];
					chunk_copy->id[1] = packet->chunks[chunk_index]->id[1];
					chunk_copy->id[2] = packet->chunks[chunk_index]->id[2];
					chunk_copy->id[3] = packet->chunks[chunk_index]->id[3];
					chunk_copy->length = packet->chunks[chunk_index]->length;
					chunk_copy->data = new BUFFER[chunk_copy->length];
					memcpy(chunk_copy->data, packet->chunks[chunk_index]->data, chunk_copy->length);
					selected_chunks.push_back(chunk_copy);
					cur_size += chunk_size;
				}
				cur_packet->chunks = new Chunk*[selected_chunks.size()];
				cur_packet->numChunks = selected_chunks.size();
				for (unsigned i = 0; i < selected_chunks.size(); i++)
				{
					cur_packet->chunks[i] = selected_chunks.at(i);
				}
				selected_chunks.clear();
				packets.push_back(cur_packet);
				if (packets.size() == 16)
				{
					cout << "OMFG SO MANY PACKETS, SKIP ZE REST" << endl;
					break;
				}
			}
			if (packets.size() > 1)
			{
				cout << "Split packet into " << packets.size() << " fragments" << endl;
			}
			for (unsigned i = 0; i < packets.size(); i++)
			{
				*(packets.at(i)->frame + packets.at(i)->frameLength - 1) = packets.size();
				*(packets.at(i)->frame + packets.at(i)->frameLength - 2) = i;
			}
			return packets;
		}

		void SendClientFramePacket()
		{
			unsigned i = 0, j;
			Packet* packet = new Packet;
			packet->id[0] = 'F';
			packet->id[1] = 'R';
			packet->id[2] = 'A';
			packet->id[3] = 'M';
			packet->numChunks = unsentActions.size() + unsentPaths.size() + unsentCreations.size() + unsentDamagings.size() + unsentSells.size() + unsentChecksums.size();
			packet->frame = new BUFFER[sizeof(Uint32)];
			SDLNet_Write32(AI::currentFrame, packet->frame);
#ifdef NET_DEBUG
			cout << "SEND " << AI::currentFrame << endl;
#endif
			packet->frameLength = sizeof(Uint32);
			packet->chunks = new Chunk*[packet->numChunks];
			packet->references = 1;
			packet->node = -1;
			for (; i < unsentActions.size(); i++)
			{
				Chunk *chunk = CreateActionChunk(unsentActions.at(i));
				delete unsentActions.at(i);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i] = chunk;
			}
			for (j = 0; j < unsentPaths.size(); j++)
			{
				Chunk *chunk = CreatePathChunk(unsentPaths.at(j));
				delete unsentPaths.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i+= j;
			for (j = 0; j < unsentCreations.size(); j++)
			{
				Chunk *chunk = CreateCreationChunk(unsentCreations.at(j));
				delete unsentCreations.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i+= j;
			for (j = 0; j < unsentDamagings.size(); j++)
			{
				Chunk *chunk = CreateDamagingChunk(unsentDamagings.at(j));
				delete unsentDamagings.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i+= j;
			for (j = 0; j < unsentSells.size(); j++)
			{
				Chunk *chunk = CreateSellChunk(unsentSells.at(j));
				delete unsentSells.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i+= j;
			for (j = 0; j < unsentChecksums.size(); j++)
			{
				Chunk *chunk = CreateChecksumChunk(unsentChecksums.at(j));
				delete unsentChecksums.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			unsentActions.clear();
			unsentPaths.clear();
			unsentCreations.clear();
			unsentDamagings.clear();
			unsentChecksums.clear();
			unsentSells.clear();
			framePacketsSent[(netDelay<<1)-1] = packet;

			vector<Packet*> packets = SplitPacket(packet);
			for (unsigned i = 0; i < packets.size(); i++)
				PushPacketToSend(packets.at(i));
		}

		void SendServerFramePacket(Uint32 frame)
		{
			unsigned i = 0, j;
			vector<NetActionData*> actions;
			vector<NetPath*> paths;
			vector<NetCreate*> creations;
			vector<NetDamage*> damagings;
			vector<NetSell*> sells;
			Packet* packet = new Packet;
			packet->id[0] = 'F';
			packet->id[1] = 'R';
			packet->id[2] = 'A';
			packet->id[3] = 'M';

			for (unsigned k = 0; k < unsentActions.size(); k++)
			{
				if (unsentActions.at(k)->valid_at_frame == frame + netDelay)
				{
					actions.push_back(unsentActions.at(k));
					unsentActions.erase(unsentActions.begin() + k--);
				}
			}

			for (unsigned k = 0; k < unsentPaths.size(); k++)
			{
				if (unsentPaths.at(k)->valid_at_frame == frame + netDelay)
				{
					paths.push_back(unsentPaths.at(k));
					unsentPaths.erase(unsentPaths.begin() + k--);
				}
			}

			for (unsigned k = 0; k < unsentCreations.size(); k++)
			{
				if (unsentCreations.at(k)->valid_at_frame == frame + netDelay)
				{
					creations.push_back(unsentCreations.at(k));
					unsentCreations.erase(unsentCreations.begin() + k--);
				}
			}

			for (unsigned k = 0; k < unsentDamagings.size(); k++)
			{
				if (unsentDamagings.at(k)->valid_at_frame == frame + netDelay)
				{
					damagings.push_back(unsentDamagings.at(k));
					unsentDamagings.erase(unsentDamagings.begin() + k--);
				}
			}

			for (unsigned k = 0; k < unsentSells.size(); k++)
			{
				if (unsentSells.at(k)->valid_at_frame == frame + netDelay)
				{
					sells.push_back(unsentSells.at(k));
					unsentSells.erase(unsentSells.begin() + k--);
				}
			}

			packet->numChunks = actions.size() + paths.size() + creations.size() + damagings.size() + sells.size() + unsentChecksums.size();
			packet->frame = new BUFFER[sizeof(Uint32)];
			SDLNet_Write32(frame, packet->frame);
#ifdef NET_DEBUG
			cout << "SERVSEND " << frame << endl;
#endif
			packet->frameLength = sizeof(Uint32);
			packet->chunks = new Chunk*[packet->numChunks];
			packet->references = 1;
			packet->node = -1;
			for (; i < actions.size(); i++)
			{
				Chunk *chunk = CreateActionChunk(actions.at(i));
				delete actions.at(i);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i] = chunk;
			}
			for (j = 0; j < paths.size(); j++)
			{
				Chunk *chunk = CreatePathChunk(paths.at(j));
				delete paths.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i += j;
			for (j = 0; j < creations.size(); j++)
			{
				Chunk *chunk = CreateCreationChunk(creations.at(j));
				delete creations.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i += j;
			for (j = 0; j < damagings.size(); j++)
			{
				Chunk *chunk = CreateDamagingChunk(damagings.at(j));
				delete damagings.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i += j;
			for (j = 0; j < sells.size(); j++)
			{
				Chunk *chunk = CreateSellChunk(sells.at(j));
				delete sells.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			i+= j;
			for (j = 0; j < unsentChecksums.size(); j++)
			{
				Chunk *chunk = CreateChecksumChunk(unsentChecksums.at(j));
				delete unsentChecksums.at(j);
				if (!chunk)
				{
					return;
				}
				packet->chunks[i + j] = chunk;
			}
			unsentChecksums.clear();
			if (frame-AI::currentFrame+(netDelay<<1)-1 >= netDelay<<2 || (signed)frame-(signed)AI::currentFrame+(signed)(netDelay<<1)-1 < 0)
				cout << "sakdgajskfgsa" << endl;
			else
				framePacketsSent[frame-AI::currentFrame+(netDelay<<1)-1] = packet;

			vector<Packet*> packets = SplitPacket(packet);
			for (unsigned i = 0; i < packets.size(); i++)
				PushPacketToSend(packets.at(i));
		}

		void SendRFRSPacket(Uint32 frame, int node)
		{
			Packet* packet = new Packet;
			packet->id[0] = 'R';
			packet->id[1] = 'F';
			packet->id[2] = 'R';
			packet->id[3] = 'S';
			packet->chunks = NULL;
			packet->numChunks = 0;
			packet->frame = new BUFFER[sizeof(Uint32)];
			packet->frameLength = sizeof(Uint32);
			SDLNet_Write32(frame, packet->frame);
			packet->references = 0;
			packet->node = node;
			PushPacketToSend(packet);
		}

		int InitClient(NetworkSocket *net, int port)
		{
			clientID = 0;
			clientError = "";
			net->pBufferIn = new Uint8[NETWORK_BUFFER];
			net->pBufferOut = new Uint8[NETWORK_BUFFER];
			net->set = SDLNet_AllocSocketSet(1);
			connect = false;

			Networking::clientConnected = false;
			Networking::joinStatus = JOIN_WAITING;

			nodeTypes[0] = NETWORKNODETYPE_PLAYER;

			thrRecv = SDL_CreateThread(&_networkRecvThread, net);
			thrSend = SDL_CreateThread(&_networkSendThread, net);
			return SUCCESS;
		}

		int InitServer(NetworkSocket *net, int port)
		{
			terminateNetwork = false;
			numConnected = 0;
			serverListening = true;

			net->pBufferIn = new Uint8[NETWORK_BUFFER];
			net->pBufferOut = new Uint8[NETWORK_BUFFER];
			net->set = SDLNet_AllocSocketSet(netDestCount);
			IPaddress adr;
			adr.host = INADDR_ANY;
			SDLNet_Write16(port, &adr.port);
			netDestBufferIn = new Uint8*[netDestCount];
			for(int i = 0; i < netDestCount; i++)
				netDestBufferIn[i] = new Uint8[NETWORK_BUFFER];

			netDataLeft = new int[netDestCount];
			netDataTotal = new int[netDestCount];
			netBufferStart = new int[netDestCount];
			netBufferEnd = new int[netDestCount];
			for(int i = 0; i < netDestCount; i++)
			{
				netDataLeft[i] = -1;
				netDataTotal[i] = 0;
				netBufferStart[i] = 0;
				netBufferEnd[i] = 0;
			}

			net->socket = SDLNet_TCP_Open(&adr);
			if(!net->socket)
			{
				cout << "Failed to open Server: " << SDLNet_GetError() << endl;
				return ERROR_GENERAL;
			}
			thrServerRecv = SDL_CreateThread(&_networkServerRecvThread, net);
			thrServerSend = SDL_CreateThread(&_networkServerSendThread, net);
			return SUCCESS;
		}

		void DestroyServer()
		{
			delete [] netDataLeft;
			delete [] netDataTotal;
			delete [] netBufferStart;
			delete [] netBufferEnd;
			for(int i = 0; i < netDestCount; i++)
				delete [] netDestBufferIn[i];

			delete [] netDestBufferIn;
		}


		void InitIngameNetworking()
		{
			framePacketsReceived = new bool[netDelay<<2];
			frameFragmentsReceived = new bool*[netDelay<<2];
			frameMayAdvance = new bool[netDelay<<2];
			framePacketsSent = new Packet*[netDelay<<2];

			if (networkType == SERVER)
			{
				individualFramePacketsReceived = new bool*[netDelay<<2];
				individualFrameFragmentsReceived = new bool**[netDelay<<2];
			}

			for (unsigned i = 0; i < netDelay<<2; i++)
			{
				framePacketsReceived[i] = false;
				frameMayAdvance[i] = (i < (netDelay<<1)-1);
				framePacketsSent[i] = NULL;

				if (networkType == SERVER)
				{
					individualFramePacketsReceived[i] = new bool[numClients];
					individualFrameFragmentsReceived[i] = new bool*[numClients];
					for (unsigned j = 0; j < numClients; j++)
					{
						individualFramePacketsReceived[i][j] = false;
						individualFrameFragmentsReceived[i][j] = new bool[16];
						for (unsigned k = 0; k < 16; k++)
						{
							individualFrameFragmentsReceived[i][j][k] = false;
						}
					}
				}
				frameFragmentsReceived[i] = new bool[16];
				for (unsigned j = 0; j < 16; j++)
				{
					frameFragmentsReceived[i][j] = false;
				}
			}
			if (networkType == SERVER)
			{
				for (unsigned i = 2; i < numClients+1; i++)
				{
					Dimension::pWorld->vPlayers.at(i)->isRemote = true;
				}
				Dimension::currentPlayer = Dimension::pWorld->vPlayers.at(1);
				Dimension::currentPlayerView = Dimension::pWorld->vPlayers.at(1);
			}
			else
			{
				for (unsigned i = 0; i < Dimension::pWorld->vPlayers.size(); i++)
				{
					if (i != clientID)
						Dimension::pWorld->vPlayers.at(i)->isRemote = true;
				}
				Dimension::currentPlayer = Dimension::pWorld->vPlayers.at(clientID);
				Dimension::currentPlayerView = Dimension::pWorld->vPlayers.at(clientID);
			}
			
			if (Dimension::currentPlayer->vUnits.size() > 0)
			{
				const gc_ptr<Dimension::Unit>& unit = Dimension::currentPlayer->vUnits.at(0);
				
				if (unit)
				{
					Dimension::Camera* camera = &Game::Dimension::Camera::instance;
					camera->SetCamera(unit, camera->GetZoom(), camera->GetRotation());
				}
			}
			
			frameRFRSSentAt = SDL_GetTicks();
		}

		int StartNetwork(NETWORKTYPE type)
		{
			prepareActionMutex = SDL_CreateMutex();
			prepareCreationMutex = SDL_CreateMutex();
			prepareDamagingMutex = SDL_CreateMutex();
			prepareSellMutex = SDL_CreateMutex();

			mutPacketInQueue = SDL_CreateMutex();
			mutPacketOutQueue = SDL_CreateMutex();
			mutPacketFrameOutQueue = SDL_CreateMutex();
			mutServerConnected = SDL_CreateMutex();
			mutClientConnect = SDL_CreateMutex();
			mutNetworkShutdown = SDL_CreateMutex();
			terminateNetwork = false;
			CRC32_init();

			netDestCount = Dimension::pWorld->vPlayers.size()-2;

			NetworkSocket* net = new NetworkSocket();
			net->socket = NULL;
			connect = false;

#ifdef NET_DEBUG_CONNECTION
			NetworkSocket* netServer = new NetworkSocket();
			connect = true;
#endif
			networkType = type;

			if (Game::Rules::checksumLog.length() == 0)
			{
				if (type == SERVER)
				{
					Game::Rules::checksumLog = "checksum-server.log";
				}
				else if (type == CLIENT)
				{
					Game::Rules::checksumLog = "checksum-client.log";
				}
			}
#ifdef CHECKSUM_DEBUG
			checksum_output.SetFile(Game::Rules::checksumLog.c_str());
#endif

			if(type == CLIENT)
			{
				int ret = InitClient(net, netPort); 
				if(ret != SUCCESS)
					return ret;

#ifdef NET_DEBUG_CONNECTION
				ret = InitServer(netServer, netPort);
				if(ret != SUCCESS)
					return ret;
#else
			}
			else if(type == SERVER)
			{
				int ret = InitServer(net, netPort);
				if(ret != SUCCESS)
					return ret;
#endif
			}
			if (type == SERVER)
			{
				Utilities::gameTracker.BeginGame(playerCounter + 2, netDestCount - numConnected, 0);
			}
			return SUCCESS;
		}

		int GetDataInBuffer(int start, int end)
		{
			if(end - start < 0)
			{
				int amount = NETWORK_BUFFER - start;
				return amount + end;
			}
			else
			{
				return end - start;
			}
		}

		int GetDataInBuffer(int id)
		{
			if(netBufferEnd[id] - netBufferStart[id] < 0)
			{
				int amount = NETWORK_BUFFER - netBufferStart[id];
				return amount + netBufferEnd[id];
			}
			else
			{
				return netBufferEnd[id] - netBufferStart[id];
			}
		}

		void ExtractDataFromBuffer(Uint8 *pBuffer, int len, Uint8 *pSrc, int& start, int end)
		{
			if(start + len > NETWORK_BUFFER)
			{
				int amount = NETWORK_BUFFER - start;
				memcpy(pBuffer, &pSrc[start], amount);
				start = 0;
				memcpy(&pBuffer[amount], pSrc, len - amount);
				start += len - amount;	
#ifdef NET_DEBUG_CONNECTION
				cout << "CLIENT RECV: ExtractDataFromBuffer, buffer restart." << endl;
#endif
			}
			else
			{
				memcpy(pBuffer, pSrc + start, len);
				start += len;
				if(start == NETWORK_BUFFER)
					start = 0;
			}
		}

		void ExtractDataFromBuffer(Uint8 *pBuffer, int len, int id)
		{
			if(netBufferStart[id] + len > NETWORK_BUFFER)
			{
				int amount = NETWORK_BUFFER - netBufferStart[id];
				memcpy(pBuffer, &netDestBufferIn[id][netBufferStart[id]], amount);
				netBufferStart[id] = 0;
				memcpy(&pBuffer[amount], netDestBufferIn[id], len - amount);
				netBufferStart[id] += len - amount;
#ifdef NET_DEBUG_CONNECTION
				cout << "SERVER RECV: ExtractDataFromBuffer, buffer restart." << endl;
#endif
			}
			else
			{
				memcpy(pBuffer, netDestBufferIn[id] + netBufferStart[id], len);
				netBufferStart[id] += len;
				if(netBufferStart[id] == NETWORK_BUFFER)
					netBufferStart[id] = 0;
			}
		}

		void AppendDataToBuffer(Uint8 *pBufferIn, int len, Uint8 *pSrc, int start, int& end)
		{
			if(end + len > NETWORK_BUFFER)
			{
				int amount = NETWORK_BUFFER - end;
				memcpy(&pSrc[end], pBufferIn, amount);
				end = 0;
				memcpy(pSrc, &pBufferIn[amount], len - amount);
				end += len - amount;
#ifdef NET_DEBUG_CONNECTION
				cout << "CLIENT RECV: AppendDataToBuffer, buffer restart." << endl;
#endif
			}
			else
			{
				memcpy(pSrc + end, pBufferIn, len);
				end += len;
				if(end == NETWORK_BUFFER)
					end = 0;
			}
		}

		void AppendDataToBuffer(Uint8 *pBufferIn, int len, int id)
		{
			if(netBufferEnd[id] + len > NETWORK_BUFFER)
			{
				int amount = NETWORK_BUFFER - netBufferEnd[id];
				memcpy(&netDestBufferIn[id][netBufferEnd[id]], pBufferIn, amount);
				netBufferEnd[id] = 0;
				memcpy(netDestBufferIn[id], &pBufferIn[amount], len - amount);
				netBufferEnd[id] += len - amount;
#ifdef NET_DEBUG_CONNECTION
				cout << "SERVER RECV: AppendDataToBuffer, buffer restart." << endl;
#endif
			}
			else
			{
				memcpy(netDestBufferIn[id] + netBufferEnd[id], pBufferIn, len);
				netBufferEnd[id] += len;
				if(netBufferEnd[id] == NETWORK_BUFFER)
					netBufferEnd[id] = 0;
			}
		}

		int _networkServerRecvThread(void* arg)
		{
			NetworkSocket *net = (NetworkSocket*)arg;
			TCPsocket acceptSock = NULL;
#ifdef NET_DEBUG_CONNECTION
			bool activty = false;
#endif
			while(true)
			{
				SDL_LockMutex(mutNetworkShutdown);
				if(terminateNetwork)
				{
					SDL_UnlockMutex(mutNetworkShutdown);
					break;
				}
				SDL_UnlockMutex(mutNetworkShutdown);

				acceptSock = SDLNet_TCP_Accept(net->socket);
				if(acceptSock != NULL)
				{
					if(!SDLNet_TCP_AddSocket(net->set, acceptSock))
					{
						cout << "Failed to add to Set" << endl;
					}
					PendingConnection a;
					a.socket = acceptSock;
					pendingSockets.push_back(a);
				}

				int nb = SDLNet_CheckSockets(net->set, 0);
				if(nb > 0)
				{
#ifdef NET_DEBUG_CONNECTION
					activty = true;
#endif
					for (std::vector<PendingConnection>::iterator it = pendingSockets.begin(); it != pendingSockets.end(); )
					{
						if(SDLNet_SocketReady(it->socket))
						{
							char buffer[32];
							(it->received) += std::string(buffer,SDLNet_TCP_Recv(it->socket, buffer, 4 - it->received.length()));
							if (it->received.length() >= 4)
							{
								if (it->received == "PLAY")
								{
									netDest[numConnected] = acceptSock;
									nodeTypes[numConnected] = NETWORKNODETYPE_PLAYER;
									SDL_LockMutex(mutServerConnected);
									numConnected++;
									if(numConnected == netDestCount)
										serverListening = false;

									Utilities::gameTracker.UpdateGame(playerCounter + 2, netDestCount - numConnected, 0, isReadyToStart);

									SDL_UnlockMutex(mutServerConnected);

									it = pendingSockets.erase(it);
								}
								else if (it->received.substr(0, 4) == "PING")
								{
									it->received += std::string(buffer, SDLNet_TCP_Recv(it->socket, buffer, 24 - it->received.length()));
									if (it->received.length() == 24)
									{
										if (Utilities::gameTracker.IsPrefixOfSecretGameID(it->received.substr(4, 20)))
										{
											std::string lastPart = Utilities::gameTracker.GetLastPartOfSecretGameId();
											SDLNet_TCP_Send(it->socket, lastPart.c_str(), 20);
										}
									}
									SDLNet_TCP_AddSocket(net->set, it->socket);
									SDLNet_TCP_Close(it->socket);
									it = pendingSockets.erase(it);
								}
								else
								{
									it++;
								}
							}
							else
							{
								it++;
							}
						}
						else
						{
							it++;
						}
					}

					for(int i = 0; i < numConnected; i++)
					{
						if(SDLNet_SocketReady(netDest[i]))
						{
							int left = NETWORK_RECEIVE_CHUNK;
							if(GetDataInBuffer(i) + NETWORK_RECEIVE_CHUNK > NETWORK_BUFFER)
								left = NETWORK_BUFFER - GetDataInBuffer(i);

							if(left > 0)
							{
								int packetRead = SDLNet_TCP_Recv(netDest[i], net->pBufferIn, left);
								if(packetRead <= 0)
									//FATAL ERROR, reconnect is a good thing todo.
									continue;

								AppendDataToBuffer(net->pBufferIn, packetRead, i);
							}
						}

						bool loop = true;
						while(loop)
						{
							if(netDataLeft[i] == -1)
							{
								if(GetDataInBuffer(i) >= 2)
								{
									Uint8 buff[2];
									ExtractDataFromBuffer((Uint8*)&buff, 2, i);
									netDataLeft[i] = netDataTotal[i] = SDLNet_Read16(&buff);
									if(netDataLeft[i] == 0)
									{
										netDataLeft[i] = -1;
									}
								}
								else
								{
									loop = false;
								}
							}
							
							if(netDataLeft[i] > 0)
							{
								//A packet exists in buffer..
								if(netDataLeft[i] - GetDataInBuffer(i) <= 0)
								{
									ExtractDataFromBuffer(net->pBufferIn, netDataLeft[i], i);
									netDataLeft[i] = -1;
									Packet *pPacket = ProcessPacket(net->pBufferIn, netDataTotal[i]);
									if(pPacket != NULL)
									{
										pPacket->node = i;
#ifdef NET_DEBUG
										if(pPacket->frameLength >= 4)
										{
											cout << "SERVER RECV Packet: " << pPacket->id[0] << pPacket->id[1] << pPacket->id[2] << pPacket->id[3] << " FrameData:" << SDLNet_Read32(pPacket->frame) << endl;
										}
										else
										{
											cout << "SERVER RECV Packet: " << pPacket->id[0] << pPacket->id[1] << pPacket->id[2] << pPacket->id[3] << " Total packet len: " << netDataTotal[i] << endl;
										}
#endif	
										SDL_LockMutex(mutPacketInQueue);
										packetInQueue.push(pPacket);
										SDL_UnlockMutex(mutPacketInQueue);
										loop = true;
									}
									else
									{
#ifdef NET_DEBUG
										cout << "SERVER RECV Packet: failed checksum" << endl;
#endif
									}
								}
								else
								{
									loop = false;
								}
							}
						}
					}
				}
				else
				{
#ifdef NET_DEBUG_CONNECTION
					if(activty != false)
					{
#ifdef NET_DEBUG
						cout << "No network server activity inbound..." << endl;
#endif
						activty = false;
					}
#endif
					SDL_Delay(1);
				}

			}
			return SUCCESS;
		}

		void SendPacket(Packet* packet, Uint8 *pBuffer, int packetLen)
		{
			//TODO send to correct address and location.
			int result = 0;

			if(packet->node == -1)
			{
				SDL_LockMutex(mutServerConnected);
				int connectedClients = numConnected;
				SDL_UnlockMutex(mutServerConnected);

				//Broadcast
				for(int i = 0; i < connectedClients; i++)
				{
					if (nodeTypes[i] != NETWORKNODETYPE_DISCONNECTED)
					{
						bytes_sent += packetLen;
						result = SDLNet_TCP_Send(netDest[i], pBuffer, packetLen);
						if(!result || result < packetLen) 
						{
							cout << "NetOUT Failed: " << SDLNet_GetError() << endl;
							nodeTypes[i] = NETWORKNODETYPE_DISCONNECTED;
						}
					}
				}
			}
			else
			{
				if (nodeTypes[packet->node] != NETWORKNODETYPE_DISCONNECTED)
				{
					bytes_sent += packetLen;
					result = SDLNet_TCP_Send(netDest[packet->node], pBuffer, packetLen);
					if(!result || result < packetLen) 
					{
						cout << "NetOUT Failed: " << SDLNet_GetError() << endl;
						nodeTypes[packet->node] = NETWORKNODETYPE_DISCONNECTED;
					}
#ifdef NET_DEBUG
					if(packet->frameLength >= 4)
					{
						cout << "SERVER SEND Packet: " << packet->id[0] << packet->id[1] << packet->id[2] << packet->id[3] << " FrameData:" << SDLNet_Read32(packet->frame) << endl;
					}
					else
					{
						cout << "SERVER SEND Packet: " << packet->id[0] << packet->id[1] << packet->id[2] << packet->id[3] << " Total packet len: " << packetLen - 2 << endl;
					}
#endif
				}
			}
		}

		void SendPacket(TCPsocket sock, Packet* packet, Uint8 *pBuffer, int packetLen)
		{
			//TODO send to correct address and location.
			int result = 0;

			bytes_sent += packetLen;
			result = SDLNet_TCP_Send(sock, pBuffer, packetLen);
			if(!result || result < packetLen) 
			{
				cout << "NetOUT Failed: " << SDLNet_GetError() << endl;
			}
#ifdef NET_DEBUG
			if(packet->frameLength >= 4)
			{
				cout << "CLIENT SEND Packet: " << packet->id[0] << packet->id[1] << packet->id[2] << packet->id[3] << " FrameData:" << SDLNet_Read32(packet->frame) << endl;
			}
			else
			{
				cout << "CLIENT SEND Packet: " << packet->id[0] << packet->id[1] << packet->id[2] << packet->id[3] << " Total packet len: " << packetLen - 2 << endl;
			}
#endif
		}

		int _networkServerSendThread(void* arg)
		{
			NetworkSocket *net = (NetworkSocket*)arg;
			int count = 0;
			while(true)
			{
				SDL_LockMutex(mutNetworkShutdown);
				if(terminateNetwork)
				{
					SDL_UnlockMutex(mutNetworkShutdown);
					break;
				}
				SDL_UnlockMutex(mutNetworkShutdown);

				SDL_LockMutex(mutPacketFrameOutQueue);
				while (packetFrameOutQueue.size())
				{
					Packet *packet = packetFrameOutQueue.front();
					bool scheduleDeletion = false;
					if (!packet)
						*(int*) 0 = 0;
					packetFrameOutQueue.pop();
					packet->references--;
					if (!packet->references)
						scheduleDeletion = true;
					SDL_UnlockMutex(mutPacketFrameOutQueue);

					int packetLen = CreatePacket(net->pBufferOut, packet);
//					bytes_sent += packetLen;

					SendPacket(packet, net->pBufferOut, packetLen);

					if (scheduleDeletion)
						DeletePacket(packet);

					SDL_LockMutex(mutPacketFrameOutQueue);
				}
				SDL_UnlockMutex(mutPacketFrameOutQueue);

				count = 0;
				SDL_LockMutex(mutPacketOutQueue);
				while (packetOutQueue.size())
				{
					Packet *packet = packetOutQueue.front();
					bool scheduleDeletion = false;
					packetOutQueue.pop();
					packet->references--;
					if (!packet->references)
						scheduleDeletion = true;
					SDL_UnlockMutex(mutPacketOutQueue);

					int packetLen = CreatePacket(net->pBufferOut, packet);

//					bytes_sent += packetLen;

					SendPacket(packet, net->pBufferOut, packetLen);

					if (scheduleDeletion)
						DeletePacket(packet);

					count++;

					SDL_LockMutex(mutPacketOutQueue);

				}
				SDL_UnlockMutex(mutPacketOutQueue);
				SDL_Delay(1);
			}
			return SUCCESS;
		}

		bool isClientConnected()
		{
			SDL_LockMutex(mutClientConnect);
			bool clint = clientConnected;
			SDL_UnlockMutex(mutClientConnect);
			return clint;
		}

		int _networkRecvThread(void* arg)
		{
			//Perform connection.
			NetworkSocket *net = (NetworkSocket*)arg;
			int dataLeft = -1;
			int dataTotal = 0;
			int start = 0;
			int end = 0;
			Uint8 *pCircularBuffer = new Uint8[NETWORK_BUFFER];
#ifdef NET_DEBUG_CONNECTION
			bool activity = false;
#endif
			while(true)
			{
				SDL_LockMutex(mutNetworkShutdown);
				if(terminateNetwork)
				{
					SDL_UnlockMutex(mutNetworkShutdown);
					break;
				}
				SDL_UnlockMutex(mutNetworkShutdown);

				SDL_LockMutex(mutClientConnect);
				if(connect == true)
				{
					SDL_UnlockMutex(mutClientConnect);
					net->socket = SDLNet_TCP_Open(&clientConnect);
					SDL_LockMutex(mutClientConnect);
					if(net->socket != NULL)
					{
						//Connection Success
						connect = false;
						clientConnected = true;
						SDLNet_TCP_AddSocket(net->set, net->socket);
#ifdef NET_DEBUG
						cout << "Client successfully connected..." << endl;
#endif
						SDLNet_TCP_Send(net->socket, "PLAY", 4);
					}
					else
					{
#ifdef NET_DEBUG
						cout << "Client failed to connect" << endl;
#endif
					}
				}
				SDL_UnlockMutex(mutClientConnect);
				if(net->socket)
				{
					int nb = SDLNet_CheckSockets(net->set, 0);
					if(nb > 0)
					{
#ifdef NET_DEBUG_CONNECTION
						activity = true;
#endif
						if(SDLNet_SocketReady(net->socket))
						{
							int left = NETWORK_RECEIVE_CHUNK;
							if(GetDataInBuffer(start, end) + NETWORK_RECEIVE_CHUNK > NETWORK_BUFFER)
								left = NETWORK_BUFFER - GetDataInBuffer(start, end);

							if(left > 0)
							{
								int packetRead = SDLNet_TCP_Recv(net->socket, net->pBufferIn, left);
								if(packetRead <= 0)
									//FATAL ERROR, reconnect is a good thing todo.
									continue;

								AppendDataToBuffer(net->pBufferIn, packetRead, pCircularBuffer, start, end);
							}
						}

						bool loop = true;
						while(loop)
						{
							if(dataLeft == -1)
							{
								if(GetDataInBuffer(start, end) >= 2)
								{
									Uint8 buff[2];
									ExtractDataFromBuffer((Uint8*)&buff, 2, pCircularBuffer, start, end);
									dataLeft = dataTotal = SDLNet_Read16(&buff);
									if(dataLeft == 0)
									{
										dataLeft = -1;
									}
								}
								else
								{
									loop = false;
								}
							}

							if(dataLeft > 0)
							{
								//A packet exists in buffer..
								if(dataLeft - GetDataInBuffer(start, end) <= 0)
								{
									ExtractDataFromBuffer(net->pBufferIn, dataLeft, pCircularBuffer, start, end);
									dataLeft = -1;
									Packet *pPacket = ProcessPacket(net->pBufferIn, dataTotal);
									if(pPacket != NULL)
									{
										pPacket->node = 0;
#ifdef NET_DEBUG
										if(pPacket->frameLength >= 4)
										{
											cout << "CLIENT RECV Packet: " << pPacket->id[0] << pPacket->id[1] << pPacket->id[2] << pPacket->id[3] << " FrameData:" << SDLNet_Read32(pPacket->frame) << endl;
										}
										else
										{
											cout << "CLIENT RECV Packet: " << pPacket->id[0] << pPacket->id[1] << pPacket->id[2] << pPacket->id[3] << " Total packet len: " << dataTotal << endl;
										}
#endif	
										SDL_LockMutex(mutPacketInQueue);
										packetInQueue.push(pPacket);
										SDL_UnlockMutex(mutPacketInQueue);
										loop = true;
									}
									else
									{
#ifdef NET_DEBUG_CONNECTION
										cout << "CLIENT RECV Packet: failed checksum" << endl;
#endif
									}
								}
								else
								{
									loop = false;
								}
							}
						}
					}
					else
					{
#ifdef NET_DEBUG_CONNECTION
#ifdef NET_DEBUG
						if(activity != false)
						{
							cout << "No network client activity inbound..." << endl;
						}
#endif
						activity = false;
#endif
						SDL_Delay(1);
					}
				}
			}
			return SUCCESS;
		}

		int _networkSendThread(void* arg)
		{
			NetworkSocket *net = (NetworkSocket*)arg;
			int count = 0;
			while(true)
			{
				SDL_LockMutex(mutNetworkShutdown);
				if(terminateNetwork)
				{
					SDL_UnlockMutex(mutNetworkShutdown);
					break;
				}

				SDL_UnlockMutex(mutNetworkShutdown);
				SDL_LockMutex(mutClientConnect);
				if(clientConnected == false)
				{
					SDL_UnlockMutex(mutClientConnect);
#ifdef NET_DEBUG
					cout << "Client awaiting connection..." << endl;
#endif
					SDL_LockMutex(mutNetworkShutdown);
					if(terminateNetwork)
					{
						SDL_UnlockMutex(mutNetworkShutdown);
						break;
					}
					SDL_UnlockMutex(mutNetworkShutdown);
					SDL_Delay(1);
					continue;
				}
				else
					SDL_UnlockMutex(mutClientConnect);

				SDL_LockMutex(mutPacketFrameOutQueue);
				while (packetFrameOutQueue.size())
				{
					Packet *packet = packetFrameOutQueue.front();
					bool scheduleDeletion = false;
					if (!packet)
						*(int*) 0 = 0;
					packetFrameOutQueue.pop();
					packet->references--;
					if (!packet->references)
						scheduleDeletion = true;
					SDL_UnlockMutex(mutPacketFrameOutQueue);
					
					int packetLen = CreatePacket(net->pBufferOut, packet);
//					bytes_sent += packetLen;
					
					SendPacket(net->socket, packet, net->pBufferOut, packetLen);

					if (scheduleDeletion)
						DeletePacket(packet);

					SDL_LockMutex(mutPacketFrameOutQueue);
				}
				SDL_UnlockMutex(mutPacketFrameOutQueue);

				count = 0;
				SDL_LockMutex(mutPacketOutQueue);
				while (packetOutQueue.size())
				{
					Packet *packet = packetOutQueue.front();
					bool scheduleDeletion = false;
					packetOutQueue.pop();
					packet->references--;
					if (!packet->references)
						scheduleDeletion = true;
					SDL_UnlockMutex(mutPacketOutQueue);

					int packetLen = CreatePacket(net->pBufferOut, packet);

//					bytes_sent += packetLen;

					SendPacket(net->socket, packet, net->pBufferOut, packetLen);

					if (scheduleDeletion)
						DeletePacket(packet);

					count++;

					SDL_LockMutex(mutPacketOutQueue);

				}
				SDL_UnlockMutex(mutPacketOutQueue);
				SDL_Delay(1);
			}
			return SUCCESS;
		}
		
#define APPEND32BIT(dest, src) \
	SDLNet_Write32(src, dest); \
	dest += 4;

#define APPEND16BIT(dest, src) \
	SDLNet_Write16(src, dest); \
	dest += 2;

#define APPEND8BIT(dest, src) \
	*((Uint8*) dest) = src; \
	dest++;

		Uint32 READ32BIT(Uint8*& src)
		{
			Uint32 ret = SDLNet_Read32(src);
			src += 4;
			return ret;
		}

		Uint16 READ16BIT(Uint8*& src)
		{
			Uint16 ret = SDLNet_Read16(src);
			src += 2;
			return ret;
		}

		Uint8 READ8BIT(Uint8*& src)
		{
			Uint8 ret = *src;
			src++;
			return ret;
		}

		const unsigned ACTION_CHUNK_SIZE = 15;

		Chunk *CreateActionChunk(NetActionData *actiondata)
		{
			BUFFER *data = new BUFFER[ACTION_CHUNK_SIZE];
			Chunk *chunk = new Chunk;
			chunk->id[0] = 'A'; // bleh
			chunk->id[1] = 'C';
			chunk->id[2] = 'T';
			chunk->id[3] = 'N';
			chunk->length = ACTION_CHUNK_SIZE;
			chunk->data = data;

			APPEND32BIT(data, actiondata->valid_at_frame)
			APPEND16BIT(data, actiondata->unit_id)
			APPEND16BIT(data, actiondata->x)
			APPEND16BIT(data, actiondata->y)
			APPEND16BIT(data, actiondata->goalunit_id)
			APPEND16BIT(data, actiondata->arg)
			APPEND8BIT(data, actiondata->action)
			APPEND8BIT(data, actiondata->rot)

			return chunk;
		}

		int InterpretActionChunk(Chunk* chunk)
		{
			Uint8* data = chunk->data;
			NetActionData* actiondata = new NetActionData;

			if (chunk->length != ACTION_CHUNK_SIZE)
			{
				delete actiondata;
				return ERROR_GENERAL;
			}

			actiondata->valid_at_frame = READ32BIT(data);
			actiondata->unit_id = READ16BIT(data);
			actiondata->x = READ16BIT(data);
			actiondata->y = READ16BIT(data);
			actiondata->goalunit_id = READ16BIT(data);
			actiondata->arg = READ16BIT(data);
			actiondata->action = (AI::UnitAction) READ8BIT(data);
			actiondata->rot = READ8BIT(data);

			const gc_ptr<Dimension::Unit>& unit = Dimension::GetUnitByID(actiondata->unit_id);

			if (!unit)
			{
				delete actiondata;
				return ERROR_GENERAL;
			}

			waitingActions.push_back(actiondata);
			if (networkType == SERVER)
			{
				NetActionData* actiondata_copy = new NetActionData;
				*actiondata_copy = *actiondata;
				unsentActions.push_back(actiondata_copy);
			}
			return SUCCESS;
		}

		Chunk *CreatePathChunk(NetPath* path)
		{
			BUFFER *data = new BUFFER[1536];
			Chunk *chunk = new Chunk;
			int len;
			memset(data, 0, 1536);
			chunk->id[0] = 'P';
			chunk->id[1] = 'A';
			chunk->id[2] = 'T';
			chunk->id[3] = 'H';
			chunk->data = data;
			APPEND32BIT(data, path->valid_at_frame)
			APPEND16BIT(data, path->unit_id)
			len = EncodePath(path->pGoal, data, 1536-6);
			DeallocPath(path->pGoal);
			if (!len)
			{
				delete[] data;
				delete chunk;
				return NULL;
			}
			chunk->length = 6 + len;
			return chunk;
		}

		int InterpretPathChunk(Chunk *chunk)
		{
			Uint8 *data = chunk->data;
			NetPath *path = new NetPath;

			if (chunk->length < 6)
			{
				delete path;
				return ERROR_GENERAL;
			}

			path->valid_at_frame = READ32BIT(data);
			path->unit_id = READ16BIT(data);

			if (DecodePath(path->pStart, path->pGoal, data, chunk->length-6) != SUCCESS)
			{
				delete path;
				return ERROR_GENERAL;
			}
			waitingPaths.push_back(path);
			if (networkType == SERVER)
			{
				NetPath* path_copy = new NetPath;
				*path_copy = *path;
				ClonePath(path_copy->pStart, path_copy->pGoal);
				unsentPaths.push_back(path_copy);
			}
			return SUCCESS;
		}

		const unsigned CREATE_CHUNK_SIZE = 14;

		Chunk *CreateCreationChunk(NetCreate *create)
		{
			BUFFER *data = new BUFFER[CREATE_CHUNK_SIZE];
			Chunk *chunk = new Chunk;
			chunk->id[0] = 'C'; // bleh
			chunk->id[1] = 'R';
			chunk->id[2] = 'T';
			chunk->id[3] = 'E';
			chunk->length = CREATE_CHUNK_SIZE;
			chunk->data = data;

			APPEND32BIT(data, create->valid_at_frame)
			APPEND16BIT(data, create->unittype_id)
			APPEND16BIT(data, create->owner_id)
			APPEND16BIT(data, create->x)
			APPEND16BIT(data, create->y)
			APPEND8BIT(data, create->rot)

			return chunk;
		}

		int InterpretCreationChunk(Chunk* chunk)
		{
			Uint8* data = chunk->data;
			NetCreate* create = new NetCreate;

			if (chunk->length != CREATE_CHUNK_SIZE)
			{
				delete create;
				return ERROR_GENERAL;
			}

			create->valid_at_frame = READ32BIT(data);
			create->unittype_id = READ16BIT(data);
			create->owner_id = READ16BIT(data);
			create->x = READ16BIT(data);
			create->y = READ16BIT(data);
			create->rot = READ8BIT(data);
			
			waitingCreations.push_back(create);
			if (networkType == SERVER)
			{
				NetCreate* create_copy = new NetCreate;
				*create_copy = *create;
				unsentCreations.push_back(create_copy);
			}
			return SUCCESS;
		}

		const unsigned DAMAGE_CHUNK_SIZE = 10;

		Chunk *CreateDamagingChunk(NetDamage *damage)
		{
			BUFFER *data = new BUFFER[DAMAGE_CHUNK_SIZE];
			Chunk *chunk = new Chunk;
			chunk->id[0] = 'D'; // bleh
			chunk->id[1] = 'M';
			chunk->id[2] = 'G';
			chunk->id[3] = 'E';
			chunk->length = DAMAGE_CHUNK_SIZE;
			chunk->data = data;

			APPEND32BIT(data, damage->valid_at_frame)
			APPEND32BIT(data, damage->damage)
			APPEND16BIT(data, damage->unit_id)

			return chunk;
		}

		int InterpretDamagingChunk(Chunk* chunk)
		{
			Uint8* data = chunk->data;
			NetDamage* damage = new NetDamage;

			if (chunk->length != DAMAGE_CHUNK_SIZE)
			{
				delete damage;
				return ERROR_GENERAL;
			}

			damage->valid_at_frame = READ32BIT(data);
			damage->damage = READ32BIT(data);
			damage->unit_id = READ16BIT(data);
			
			waitingDamagings.push_back(damage);
			if (networkType == SERVER)
			{
				NetDamage* damage_copy = new NetDamage;
				*damage_copy = *damage;
				unsentDamagings.push_back(damage_copy);
			}
			return SUCCESS;
		}

		const unsigned SELL_CHUNK_SIZE = 10;

		Chunk *CreateSellChunk(NetSell *sell)
		{
			BUFFER *data = new BUFFER[SELL_CHUNK_SIZE];
			Chunk *chunk = new Chunk;
			chunk->id[0] = 'S'; // bleh
			chunk->id[1] = 'E';
			chunk->id[2] = 'L';
			chunk->id[3] = 'L';
			chunk->length = SELL_CHUNK_SIZE;
			chunk->data = data;

			APPEND32BIT(data, sell->valid_at_frame)
			APPEND32BIT(data, sell->amount)
			APPEND16BIT(data, sell->owner_id)

			return chunk;
		}

		int InterpretSellChunk(Chunk* chunk)
		{
			Uint8* data = chunk->data;
			NetSell* sell = new NetSell;

			if (chunk->length != SELL_CHUNK_SIZE)
			{
				delete sell;
				return ERROR_GENERAL;
			}

			sell->valid_at_frame = READ32BIT(data);
			sell->amount = READ32BIT(data);
			sell->owner_id = READ16BIT(data);
			
			waitingSells.push_back(sell);
			if (networkType == SERVER)
			{
				NetSell* sell_copy = new NetSell;
				*sell_copy = *sell;
				unsentSells.push_back(sell_copy);
			}
			return SUCCESS;
		}

		vector<Checksum*> receivedChecksums;

		void CalculateChecksum()
		{
			Uint32 checksum = 0;
			Checksum *checksum_struct = new Checksum;
			std::stringstream sstr;
			int index = 0;

			for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
			{
				const gc_ptr<Dimension::Unit>& unit = *it;
				checksum ^= index | unit->GetHandle() << (index % 15); // To get a number that is different for different index/id combos.
#ifdef CHECKSUM_DEBUG
				sstr << index << " ";
#endif
				checksum ^= unit->GetHandle();
#ifdef CHECKSUM_DEBUG
				sstr << unit->GetHandle() << " ";
#endif
				checksum ^= (Uint32) floor((float)unit->curAssociatedSquare.x);
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor((float)unit->curAssociatedSquare.x) << " ";
#endif
				checksum ^= ((Uint32) floor((float)unit->curAssociatedSquare.y))<<8;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor((float)unit->curAssociatedSquare.y) << " ";
#endif
				checksum ^= ((Uint32) floor(unit->health))<<16;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor(unit->health) << " ";
#endif
				checksum ^= ((Uint32) floor(unit->power))<<24;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor(unit->power) << " ";
#endif
				checksum ^= unit->isCompleted;
#ifdef CHECKSUM_DEBUG
				sstr << unit->isCompleted << " ";
#endif
				checksum ^= unit->isDisplayed<<1;
#ifdef CHECKSUM_DEBUG
				sstr << unit->isDisplayed << " ";
#endif
				checksum ^= unit->isMoving<<2;
#ifdef CHECKSUM_DEBUG
				sstr << unit->isMoving << " ";
#endif
				checksum ^= unit->isLighted<<3;
#ifdef CHECKSUM_DEBUG
				sstr << unit->isLighted << " ";
				sstr << endl;
#endif
				index++;

			}

			int bits = 0;
			for (vector<gc_ptr<Dimension::Player> >::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
			{
				const gc_ptr<Dimension::Player>& player = *it;
				checksum ^= ((Uint32) floor(player->resources.power))<<bits;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor(player->resources.power) << " ";
#endif
				bits += 11;
				if (bits >= 32)
					bits -= 32;

				checksum ^= ((Uint32) floor((player->resources.power - player->oldResources.power) * 100))<<bits;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor((player->resources.power - player->oldResources.power) * 100) << " ";
#endif
				bits += 12;
				if (bits >= 32)
					bits -= 32;

				checksum ^= ((Uint32) floor(player->resources.money))<<bits;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor(player->resources.money) << " ";
#endif
				bits += 9;
				if (bits >= 32)
					bits -= 32;
				
				checksum ^= ((Uint32) floor((player->resources.money - player->oldResources.money) * 100))<<bits;
#ifdef CHECKSUM_DEBUG
				sstr << (Uint32) floor((player->resources.money - player->oldResources.money) * 100) << " ";
#endif
				bits += 10;
				if (bits >= 32)
					bits -= 32;

#ifdef CHECKSUM_DEBUG
				sstr << endl;
#endif
			}
			
#ifdef CHECKSUM_DEBUG
			checksum_struct->data = sstr.str();
#endif
			checksum_struct->checksum = checksum;
			checksum_struct->frame = AI::currentFrame;
#ifdef CHECKSUM_DEBUG
			checksum_output << "Data for frame " << AI::currentFrame << "\n" << checksum_struct->data;
			checksum_output << "Checksum for frame " << AI::currentFrame << ": " << (void*) checksum_struct->checksum << "\n";
#endif

			unsentChecksums.push_back(checksum_struct);

			Checksum *checksum_struct2 = new Checksum;
			*checksum_struct2 = *checksum_struct;
			waitingChecksums.push_back(checksum_struct2);

		}

		const unsigned CHECKSUM_CHUNK_SIZE = 8;

		Chunk *CreateChecksumChunk(Checksum* checksum_struct)
		{
			BUFFER *data = new BUFFER[CHECKSUM_CHUNK_SIZE];
			Chunk *chunk = new Chunk;
			chunk->id[0] = 'C';
			chunk->id[1] = 'H';
			chunk->id[2] = 'K';
			chunk->id[3] = 'S';
			chunk->length = CHECKSUM_CHUNK_SIZE;
			chunk->data = data;

#ifdef CHECKSUM_DEBUG_HIGH
			checksum_output << "CHECKSUM SEND: " << checksum_struct->frame << " " << (void*) checksum_struct->checksum << "\n";
#endif

			APPEND32BIT(data, checksum_struct->frame)
			APPEND32BIT(data, checksum_struct->checksum)

			return chunk;
		}

		int InterpretChecksumChunk(Chunk* chunk)
		{
			Uint32 frame, checksum;
			BUFFER* data = chunk->data;
			if (chunk->length != CHECKSUM_CHUNK_SIZE)
			{
				return ERROR_GENERAL;
			}
			frame = READ32BIT(data);
			checksum = READ32BIT(data);

#ifdef CHECKSUM_DEBUG_HIGH
			checksum_output << "CHECKSUM RECV: " << frame << " " << (void*) checksum << "\n";
#endif

			Checksum *checksum_struct = new Checksum;
			checksum_struct->checksum = checksum;
			checksum_struct->frame = frame;
			receivedChecksums.push_back(checksum_struct);

			for (unsigned i = 0; i < receivedChecksums.size(); i++)
			{
				for (unsigned j = 0; j < waitingChecksums.size(); j++)
				{
					if (waitingChecksums.at(j)->frame == receivedChecksums.at(i)->frame)
					{
						if (waitingChecksums.at(j)->checksum != receivedChecksums.at(i)->checksum)
						{
#ifdef CHECKSUM_DEBUG
							checksum_output << "Checksum failed on frame " << waitingChecksums.at(j)->frame << "!" << "\n";
							checksum_output << (void*) waitingChecksums.at(j)->checksum << " != " << (void*) receivedChecksums.at(i)->checksum << "\n";
#endif
							if (!scheduleShutdown)
							{
								scheduleShutdown = true;
								shutdownFrame = AI::currentFrame + netDelay;
							}

							cout << "Checksum failed on frame " << waitingChecksums.at(j)->frame << "!" << endl;
						}
						delete waitingChecksums.at(j);
						delete receivedChecksums.at(i);
						waitingChecksums.erase(waitingChecksums.begin() + j--);
						receivedChecksums.erase(receivedChecksums.begin() + i--);
						break;
					}
				}
			}
			return SUCCESS;
		}

		void AppendDataToPacket(BUFFER*& pDest, void* pSrc, int len)
		{
			memcpy(pDest, pSrc, len);
			pDest += len;
		}

		Packet* ProcessPacket(BUFFER* rawdata, int rawlen)
		{
			//Packet: [checksum(4byte)][id(4byte)][numChunk(2byte)][frame length(2byte)][frame data][chunk data]

			if(rawlen < 12) //Minimum packet size.
				return NULL;

			BUFFER* pRawdata = rawdata;
			BUFFER* pMaxRawData = rawdata + rawlen;
			if(CRC32(rawdata + 4, rawlen - 4) == SDLNet_Read32(rawdata))
			{
				pRawdata += 4;
				//Checksum OK
				Packet* pPacket = new Packet();
				memcpy(&pPacket->id, pRawdata, 4);
				pRawdata += 4;

				pPacket->numChunks = READ16BIT(pRawdata);
				pPacket->frameLength = READ16BIT(pRawdata);

				if(pPacket->frameLength)
				{
					if(pRawdata + pPacket->frameLength > pMaxRawData)
					{
						//Invalid sizes
						delete pPacket;
						return NULL;
					}
					else
					{
						pPacket->frame = new BUFFER[pPacket->frameLength];
						memcpy(pPacket->frame, pRawdata, pPacket->frameLength);
						pRawdata += pPacket->frameLength;
					}
				}
				
				if(pPacket->numChunks)
				{
					//Chunk: [id(4byte)][length(2byte)][data]
					pPacket->chunks = new Chunk*[pPacket->numChunks];
					for(int i = 0; i < pPacket->numChunks; i++)
					{
						pPacket->chunks[i] = NULL;
					}

					for(int i = 0; i < pPacket->numChunks; i++)
					{
						if(pRawdata + 6 > pMaxRawData)
						{
							//ERROR! - Delete and return NULL
							DeletePacket(pPacket);
							return NULL;
						}

						pPacket->chunks[i] = new Chunk();
						Chunk *pCurrent = pPacket->chunks[i];
						pCurrent->data = NULL;
					
						memcpy(&pCurrent->id, pRawdata, 4);
						pRawdata += 4;

						pCurrent->length = READ16BIT(pRawdata);

						if(pRawdata + pCurrent->length > pMaxRawData)
						{
							DeletePacket(pPacket);
							return NULL;
						}
						
						pCurrent->data = new BUFFER[pCurrent->length];
						memcpy(pCurrent->data, pRawdata, pCurrent->length);
						pRawdata +=  pCurrent->length;
					}
				}
				else
				{
					pPacket->chunks = NULL;
				}
				return pPacket;
			}
			return NULL;
		}

		int CreatePacket(Uint8* packet, Packet* pPacket)
		{
			//Check for size

			//Packet: [checksum(4byte)][id(4byte)][numChunk(2byte)][frame length(2byte)][frame data][chunk data]
			//Chunk: [id(4byte)][length(2byte)][data]

			Uint8* packetData = packet;

			packetData += 6;
			AppendDataToPacket(packetData, &pPacket->id, 4); //ID
			APPEND16BIT(packetData, pPacket->numChunks)
			APPEND16BIT(packetData, pPacket->frameLength)
			AppendDataToPacket(packetData, pPacket->frame, (int)pPacket->frameLength);

			for(int i = 0; i < pPacket->numChunks; i++)
			{
				Chunk* pChunk = pPacket->chunks[i];
				AppendDataToPacket(packetData, &pChunk->id, 4); //ID
				APPEND16BIT(packetData, pChunk->length)
				AppendDataToPacket(packetData, pChunk->data, (int)pChunk->length); //ID
			}

			SDLNet_Write32(CRC32(packet + 6, (int)(packetData - packet) - 6), packet + 2);
			SDLNet_Write16((Uint16)(packetData - packet) - 2, packet);
			return (int)(packetData - packet);
		}

		void DeletePacket(Packet* data)
		{
			if(data->chunks)
			{	
				for(int i = 0; i < data->numChunks; i++)
				{
					if(data->chunks[i])
					{
						if(data->chunks[i]->data)
							delete [] data->chunks[i]->data;

						delete data->chunks[i];
					}
				}
				delete [] data->chunks;
			}
			if(data->frame)
				delete [] data->frame;

			delete data;
		}

		Packet* ClonePacket(Packet* packet)
		{
			Packet* new_packet = new Packet;
			new_packet->references = 0;
			new_packet->id[0] = packet->id[0];
			new_packet->id[1] = packet->id[1];
			new_packet->id[2] = packet->id[2];
			new_packet->id[3] = packet->id[3];
			new_packet->node = packet->node;
			new_packet->frameLength = packet->frameLength;
			new_packet->numChunks = packet->numChunks;
			if(packet->chunks && packet->numChunks)
			{
				new_packet->chunks = new Chunk*[packet->numChunks];
				for(int i = 0; i < packet->numChunks; i++)
				{
					Chunk *new_chunk = new Chunk;
					new_chunk->id[0] = packet->chunks[i]->id[0];
					new_chunk->id[1] = packet->chunks[i]->id[1];
					new_chunk->id[2] = packet->chunks[i]->id[2];
					new_chunk->id[3] = packet->chunks[i]->id[3];
					new_chunk->length = packet->chunks[i]->length;
					new_chunk->data = new BUFFER[new_chunk->length];
					memcpy(new_chunk->data, packet->chunks[i]->data, new_chunk->length);
					new_packet->chunks[i] = new_chunk;
				}
			}
			else
			{
				new_packet->chunks = NULL;
			}
			if(packet->frame && packet->frameLength)
			{
				new_packet->frame = new BUFFER[new_packet->frameLength];
				memcpy(new_packet->frame, packet->frame, packet->frameLength);
			}
			else
			{
				packet->frame = NULL;
			}

			return new_packet;
		}

		Packet *PopReceivedPacket()
		{
			Packet *ret = NULL;

			SDL_LockMutex(mutPacketInQueue);

			if (packetInQueue.size())
			{
				ret = packetInQueue.front();
				packetInQueue.pop();
			}

			SDL_UnlockMutex(mutPacketInQueue);

			return ret;
		}
		
		void PushPacketToSend(Packet *packet)
		{
			if (!packet)
				*(int*) 0 = 0;
			if (packet->id[0] == 'F' && packet->id[1] == 'R' && packet->id[2] == 'A' && packet->id[3] == 'M')
			{
				SDL_LockMutex(mutPacketFrameOutQueue);

				packet->references++;

				packetFrameOutQueue.push(packet);

				SDL_UnlockMutex(mutPacketFrameOutQueue);
			}
			else
			{
				SDL_LockMutex(mutPacketOutQueue);

				packet->references++;

				packetOutQueue.push(packet);

				SDL_UnlockMutex(mutPacketOutQueue);
			}
		}

		unsigned int QueueSize()
		{
			SDL_LockMutex(mutPacketFrameOutQueue);
			
			SDL_LockMutex(mutPacketOutQueue);
			
			unsigned int len = packetFrameOutQueue.size() + packetOutQueue.size();

			SDL_UnlockMutex(mutPacketFrameOutQueue);

			SDL_UnlockMutex(mutPacketOutQueue);

			return len;
		}
	}
}

