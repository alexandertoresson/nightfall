/*
 *  aibase.cpp
 *  Projektarbete
 *
 *  Created by Leonard Wickmark on 2006-11-10.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "aibase.h"

#include "game.h"
#include "dimension.h"
#include "ainode.h"
#include "unit.h"
#include "aipathfinding.h"
#include "luawrapper.h"
#include "networking.h"
#include "environment.h"
#include "unitinterface.h"

namespace Game
{
	namespace AI
	{
		unsigned aiFps = 30;
		unsigned resetFrame = 0;   // for determining when to reset Dimension::PositionSearch_NumStepsTaken
		Uint32 currentFrame = 0; // for tracking the number of the current frame
		int aiFramesPerformedSinceLastRender = 0;
		int action_changes = 0;
		int pathnodes = 0;
		int paths = 0;

		using namespace Utilities::Scripting;

		bool NextFrame()
		{
			bool may_advance = false;
			if (Networking::isNetworked)
			{
				if (Networking::PerformIngameNetworking())
				{
					may_advance = true;
				}
			}
			else
			{
				may_advance = true;
			}
			if (may_advance)
			{
				currentFrame++;
				resetFrame++;
				if (resetFrame > aiFps)
				{
					static int ac_highest = 0, pn_highest = 0, p_highest = 0;
					Dimension::PositionSearch_NumStepsTaken = 0;
					resetFrame = 0;
					if (action_changes > ac_highest)
						ac_highest = action_changes;
					if (pathnodes > pn_highest)
						pn_highest = pathnodes;
					if (paths > p_highest)
						p_highest = paths;
	//				cout << action_changes << " " << pathnodes << " " << paths << " : " << ac_highest << " " << pn_highest << " " << p_highest << endl;
					action_changes = 0;
					pathnodes = 0;
					paths = 0;
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		void SendCommandUnitToLua(int unitID, int playerIndex, int x, int y, UnitAction action, void* argument)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(playerIndex);
			if (Dimension::pWorld->vPlayers[playerIndex]->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (Dimension::pWorld->vPlayers[playerIndex]->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("CommandUnit_TargetPos_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("CommandUnit_TargetPos_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("CommandUnit_TargetPos_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), (void*) unitID);
			lua_pushnumber(pVM->GetVM(), x);
			lua_pushnumber(pVM->GetVM(), y);
			lua_pushnumber(pVM->GetVM(), action);
			lua_pushlightuserdata(pVM->GetVM(), argument);
			pVM->CallFunction(5);
		}

		void SendCommandUnitToLua(int unitID, int playerIndex, int destinationID, UnitAction action, void* argument)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(playerIndex);
			if (Dimension::pWorld->vPlayers[playerIndex]->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (Dimension::pWorld->vPlayers[playerIndex]->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("CommandUnit_TargetUnit_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("CommandUnit_TargetUnit_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("CommandUnit_TargetUnit_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), (void*) unitID);
			lua_pushlightuserdata(pVM->GetVM(), (void*) destinationID);
			lua_pushnumber(pVM->GetVM(), action);
			lua_pushlightuserdata(pVM->GetVM(), argument);
			pVM->CallFunction(4);
		}

		struct ScheduledCommand
		{
			bool hasUnitTarget;
			int playerIndex;
			int unitID;
			int destinationID;
			int x, y;
			UnitAction action;
			void* argument;
		};

		std::vector<ScheduledCommand*> scheduledCommands;

		void ScheduleCommandUnit(Dimension::Unit* pUnit, int x, int y, UnitAction action, void* argument)
		{
			pUnit->lastCommand = SDL_GetTicks();
			ScheduledCommand *command = new ScheduledCommand();
			command->hasUnitTarget = false;
			command->unitID = pUnit->id;
			command->playerIndex = pUnit->owner->index;
			command->x = x;
			command->y = y;
			command->action = action;
			command->argument = argument;
			scheduledCommands.push_back(command);
		}

		void ScheduleCommandUnit(Dimension::Unit* pUnit, Dimension::Unit* destination, UnitAction action, void* argument)
		{
			pUnit->lastCommand = SDL_GetTicks();
			ScheduledCommand *command = new ScheduledCommand();
			command->hasUnitTarget = true;
			command->unitID = pUnit->id;
			command->playerIndex = pUnit->owner->index;
			command->destinationID = destination->id;
			command->action = action;
			command->argument = argument;
			scheduledCommands.push_back(command);
		}

		void ApplyScheduledCommandUnits()
		{
			for (std::vector<ScheduledCommand*>::iterator it = scheduledCommands.begin(); it != scheduledCommands.end(); it++)
			{
				ScheduledCommand *command = *it;
				if (command->hasUnitTarget)
				{
					SendCommandUnitToLua(command->unitID, command->playerIndex, command->destinationID, command->action, command->argument);
				}
				else
				{
					SendCommandUnitToLua(command->unitID, command->playerIndex, command->x, command->y, command->action, command->argument);
				}
				delete command;
			}
			scheduledCommands.clear();
		}

		enum UnitEventType
		{
			UNITEVENTTYPE_ACTION,
			UNITEVENTTYPE_SIMPLE,
			UNITEVENTTYPE_ATTACK
		};

		struct UnitEvent
		{
			UnitEventType eventType;
			int unitID;
			int playerIndex;
			UnitAction action;
			int x, y;
			int targetID;
			void *arg;
			std::string *func;
		};

		vector<UnitEvent*> scheduledUnitEvents;

		void SendScheduledUnitEvents()
		{
			for (vector<UnitEvent*>::iterator it = scheduledUnitEvents.begin(); it != scheduledUnitEvents.end(); it++)
			{
				UnitEvent* event = *it;
				Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(event->playerIndex);
				pVM->SetFunction(*event->func);
				switch (event->eventType)
				{
					case UNITEVENTTYPE_ACTION:
						lua_pushlightuserdata(pVM->GetVM(), (void*) event->unitID);
						lua_pushinteger(pVM->GetVM(), event->action);
						lua_pushnumber(pVM->GetVM(), event->x);
						lua_pushnumber(pVM->GetVM(), event->y);
						lua_pushlightuserdata(pVM->GetVM(), (void*) event->targetID);
						lua_pushlightuserdata(pVM->GetVM(), event->arg);
						pVM->CallFunction(6);
						break;
					case UNITEVENTTYPE_SIMPLE:
//						std::cout << *event->func << " " << event->unitID << std::endl;
						lua_pushlightuserdata(pVM->GetVM(), (void*) event->unitID);
						pVM->CallFunction(1);
						break;
					case UNITEVENTTYPE_ATTACK:
						lua_pushlightuserdata(pVM->GetVM(), (void*) event->unitID);
						lua_pushlightuserdata(pVM->GetVM(), (void*) event->targetID);
						pVM->CallFunction(2);
						break;
				}
				delete event;
			}
			scheduledUnitEvents.clear();
		}

		SDL_mutex *scheduleUnitEventMutex = SDL_CreateMutex();

		void ScheduleActionUnitEvent(Dimension::Unit* pUnit, EventAIFunc *aiEvent)
		{

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			UnitEvent *event = new UnitEvent;

			event->unitID = pUnit->id;
			event->playerIndex = pUnit->owner->index;
			event->action = pUnit->pMovementData->action.action;
			event->x = pUnit->pMovementData->action.goal.pos.x;
			event->y = pUnit->pMovementData->action.goal.pos.y;
			if (pUnit->pMovementData->action.goal.unit)
			{
				event->targetID = pUnit->pMovementData->action.goal.unit->id;
			}
			else
			{
				event->targetID = 0;
			}
			event->arg = pUnit->pMovementData->action.arg;
			event->func = &aiEvent->func;
			event->eventType = UNITEVENTTYPE_ACTION;

			SDL_LockMutex(scheduleUnitEventMutex);
			scheduledUnitEvents.push_back(event);
			SDL_UnlockMutex(scheduleUnitEventMutex);

		}

		void ScheduleSimpleUnitEvent(Dimension::Unit* pUnit, EventAIFunc *aiEvent)
		{

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			UnitEvent *event = new UnitEvent;

			event->unitID = pUnit->id;
			event->playerIndex = pUnit->owner->index;
			event->func = &aiEvent->func;
			event->eventType = UNITEVENTTYPE_SIMPLE;

			SDL_LockMutex(scheduleUnitEventMutex);
			scheduledUnitEvents.push_back(event);
			SDL_UnlockMutex(scheduleUnitEventMutex);
		}

		void SendUnitEventToLua_CommandCompleted(Dimension::Unit* pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.commandCompleted);
		}

		void SendUnitEventToLua_CommandCancelled(Dimension::Unit* pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.commandCancelled);
		}

		void SendUnitEventToLua_NewCommand(Dimension::Unit* pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.newCommand);
		}

		void SendUnitEventToLua_BecomeIdle(Dimension::Unit* pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->unitAIFuncs.becomeIdle);
		}

		void SendUnitEventToLua_UnitCreation(Dimension::Unit* pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->type->playerAIFuncs[pUnit->owner->index].unitCreation);
		}

		void SendUnitEventToLua_UnitKilled(Dimension::Unit* pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->unitAIFuncs.unitKilled);
		}

		void SendUnitEventToLua_IsAttacked(Dimension::Unit* pUnit, Dimension::Unit* attacker)
		{
			
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			UnitEvent *event = new UnitEvent;

			event->eventType = UNITEVENTTYPE_ATTACK;
			event->unitID = pUnit->id;
			event->playerIndex = pUnit->owner->index;
			event->targetID = attacker->id;
			event->func = &pUnit->type->unitAIFuncs[pUnit->owner->index].isAttacked.func;

			SDL_LockMutex(scheduleUnitEventMutex);
			scheduledUnitEvents.push_back(event);
			SDL_UnlockMutex(scheduleUnitEventMutex);
		}

		void HandleUnitPower()
		{
			double power_usage;
			vector<Dimension::Unit*>::iterator it_end = Dimension::pWorld->vUnits.end();
			for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != it_end; it++)
			{
				Dimension::Unit* pUnit = *it;
				pUnit->hasPower = true;
				if (pUnit->isCompleted && pUnit->isDisplayed && pUnit->pMovementData->action.action != ACTION_DIE)
				{

					power_usage = (pUnit->type->powerUsage + pUnit->type->lightPowerUsage) / aiFps;
					
					if (pUnit->owner->resources.power < power_usage)
					{
						pUnit->hasPower = false;
						NotEnoughPowerForLight(pUnit);
						continue;
					}
						
					pUnit->hasPower = true;
					EnoughPowerForLight(pUnit);

					pUnit->owner->resources.power -= power_usage;
					if (pUnit->type->powerIncrement > 0.0)
					{
						if (pUnit->type->powerType == Game::Dimension::POWERTYPE_DAYLIGHT)
						{
							Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
							double curh = pDimension->GetCurrentHour();
							if (curh >= 6.0 && curh <= 18.0)
							{
								pUnit->owner->resources.power += (pUnit->type->powerIncrement / aiFps) * (pow(sin((curh-6.0)/12*PI), 1.0/3) * 0.8 + 0.2);

							}
						}
						else
						{
							pUnit->owner->resources.power += pUnit->type->powerIncrement / aiFps;
						}
					}
				}
				else
				{
					pUnit->hasPower = false;
				}
			}
			
		}

		void PerformSimpleAI(Dimension::Unit* pUnit)
		{
			UnitAction action;
			int should_move;
			
			HandleProjectiles(pUnit);

			if (pUnit->isCompleted && pUnit->hasPower && pUnit->isDisplayed && pUnit->pMovementData->action.action != ACTION_DIE)
			{
			
				action = pUnit->pMovementData->action.action;
				
				if (action == ACTION_GOTO || action == ACTION_FOLLOW || action == ACTION_ATTACK || action == ACTION_BUILD || action == ACTION_RESEARCH)
				{
		
					should_move = pUnit->type->isMobile;
					if (!should_move)
					{
						pUnit->isWaiting = false;
					}

					if (action == ACTION_ATTACK)
					{
						Dimension::Unit* targetUnit = pUnit->pMovementData->action.goal.unit;
						if (pUnit->owner == targetUnit->owner)
						{
							AI::CancelAction(pUnit);
							should_move = false;
						}
						if (Dimension::CanReach(pUnit, targetUnit))
						{
							if (Dimension::CanAttack(pUnit))
							{
								Dimension::InitiateAttack(pUnit, targetUnit);
							}
							if (should_move)
							{
								pUnit->isWaiting = true;
								should_move = false;
							}
						}
						else if (!pUnit->type->isMobile)
						{
							AI::CancelAction(pUnit);
						}
					}

					if (action == ACTION_BUILD)
					{
						if (IsWithinRangeForBuilding(pUnit))
						{
							Dimension::Build(pUnit);
							AI::DeallocPathfindingNodes(pUnit);
							if (should_move)
							{
								pUnit->isWaiting = true;
								should_move = false;
							}
						}
					}

					if (action == ACTION_RESEARCH)
					{
						Dimension::Research(pUnit);
					}

					if (should_move)
					{
						Dimension::MoveUnit(pUnit);
					}
					else
					{
						pUnit->isMoving = false;
					}
					
					if (pUnit->isMoving)
					{
						if (pUnit->soundNodes[Audio::SFX_ACT_MOVE_RPT] == NULL)
						{
							PlayRepeatingActionSound(pUnit, Audio::SFX_ACT_MOVE_RPT);
						}
					}
					else
					{
						if (pUnit->soundNodes[Audio::SFX_ACT_MOVE_RPT] != NULL)
						{
							StopRepeatingActionSound(pUnit, Audio::SFX_ACT_MOVE_RPT);
							PlayActionSound(pUnit, Audio::SFX_ACT_MOVE_DONE_FNF);
						}
					}

				}
				else
				{
					pUnit->isWaiting = false;
					pUnit->isMoving = false;
				}
			}
		}

		void PerformVerySimpleAI(Dimension::Unit* pUnit)
		{
			double power_usage;
			if (pUnit->isCompleted && pUnit->isDisplayed && pUnit->pMovementData->action.action != ACTION_DIE)
			{

				power_usage = (pUnit->type->powerUsage + pUnit->type->lightPowerUsage) / aiFps;
				
				if (pUnit->owner->resources.power < power_usage)
				{
					pUnit->hasPower = false;
					NotEnoughPowerForLight(pUnit);
					return;
				}

				float power_inc = pUnit->type->regenPower / aiFps;
				pUnit->power = pUnit->power +  power_inc > pUnit->type->maxPower ? pUnit->type->maxPower : pUnit->power + power_inc;
					
				float health_inc = pUnit->type->regenHealth / aiFps;
				pUnit->health = pUnit->health +  health_inc > pUnit->type->maxHealth ? pUnit->type->maxHealth : pUnit->health + health_inc;
					
				pUnit->hasPower = true;
				EnoughPowerForLight(pUnit);

				pUnit->owner->resources.power -= power_usage;
				if (pUnit->type->powerIncrement > 0.0)
				{
					if (pUnit->type->powerType == Game::Dimension::POWERTYPE_DAYLIGHT)
					{
						Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
						double curh = pDimension->GetCurrentHour();
						if (curh >= 6.0 && curh <= 18.0)
						{
							pUnit->owner->resources.power += (pUnit->type->powerIncrement / aiFps) * (pow(sin((curh-6.0)/12*PI), 1.0/3) * 0.8 + 0.2);

						}
					}
					else
					{
						pUnit->owner->resources.power += pUnit->type->powerIncrement / aiFps;
					}
				}
			
				if (pUnit->pMovementData->action.action == AI::ACTION_NONE || pUnit->pMovementData->action.action == AI::ACTION_NETWORK_AWAITING_SYNC)
				{
					pUnit->isMoving = false;
				}

			}
			else
			{
				pUnit->hasPower = false;
			}
			
		}

		void PerformLuaUnitAI(Dimension::Unit* pUnit)
		{
			if (pUnit->hasPower)
			{

				pUnit->aiFrame++;

				if (pUnit->aiFrame >= pUnit->unitAIFuncs.performUnitAI.delay && pUnit->owner->type != Dimension::PLAYER_TYPE_REMOTE)
				{
					Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);

					if (pUnit->unitAIFuncs.performUnitAI.enabled)
					{
						pVM->SetFunction(pUnit->unitAIFuncs.performUnitAI.func);

						lua_pushlightuserdata(pVM->GetVM(), (void*) pUnit->id);
						lua_pushinteger(pVM->GetVM(), pUnit->pMovementData->action.action);
						pVM->CallFunction(2);
					}
					pUnit->aiFrame = 0;
				}
			}
			
		}

		void PerformLuaPlayerAI(Dimension::Player* player)
		{
			player->aiFrame++;
			if (player->aiFrame >= player->playerAIFuncs.performPlayerAI.delay && player->type != Dimension::PLAYER_TYPE_REMOTE)
			{
				Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(player->index);

				if (player->playerAIFuncs.performPlayerAI.enabled)
				{
					pVM->SetFunction(player->playerAIFuncs.performPlayerAI.func);

					lua_pushlightuserdata(pVM->GetVM(), player);
					pVM->CallFunction(1);
				}
				player->aiFrame = 0;
			}
		}

		int numLuaAIThreads = 2;

		SDL_cond **fireAIConds;
		SDL_cond *simpleAIdoneCond;
		SDL_cond **luaAIdoneConds;

		SDL_mutex **mainAIWaitMutexes;
		SDL_mutex *simpleAIWaitMutex;
		SDL_mutex **luaAIWaitMutexes;

		SDL_Thread *simpleAIThread;
		SDL_Thread **luaAIThreads;

		volatile bool simpleAIThreadRunning = false;
		volatile bool *luaAIThreadsRunning;

		vector<Dimension::Player*> *playersHandledPerLuaThread;
		int* numUnitsPerLuaThread;
		volatile int aiThreadsDone;
		volatile bool aiIsFired;

		int _SimpleAIThread(void* arg)
		{

			SDL_LockMutex(simpleAIWaitMutex);

			simpleAIThreadRunning = true;

			while (1)
			{

				do
				{
					SDL_CondWait(fireAIConds[0], simpleAIWaitMutex);
				} while (!aiIsFired);
				
				for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
				{
					Dimension::Unit* pUnit = *it;
					PerformVerySimpleAI(pUnit);
					if (pUnit->pMovementData->action.action == ACTION_DIE && currentFrame - pUnit->lastAttacked > (unsigned) aiFps)
					{
						ScheduleUnitDeletion(pUnit);
					}
				}

				for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnitsWithAI.begin(); it != Dimension::pWorld->vUnitsWithAI.end(); it++)
				{
					PerformSimpleAI(*it);
				}

				SDL_LockMutex(mainAIWaitMutexes[0]); // Make sure that the main thread is waiting for the signal
					aiThreadsDone++; // Hijack mutex to secure updating of aiThreadsDone
				SDL_UnlockMutex(mainAIWaitMutexes[0]);

				SDL_CondBroadcast(simpleAIdoneCond);
			}

			return 1;
		}

		int _LuaAIThread(void* arg)
		{
			unsigned long i = (unsigned long)arg; // <~~ 64 bit compability
			SDL_LockMutex(luaAIWaitMutexes[i]);

			luaAIThreadsRunning[i] = true;

			while (1)
			{

				do
				{
					SDL_CondWait(fireAIConds[i+1], luaAIWaitMutexes[i]);
				} while (!aiIsFired);
				
				for (vector<Dimension::Player*>::iterator it = playersHandledPerLuaThread[i].begin(); it != playersHandledPerLuaThread[i].end(); it++)
				{
					Dimension::Player* player = *it;
					PerformLuaPlayerAI(player);

					for (vector<Dimension::Unit*>::iterator it2 = player->vUnitsWithLuaAI.begin(); it2 != player->vUnitsWithLuaAI.end(); it2++)
					{
						PerformLuaUnitAI(*it2);
					}
				}

				SDL_LockMutex(mainAIWaitMutexes[i+1]); // Make sure that the main thread is waiting for the signal
					aiThreadsDone++;
				SDL_UnlockMutex(mainAIWaitMutexes[i+1]);

				SDL_CondBroadcast(luaAIdoneConds[i]);
			}

			return 1;
		}

		void InitAIThreads()
		{
			if (numLuaAIThreads)
			{
				simpleAIdoneCond = SDL_CreateCond();
				simpleAIWaitMutex = SDL_CreateMutex();

				luaAIdoneConds = new SDL_cond*[numLuaAIThreads];
				luaAIWaitMutexes = new SDL_mutex*[numLuaAIThreads];
				luaAIThreadsRunning = new bool[numLuaAIThreads];
				playersHandledPerLuaThread = new vector<Dimension::Player*>[numLuaAIThreads];
				numUnitsPerLuaThread = new int[numLuaAIThreads];
				for (int i = 0; i < numLuaAIThreads; i++)
				{
					luaAIdoneConds[i] = SDL_CreateCond();
					luaAIWaitMutexes[i] = SDL_CreateMutex();
					luaAIThreadsRunning[i] = false;
					numUnitsPerLuaThread[i] = 0;
				}

				fireAIConds = new SDL_cond*[numLuaAIThreads+1];;
				mainAIWaitMutexes = new SDL_mutex*[numLuaAIThreads+1];
				for (int i = 0; i < numLuaAIThreads+1; i++)
				{
					fireAIConds[i] = SDL_CreateCond();
					mainAIWaitMutexes[i] = SDL_CreateMutex();
					SDL_LockMutex(mainAIWaitMutexes[i]);
				}

				aiThreadsDone = 0;
				aiIsFired = false;

				simpleAIThread = SDL_CreateThread(_SimpleAIThread, NULL);
					
				luaAIThreads = new SDL_Thread*[numLuaAIThreads];
				for (int i = 0; i < numLuaAIThreads; i++)
				{
					luaAIThreads[i] = SDL_CreateThread(_LuaAIThread, (void*) i);
				}

				while (!simpleAIThreadRunning) // Make sure that the simple ai thread has started
				{
					SDL_Delay(1);
				}

				for (int i = 0; i < numLuaAIThreads; i++)
				{
					while (!luaAIThreadsRunning[i]) // Make sure that the lua ais have started
					{
						SDL_Delay(1);
					}
				}

			}

		}

		void PerformAIFrame()
		{
			static bool may_run_ai = true;
			if (may_run_ai)
			{
				aiFramesPerformedSinceLastRender++;
				Dimension::Environment::FourthDimension::Instance()->RotateWorld(1.00f / (float) aiFps);

				///////////////////////////////////////////////////////////////////////////
				// Apply Paths

				ApplyAllNewPaths();

				for (vector<Dimension::Player*>::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
				{
					Dimension::Player* player = *it;
					player->oldResources = player->resources;
				}

				if (numLuaAIThreads)
				{

					///////////////////////////////////////////////////////////////////////////
					// Balance out players over lua threads
					
					
					for (int i = 0; i < numLuaAIThreads; i++)
					{
						playersHandledPerLuaThread[i].clear();
						numUnitsPerLuaThread[i] = 0;
					}

					for (unsigned i = 0; i < Dimension::pWorld->vPlayers.size(); i++) // Try to divide units well between lua threads
					{
						int lowestUnitNum = 0;
						for (int j = 1; j < numLuaAIThreads; j++)
						{
							if (numUnitsPerLuaThread[j] < numUnitsPerLuaThread[lowestUnitNum])
							{
								lowestUnitNum = j;
							}
						}
						playersHandledPerLuaThread[lowestUnitNum].push_back(Dimension::pWorld->vPlayers[i]);
						numUnitsPerLuaThread[lowestUnitNum] += 250 + Dimension::pWorld->vPlayers[i]->vUnits.size();
					}

					///////////////////////////////////////////////////////////////////////////
					// Wake up simpleai and lua threads
					
					aiThreadsDone = 0;
					aiIsFired = true;

					SDL_LockMutex(simpleAIWaitMutex); // Make sure that the SimpleAI thread is waiting for the signal
					SDL_UnlockMutex(simpleAIWaitMutex);

					SDL_CondBroadcast(fireAIConds[0]); // Wake up SimpleAi thread

					for (int i = 0; i < numLuaAIThreads; i++)
					{
						SDL_LockMutex(luaAIWaitMutexes[i]); // Make sure that the lua threads are waiting for the signal
						SDL_UnlockMutex(luaAIWaitMutexes[i]);
						SDL_CondBroadcast(fireAIConds[i+1]); // Wake up Lua threads
					}

					///////////////////////////////////////////////////////////////////////////
					// Wait for the threads to be done
					
					while (aiThreadsDone < 1)
					{
						SDL_CondWait(simpleAIdoneCond, mainAIWaitMutexes[0]); // Wait for SimpleAI thread
					}
					
					for (int i = 0; i < numLuaAIThreads; i++)
					{
						while (aiThreadsDone < i+2)
						{
							SDL_CondWait(luaAIdoneConds[i], mainAIWaitMutexes[i+1]); // Wait for lua ai threads
						}
					}
					
					aiIsFired = false;
					
				}
				else
				{
					///////////////////////////////////////////////////////////////////////////
					// No threads? Do lua ai and simple ai the non-threaded way.
					
					for (vector<Dimension::Player*>::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
					{
						Dimension::Player* player = *it;
						PerformLuaPlayerAI(player);
						for (vector<Dimension::Unit*>::iterator it2 = player->vUnitsWithLuaAI.begin(); it2 != player->vUnitsWithLuaAI.end(); it2++)
						{
							PerformLuaUnitAI(*it2);
						}
					}

					for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
					{
						Dimension::Unit* pUnit = *it;
						PerformVerySimpleAI(pUnit);
						if (pUnit->pMovementData->action.action == ACTION_DIE && currentFrame - pUnit->lastAttacked > (unsigned) aiFps)
						{
							ScheduleUnitDeletion(pUnit);
						}
					}

					for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnitsWithAI.begin(); it != Dimension::pWorld->vUnitsWithAI.end(); it++)
					{
						PerformSimpleAI(*it);
					}

				}

				///////////////////////////////////////////////////////////////////////////
				// Particle system!

				FX::pParticleSystems->Iterate(1.0f / (float) aiFps);
				
				///////////////////////////////////////////////////////////////////////////
				// Apply various stuff that cannot be applied while the lua and simpleai threads are running

				Dimension::DisplayScheduledUnits();
				Dimension::ApplyScheduledBigSquareUpdates();
				UnitLuaInterface::ApplyScheduledActions();
				UnitLuaInterface::ApplyScheduledDamagings();

				// Send unit commands that could not be sent while lua ai was running
				ApplyScheduledCommandUnits();

				// DisplayScheduledUnits() and ApplyScheduledActions() will have queued up more 
				// events, so we do this as the last thing before deleting units, to avoid that
				// events survive onto the next frame.
				SendScheduledUnitEvents();

				// Apply deletions last, so it may 'undo' actions that have been applied before,
				// otherwise if you apply actions after it, you may apply actions with targets
				// that are deleted units.
				Dimension::DeleteScheduledUnits(); 

			}
			else
			{
				SDL_Delay(1);
			}

			may_run_ai = NextFrame();

#ifndef USE_MULTITHREADED_CALCULATIONS
			PerformPathfinding();
#endif				
		}

		bool ImmobilityCheck(Dimension::Unit* pUnit, UnitAction action, int x, int y)
		{
			if (action == AI::ACTION_MOVE_ATTACK ||
				action == AI::ACTION_MOVE_ATTACK_UNIT ||
				action == AI::ACTION_GOTO)
			{
				if (!pUnit->type->isMobile)
				{
					if (pUnit->type->canBuild.size() > 0)
					{
						if (pUnit->rallypoint == NULL)
							pUnit->rallypoint = new Dimension::IntPosition;

						pUnit->rallypoint->x = x;
						pUnit->rallypoint->y = y;
					}
					return true;
				}
			}

			return false;
		}

		void CommandUnit(Dimension::Unit* pUnit, int x, int y, UnitAction action, void* argument, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, x, y))
				return;

			if (!pUnit->type->isMobile && action == ACTION_GOTO)
				return;

			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted && pUnit->pMovementData->action.action != ACTION_DIE)
			{
				Dimension::ActionData* actiondata = NULL;

				if (!queue)
				{
					while (pUnit->actionQueue.size())
					{
						actiondata = pUnit->actionQueue.front();

						if (actiondata->visual_repr)
						{
							if (actiondata->visual_repr->ghost)
								DeleteGhostUnit(actiondata->visual_repr->ghost);
						}

						delete actiondata;
						pUnit->actionQueue.pop_front();
					}
				}

				actiondata = new Dimension::ActionData;
				actiondata->action = action;
				actiondata->goal_pos.x = x;
				actiondata->goal_pos.y = y;
				actiondata->goal_unit = NULL;
				actiondata->arg = argument;
				actiondata->visual_repr = Dimension::PrepareActionDataForVisualRepr(pUnit, action, argument, x, y);

				if (insert)
				{
					pUnit->actionQueue.push_front(actiondata);
				}
				else
				{
					pUnit->actionQueue.push_back(actiondata);
				}

				if (pUnit->actionQueue.size() == 1 || insert)
				{
					ScheduleCommandUnit(pUnit, x, y, action, argument);
				}

			}
		}

		void CommandUnit(Dimension::Unit* pUnit, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, destination->curAssociatedSquare.x, destination->curAssociatedSquare.y))
				return;
			
			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted && pUnit->pMovementData->action.action != ACTION_DIE)
			{
				Dimension::ActionData* actiondata = NULL;
				if (!queue)
				{
					while (pUnit->actionQueue.size())
					{
						actiondata = pUnit->actionQueue.front();

						if (actiondata->visual_repr)
						{
							if (actiondata->visual_repr->ghost)
								DeleteGhostUnit(actiondata->visual_repr->ghost);
						}

						delete actiondata;
						pUnit->actionQueue.pop_front();
					}
				}

				actiondata = new Dimension::ActionData;
				actiondata->action = action;
				actiondata->goal_pos.x = 0;
				actiondata->goal_pos.y = 0;
				actiondata->goal_unit = destination;
				actiondata->arg = argument;
				actiondata->visual_repr = Dimension::PrepareActionDataForVisualRepr(pUnit, action, argument, 0, 0);

				if (insert)
				{
					pUnit->actionQueue.push_front(actiondata);
				}
				else
				{
					pUnit->actionQueue.push_back(actiondata);
				}
				
				if (pUnit->actionQueue.size() == 1 || insert)
				{
					ScheduleCommandUnit(pUnit, destination, action, argument);
				}

			}
		}

		void CommandUnits(vector<Dimension::Unit*> pUnits, int x, int y, UnitAction action, void* argument, bool queue, bool insert)
		{
			for (vector<Dimension::Unit*>::iterator it = pUnits.begin(); it != pUnits.end(); it++)
			{
				CommandUnit(*it, x, y, action, argument, queue, insert);
			}
		}

		void CommandUnits(vector<Dimension::Unit*> pUnits, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert)
		{
			for (vector<Dimension::Unit*>::iterator it = pUnits.begin(); it != pUnits.end(); it++)
			{
				CommandUnit(*it, destination, action, argument, queue, insert);
			}
		}

		void CancelAction(Dimension::Unit* pUnit, unsigned int num)
		{
			if (num < pUnit->actionQueue.size())
			{
				if (num == 0)
				{
					CancelAction(pUnit);
				}
				else
				{
					pUnit->actionQueue.erase(pUnit->actionQueue.begin() + num);
				}
			}
		}
		
		void IssueNextAction(Dimension::Unit* pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			pUnit->pMovementData->action.action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			pUnit->pMovementData->action.goal.goal_id = 0xFFFF;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;

			while (pUnit->actionQueue.size())
			{
				Game::Dimension::CheckGhostUnits(pUnit->actionQueue.front());

				delete pUnit->actionQueue.front();
				pUnit->actionQueue.pop_front();
				if (pUnit->actionQueue.size())
				{
					if (pUnit->actionQueue.front()->goal_unit)
					{
						Dimension::ActionData *actiondata = pUnit->actionQueue.front();
						if (Dimension::IsDisplayedUnitPointer(actiondata->goal_unit))
						{
							ScheduleCommandUnit(pUnit, actiondata->goal_unit, actiondata->action, actiondata->arg);
							return;
						}
					}
					else
					{
						Dimension::ActionData *actiondata = pUnit->actionQueue.front();
						ScheduleCommandUnit(pUnit, actiondata->goal_pos.x, actiondata->goal_pos.y, actiondata->action, actiondata->arg);
						return;
					}
				}
			}
			AI::SendUnitEventToLua_BecomeIdle(pUnit);
		}

		void CancelAction(Dimension::Unit* pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			if (pUnit->pMovementData->action.action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->pMovementData->action.action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}

			AI::SendUnitEventToLua_CommandCancelled(pUnit);
			IssueNextAction(pUnit);
		}
		
		void CancelAllActions(Dimension::Unit* pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			AI::SendUnitEventToLua_CommandCancelled(pUnit);
			if (pUnit->pMovementData->action.action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->pMovementData->action.action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			pUnit->pMovementData->action.action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
			pUnit->actionQueue.clear();
		}
		
		void CompleteAction(Dimension::Unit* pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			AI::SendUnitEventToLua_CommandCompleted(pUnit);
			IssueNextAction(pUnit);
		}
		
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, int goal_x, int goal_y, Dimension::Unit* target, void* arg)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			if (action == ACTION_ATTACK || action == ACTION_FOLLOW || action == ACTION_MOVE_ATTACK_UNIT)
			{
				if (!target)
				{
					*(int*) 0 = 0;
				}
			}

			if (action == ACTION_BUILD)
			{
				if (!arg && !target)
				{
					*(int*) 0 = 0;
				}
			}
			
			if (action == ACTION_RESEARCH)
			{
				if (!arg)
				{
					*(int*) 0 = 0;
				}
			}

			if (pUnit->pMovementData->action.action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->pMovementData->action.action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			if (pUnit->pMovementData->action.action != ACTION_NONE)
			{
				AI::SendUnitEventToLua_CommandCancelled(pUnit);
			}
			
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
			
			pUnit->pMovementData->action.action = action;
			pUnit->pMovementData->action.goal.unit = target;
			if (target)
			{
				pUnit->pMovementData->action.goal.goal_id = target->id;
			}
			else
			{
				pUnit->pMovementData->action.goal.goal_id = 0xFFFF;
			}
			pUnit->pMovementData->action.goal.pos.x = goal_x;
			pUnit->pMovementData->action.goal.pos.y = goal_y;
			pUnit->pMovementData->action.arg = arg;
			pUnit->action_completeness = 0.0;
			if (pUnit->type->isMobile)
			{
				pUnit->isMoving = true;
			}
			pUnit->faceTarget = Dimension::FACETARGET_NONE;
			AI::SendUnitEventToLua_NewCommand(pUnit);
				
			if (!pUnit->actionQueue.size())
			{
				// When an AI executes a command, it does not go through CommandUnit().
				// If you try to perform a 'collaboration-game' with the AI (to do that,
				// make your own player an AI and hack the AI script to allow commanding)
				// you will notice that if you give a command to a unit, it will cancel the
				// last command even if you gave it a 'queueing' command. This fixes it,
				// by ensuring that the AI's commands will be in the command queue too.
				Dimension::ActionData* actiondata = new Dimension::ActionData;
				actiondata->action = action;
				actiondata->goal_pos.x = goal_x;
				actiondata->goal_pos.y = goal_y;
				actiondata->goal_unit = target;
				actiondata->arg = arg;
				actiondata->visual_repr = Dimension::PrepareActionDataForVisualRepr(pUnit, action, arg, goal_x, goal_y);

				pUnit->actionQueue.push_back(actiondata);
			}

		}
	}
}

