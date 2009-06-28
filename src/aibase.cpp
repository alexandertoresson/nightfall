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
#include "aibase.h"

#include "game.h"
#include "dimension.h"
#include "ainode.h"
#include "unit.h"
#include "unitsquares.h"
#include "aipathfinding.h"
#include "luawrapper.h"
#include "networking.h"
#include "environment.h"
#include "unitinterface.h"
#include "effect.h"
#include "utilities.h"
#include "unit.h"
#include "lockfreequeue.h"
#include <cmath>

using namespace std;

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

		SDL_mutex* updateMutex = NULL;

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

		struct ScheduledCommand : public Dimension::BaseActionData
		{
			gc_ptr<Dimension::Unit> unit;

			ScheduledCommand(gc_ptr<Dimension::Unit> unit, const Dimension::ActionQueueItem& a) : BaseActionData(a.goal, a.action, a.rotation, a.args), unit(unit)
			{
				
			}
		};

		void SendCommandUnitToLua(const ScheduledCommand& command)
		{
			gc_ptr<Utilities::Scripting::LuaVMState> pVM = command.unit->owner->aiState;
			if (command.unit->owner->isRemote)
				return;

			pVM->SetFunction(command.unit->owner->playerAIFuncs.commandUnit.func);

			lua_pushlightuserdata(pVM->GetState(), (void*) command.unit->GetHandle());
			lua_pushlightuserdata(pVM->GetState(), (void*) (command.goal.unit ? command.goal.unit->GetHandle() : -1));
			lua_pushnumber(pVM->GetState(), command.goal.pos.x);
			lua_pushnumber(pVM->GetState(), command.goal.pos.y);
			lua_pushnumber(pVM->GetState(), command.action);
			lua_pushlightuserdata(pVM->GetState(), (void*) command.args.argHandle);
			lua_pushnumber(pVM->GetState(), command.rotation);
			pVM->CallFunction(7);
		}

		std::vector<ScheduledCommand> scheduledCommands;

		void ScheduleCommandUnit(const gc_ptr<Dimension::Unit>& unit, const Dimension::ActionQueueItem& actiondata)
		{
			unit->lastCommand = SDL_GetTicks();
			scheduledCommands.push_back(ScheduledCommand(unit, actiondata));
		}

		void ApplyScheduledCommandUnits()
		{
			for (std::vector<ScheduledCommand>::iterator it = scheduledCommands.begin(); it != scheduledCommands.end(); it++)
			{
				const ScheduledCommand& command = *it;
				SendCommandUnitToLua(command);
			}
			scheduledCommands.clear();
		}

		void SendScheduledUnitEvents(gc_ptr<Dimension::Player> player)
		{
			gc_ptr<Dimension::UnitEvent> event;
			while (event = player->scheduledUnitEvents.consume())
			{
				gc_ptr<Utilities::Scripting::LuaVMState> pVM = event->unit->owner->aiState;
				pVM->SetFunction(event->func);
				switch (event->eventType)
				{
					case Dimension::UNITEVENTTYPE_ACTION:
						lua_pushlightuserdata(pVM->GetState(), (void*) event->unit->GetHandle());
						lua_pushinteger(pVM->GetState(), event->action);
						lua_pushnumber(pVM->GetState(), event->goal.pos.x);
						lua_pushnumber(pVM->GetState(), event->goal.pos.y);
						lua_pushlightuserdata(pVM->GetState(), (void*) (event->goal.unit ? event->goal.unit->GetHandle() : -1));
						lua_pushlightuserdata(pVM->GetState(), (void*) event->args.argHandle);
						pVM->CallFunction(6);
						break;
					case Dimension::UNITEVENTTYPE_SIMPLE:
						lua_pushlightuserdata(pVM->GetState(), (void*) event->unit->GetHandle());
						pVM->CallFunction(1);
						break;
					case Dimension::UNITEVENTTYPE_ATTACK:
						lua_pushlightuserdata(pVM->GetState(), (void*) event->unit->GetHandle());
						lua_pushlightuserdata(pVM->GetState(), (void*) event->goal.unit->GetHandle());
						pVM->CallFunction(2);
						break;
				}
			}
		}

		void ScheduleActionUnitEvent(const gc_ptr<Dimension::Unit>& pUnit, EventAIFunc *aiEvent)
		{

			if (pUnit->owner->isRemote)
				return;

			SDL_LockMutex(pUnit->owner->scheduleUnitEventMutex);
			pUnit->owner->scheduledUnitEvents.produce(new Dimension::UnitEvent(pUnit, aiEvent, Dimension::UNITEVENTTYPE_ACTION));
			SDL_UnlockMutex(pUnit->owner->scheduleUnitEventMutex);

		}

		void ScheduleSimpleUnitEvent(const gc_ptr<Dimension::Unit>& pUnit, EventAIFunc *aiEvent)
		{

			if (pUnit->owner->isRemote)
				return;

			SDL_LockMutex(pUnit->owner->scheduleUnitEventMutex);
			pUnit->owner->scheduledUnitEvents.produce(new Dimension::UnitEvent(pUnit, aiEvent, Dimension::UNITEVENTTYPE_SIMPLE));
			SDL_UnlockMutex(pUnit->owner->scheduleUnitEventMutex);
		}

		void SendUnitEventToLua_CommandCompleted(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.commandCompleted);
		}

		void SendUnitEventToLua_CommandCancelled(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.commandCancelled);
		}

		void SendUnitEventToLua_NewCommand(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleActionUnitEvent(pUnit, &pUnit->unitAIFuncs.newCommand);
		}

		void SendUnitEventToLua_BecomeIdle(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->unitAIFuncs.becomeIdle);
		}

		void SendUnitEventToLua_UnitCreation(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->type->playerAIFuncs.unitCreation);
		}

		void SendUnitEventToLua_UnitKilled(const gc_ptr<Dimension::Unit>& pUnit)
		{
			ScheduleSimpleUnitEvent(pUnit, &pUnit->unitAIFuncs.unitKilled);
		}

		void SendUnitEventToLua_IsAttacked(const gc_ptr<Dimension::Unit>& pUnit, const gc_ptr<Dimension::Unit>& attacker)
		{
			
			if (pUnit->owner->isRemote)
				return;

			SDL_LockMutex(pUnit->owner->scheduleUnitEventMutex);
			pUnit->owner->scheduledUnitEvents.produce(new Dimension::UnitEvent(pUnit, attacker, &pUnit->type->unitAIFuncs.isAttacked));
			SDL_UnlockMutex(pUnit->owner->scheduleUnitEventMutex);
		}

		void HandleUnitPower()
		{
			double power_usage;
			vector<gc_ptr<Dimension::Unit> >::iterator it_end = Dimension::pWorld->vUnits.end();
			for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnits.begin(); it != it_end; it++)
			{
				const gc_ptr<Dimension::Unit>& pUnit = *it;
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

		void PerformSimpleAI(gc_ptr<Dimension::Unit>& pUnit)
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
						gc_ptr<Dimension::Unit>& targetUnit = pUnit->pMovementData->action.goal.unit;
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
							Dimension::PerformBuild(pUnit);
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
						Dimension::PerformResearch(pUnit);
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
						if (!pUnit->soundNodes[Audio::SFX_ACT_MOVE_RPT].active)
						{
							PlayRepeatingActionSound(pUnit, Audio::SFX_ACT_MOVE_RPT);
						}
					}
					else
					{
						if (pUnit->soundNodes[Audio::SFX_ACT_MOVE_RPT].active)
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

		void PerformVerySimpleAI(const gc_ptr<Dimension::Unit>& pUnit)
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

		void PerformLuaUnitAI(const gc_ptr<Dimension::Unit>& pUnit)
		{
			if (pUnit->hasPower)
			{

				pUnit->aiFrame++;

				if (pUnit->aiFrame >= pUnit->unitAIFuncs.performUnitAI.delay && !pUnit->owner->isRemote)
				{
					gc_ptr<Utilities::Scripting::LuaVMState> pVM = pUnit->owner->aiState;

					if (pUnit->unitAIFuncs.performUnitAI.enabled)
					{
						pVM->SetFunction(pUnit->unitAIFuncs.performUnitAI.func);

						lua_pushlightuserdata(pVM->GetState(), (void*) pUnit->GetHandle());
						lua_pushinteger(pVM->GetState(), pUnit->pMovementData->action.action);
						pVM->CallFunction(2);
					}
					pUnit->aiFrame = 0;
				}
			}
			
		}

		void PerformLuaPlayerAI(const gc_ptr<Dimension::Player>& player)
		{
			SendScheduledUnitEvents(player);
			player->aiFrame++;
			if (player->aiFrame >= player->playerAIFuncs.performPlayerAI.delay && !player->isRemote)
			{
				gc_ptr<Utilities::Scripting::LuaVMState> pVM = player->aiState;

				if (player->playerAIFuncs.performPlayerAI.enabled)
				{
					pVM->SetFunction(player->playerAIFuncs.performPlayerAI.func);

					lua_pushlightuserdata(pVM->GetState(), (void*) player->GetHandle());
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

		vector<gc_ptr<Dimension::Player> > playersToBeProcessed;
		SDL_mutex *playersToProcessMutex;
		volatile int aiThreadsDone;
		volatile bool aiIsFired;
		volatile bool quitAIThreads;

		int simpleAITicks = 0;
		int *luaAITicks;
		int postFrameTicks[8];
		int GCTicks = 0;
		int totalAITicks = 0;

		int _SimpleAIThread(void* arg)
		{

			SDL_LockMutex(simpleAIWaitMutex);

			simpleAIThreadRunning = true;

			while (!quitAIThreads)
			{

				do
				{
					SDL_CondWait(fireAIConds[0], simpleAIWaitMutex);
				} while (!aiIsFired);
				
				Uint32 t = SDL_GetTicks();
				
				for (vector<gc_ptr<Dimension::Player> >::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
				{
					HandleProjectiles(*it);
				}

				for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
				{
					const gc_ptr<Dimension::Unit>& pUnit = *it;
					PerformVerySimpleAI(pUnit);
					if (pUnit->pMovementData->action.action == ACTION_DIE && currentFrame - pUnit->lastAttacked > (unsigned) aiFps)
					{
						ScheduleUnitDeletion(pUnit);
					}
				}

				for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnitsWithAI.begin(); it != Dimension::pWorld->vUnitsWithAI.end(); it++)
				{
					PerformSimpleAI(*it);
				}

				SDL_LockMutex(mainAIWaitMutexes[0]); // Make sure that the main thread is waiting for the signal
					simpleAITicks += SDL_GetTicks() - t;
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

			while (!quitAIThreads)
			{

				do
				{
					SDL_CondWait(fireAIConds[i+1], luaAIWaitMutexes[i]);
				} while (!aiIsFired);

				Uint32 t = SDL_GetTicks();
				
				while (1)
				{
					gc_ptr<Dimension::Player> player;

					SDL_LockMutex(playersToProcessMutex);

					if (playersToBeProcessed.size())
					{
						player = playersToBeProcessed.front();
						playersToBeProcessed.erase(playersToBeProcessed.begin());
					}
					else
					{
						SDL_UnlockMutex(playersToProcessMutex);
						break;
					}

					SDL_UnlockMutex(playersToProcessMutex);

					PerformLuaPlayerAI(player);

					for (vector<gc_ptr<Dimension::Unit> >::iterator it2 = player->vUnitsWithLuaAI.begin(); it2 != player->vUnitsWithLuaAI.end(); it2++)
					{
						PerformLuaUnitAI(*it2);
					}
				}

				SDL_LockMutex(mainAIWaitMutexes[i+1]); // Make sure that the main thread is waiting for the signal
					luaAITicks[i] += SDL_GetTicks() - t;
					aiThreadsDone++;
				SDL_UnlockMutex(mainAIWaitMutexes[i+1]);

				SDL_CondBroadcast(luaAIdoneConds[i]);
			}

			return 1;
		}

		void InitAIMiscMutexes()
		{
			updateMutex = SDL_CreateMutex();
		}

		void InitAIThreads()
		{
			if (numLuaAIThreads)
			{

				playersToProcessMutex = SDL_CreateMutex();

				quitAIThreads = false;
				simpleAIdoneCond = SDL_CreateCond();
				simpleAIWaitMutex = SDL_CreateMutex();

				luaAIdoneConds = new SDL_cond*[numLuaAIThreads];
				luaAIWaitMutexes = new SDL_mutex*[numLuaAIThreads];
				luaAIThreadsRunning = new bool[numLuaAIThreads];
				for (int i = 0; i < numLuaAIThreads; i++)
				{
					luaAIdoneConds[i] = SDL_CreateCond();
					luaAIWaitMutexes[i] = SDL_CreateMutex();
					luaAIThreadsRunning[i] = false;
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

			luaAITicks = new int[numLuaAIThreads];


			for (int i = 0; i < numLuaAIThreads; i++)
			{
				luaAITicks[i] = 0;
			}

		}

		void QuitAIThreads()
		{
			if (numLuaAIThreads)
			{
				quitAIThreads = true;

				SDL_UnlockMutex(simpleAIWaitMutex);
				for (int i = 0; i < numLuaAIThreads; i++)
				{
					SDL_UnlockMutex(luaAIWaitMutexes[i]);
				}
				
				SDL_WaitThread(simpleAIThread, NULL);
				
				for (int i = 0; i < numLuaAIThreads; i++)
				{
					SDL_WaitThread(luaAIThreads[i], NULL);
				}
				
				SDL_DestroyCond(simpleAIdoneCond);
				SDL_DestroyMutex(simpleAIWaitMutex);

				for (int i = 0; i < numLuaAIThreads; i++)
				{
					SDL_DestroyCond(luaAIdoneConds[i]);
					SDL_DestroyMutex(luaAIWaitMutexes[i]);
				}
				delete[] luaAIdoneConds;
				delete[] luaAIWaitMutexes;
				delete[] luaAIThreadsRunning;

				for (int i = 0; i < numLuaAIThreads+1; i++)
				{
					SDL_DestroyCond(fireAIConds[i]);
					SDL_DestroyMutex(mainAIWaitMutexes[i]);
				}
				delete[] fireAIConds;
				delete[] mainAIWaitMutexes;

			}

		}

		void PerformAIFrame()
		{
			Uint32 t;
			Dimension::PrintPlayerRefs();

			static bool may_run_ai = true;
			if (may_run_ai)
			{
				aiFramesPerformedSinceLastRender++;
				Dimension::Environment::FourthDimension::Instance()->RotateWorld(1.00f / (float) aiFps);

				///////////////////////////////////////////////////////////////////////////
				// Apply Paths

				ApplyAllNewPaths();

				for (vector<gc_ptr<Dimension::Player> >::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
				{
					const gc_ptr<Dimension::Player>& player = *it;
					player->oldResources = player->resources;
				}

				t = SDL_GetTicks();

				if (numLuaAIThreads)
				{

					playersToBeProcessed = Dimension::pWorld->vPlayers;

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
					
					for (vector<gc_ptr<Dimension::Player> >::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
					{
						const gc_ptr<Dimension::Player>& player = *it;
						PerformLuaPlayerAI(player);
						for (vector<gc_ptr<Dimension::Unit> >::iterator it2 = player->vUnitsWithLuaAI.begin(); it2 != player->vUnitsWithLuaAI.end(); it2++)
						{
							PerformLuaUnitAI(*it2);
						}
					}

					for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
					{
						const gc_ptr<Dimension::Unit>& pUnit = *it;
						PerformVerySimpleAI(pUnit);
						if (pUnit->pMovementData->action.action == ACTION_DIE && currentFrame - pUnit->lastAttacked > (unsigned) aiFps)
						{
							ScheduleUnitDeletion(pUnit);
						}
					}

					for (vector<gc_ptr<Dimension::Unit> >::iterator it = Dimension::pWorld->vUnitsWithAI.begin(); it != Dimension::pWorld->vUnitsWithAI.end(); it++)
					{
						PerformSimpleAI(*it);
					}

				}

				totalAITicks += SDL_GetTicks() - t;

				t = SDL_GetTicks();
				
				///////////////////////////////////////////////////////////////////////////
				// Particle system!

				FX::pParticleSystems->Iterate(1.0f / (float) aiFps);
				
				postFrameTicks[0] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				///////////////////////////////////////////////////////////////////////////
				// Apply various stuff that cannot be applied while the lua and simpleai threads are running

				Dimension::DisplayScheduledUnits();
				
				postFrameTicks[1] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				Dimension::ApplyScheduledBigSquareUpdates();

				postFrameTicks[2] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				UnitLuaInterface::ApplyScheduledActions();

				postFrameTicks[3] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				UnitLuaInterface::ApplyScheduledDamagings();

				postFrameTicks[4] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				// Send unit commands that could not be sent while lua ai was running
				ApplyScheduledCommandUnits();

				postFrameTicks[5] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				SDL_LockMutex(updateMutex);

				// Apply deletions last, so it may 'undo' actions that have been applied before,
				// otherwise if you apply actions after it, you may apply actions with targets
				// that are deleted units.
				Dimension::DeleteScheduledUnits(); 

				SDL_UnlockMutex(updateMutex);

				postFrameTicks[5] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				// Delete units that have lost their ground for existance, and 'undo' researches
				// that also have.
				Dimension::EnforceMinimumExistanceRequirements();

				postFrameTicks[6] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				postFrameTicks[7] += SDL_GetTicks() - t;
				t = SDL_GetTicks();

				static int i = 0;
				if (i % 1000 == 0)
				{
					Uint32 ticks = SDL_GetTicks();

					Game::Rules::GameWindow::Instance()->PauseRendering();
					AI::PausePathfinding();

					gc_marker_base::sweep();

					Game::Rules::GameWindow::Instance()->ResumeRendering();
					AI::ResumePathfinding();

					GCTicks += SDL_GetTicks() - ticks;

					std::cout << "gc " << GCTicks << " sat " << simpleAITicks << " tat " << totalAITicks;
					
					for (int i = 0; i < 8; i++)
					{
						std::cout << " pft" << i << " " << postFrameTicks[i];
						postFrameTicks[i] = 0;
					}

					for (int i = 0; i < numLuaAIThreads; i++)
					{
						std::cout << " lua" << i << " " << luaAITicks[i];
						luaAITicks[i] = 0;
					}

					std::cout << std::endl;

					GCTicks = 0;
					simpleAITicks = 0;
					totalAITicks = 0;

				}
				i++;

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

		bool ImmobilityCheck(const gc_ptr<Dimension::Unit>& pUnit, UnitAction action, int x, int y)
		{
			if (action == AI::ACTION_MOVE_ATTACK ||
				action == AI::ACTION_MOVE_ATTACK_UNIT ||
				action == AI::ACTION_GOTO)
			{
				if (!pUnit->type->isMobile)
				{
					if (pUnit->type->canBuild.size() > 0)
					{
						if (!pUnit->rallypoint)
							pUnit->rallypoint = new Dimension::IntPosition;

						pUnit->rallypoint->x = x;
						pUnit->rallypoint->y = y;
					}
					return true;
				}
			}

			return false;
		}

		void CommandUnit(const gc_ptr<Dimension::Unit>& pUnit, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, x, y))
				return;

			if (!pUnit->type->isMobile && action == ACTION_GOTO)
				return;

			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted && pUnit->pMovementData->action.action != ACTION_DIE)
			{
				float rotation = Utilities::RandomDegree();
				Dimension::ActionQueueItem actiondata(x, y, NULL, action, args, rotation, true);

				if (!queue)
				{
					pUnit->actionQueue.clear();
				}

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
					ScheduleCommandUnit(pUnit, actiondata);
				}

			}
		}

		void CommandUnit(const gc_ptr<Dimension::Unit>& pUnit, const gc_ptr<Dimension::Unit>& destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, destination->curAssociatedSquare.x, destination->curAssociatedSquare.y))
				return;
			
			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted && pUnit->pMovementData->action.action != ACTION_DIE)
			{
				float rotation = Utilities::RandomDegree();
				Dimension::ActionQueueItem actiondata(0, 0, destination, action, args, rotation, true);
				if (!queue)
				{
					pUnit->actionQueue.clear();
				}

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
					ScheduleCommandUnit(pUnit, actiondata);
				}

			}
		}

		void CommandUnits(const vector<gc_ptr<Dimension::Unit> >& pUnits, int x, int y, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert)
		{
			for (vector<gc_ptr<Dimension::Unit> >::const_iterator it = pUnits.begin(); it != pUnits.end(); it++)
			{
				CommandUnit(*it, x, y, action, args, queue, insert);
			}
		}

		void CommandUnits(const vector<gc_ptr<Dimension::Unit> >& pUnits, const gc_ptr<Dimension::Unit>& destination, UnitAction action, const Dimension::ActionArguments& args, bool queue, bool insert)
		{
			for (vector<gc_ptr<Dimension::Unit> >::const_iterator it = pUnits.begin(); it != pUnits.end(); it++)
			{
				CommandUnit(*it, destination, action, args, queue, insert);
			}
		}

		void CancelAction(const gc_ptr<Dimension::Unit>& pUnit, unsigned int num)
		{
			// TODO: Fix.
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
		
		void IssueNextAction(const gc_ptr<Dimension::Unit>& pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			pUnit->pMovementData->action.action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;

			while (pUnit->actionQueue.size())
			{
				pUnit->actionQueue.pop_front();
				if (pUnit->actionQueue.size())
				{
//					std::cout << "next" << std::endl;

					if (pUnit->actionQueue.front().goal.unit)
					{
						const Dimension::ActionQueueItem& actiondata = pUnit->actionQueue.front();
						if (Dimension::IsDisplayedUnitPointer(actiondata.goal.unit))
						{
							ScheduleCommandUnit(pUnit, actiondata);
							return;
						}
					}
					else
					{
						const Dimension::ActionQueueItem& actiondata = pUnit->actionQueue.front();
						ScheduleCommandUnit(pUnit, actiondata);
						return;
					}
				}
			}
			AI::SendUnitEventToLua_BecomeIdle(pUnit);
		}

		void CancelAction(const gc_ptr<Dimension::Unit>& pUnit)
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

			pUnit->pMovementData->action.action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;

//			std::cout << "cancel" << std::endl;

			AI::SendUnitEventToLua_CommandCancelled(pUnit);
			IssueNextAction(pUnit);
		}
		
		void CancelAllActions(const gc_ptr<Dimension::Unit>& pUnit)
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
		
		void CompleteAction(const gc_ptr<Dimension::Unit>& pUnit)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			AI::SendUnitEventToLua_CommandCompleted(pUnit);
			IssueNextAction(pUnit);
		}
		
		void ApplyAction(const gc_ptr<Dimension::Unit>& pUnit, UnitAction action, int goal_x, int goal_y, const gc_ptr<Dimension::Unit>& target, const Dimension::ActionArguments& args, float rotation)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
				return;
			}

			if (action == ACTION_ATTACK || action == ACTION_FOLLOW || action == ACTION_MOVE_ATTACK_UNIT)
			{
				if (!target)
				{
					std::cout << "Invalid action detected; needs target" << std::endl;
					return;
				}
			}

			if (action == ACTION_BUILD)
			{
				if (!args.unitType && !target)
				{
					std::cout << "Invalid action detected; needs unittype or target" << std::endl;
					return;
				}
			}
			
			if (action == ACTION_RESEARCH)
			{
				if (!args.research)
				{
					std::cout << "Invalid action detected; needs research" << std::endl;
					return;
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
			pUnit->pMovementData->action.goal.pos.x = goal_x;
			pUnit->pMovementData->action.goal.pos.y = goal_y;
			pUnit->pMovementData->action.args = args;
			pUnit->pMovementData->action.rotation = rotation;
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
				Dimension::ActionQueueItem actiondata(goal_x, goal_y, target, action, args, rotation, false);

				pUnit->actionQueue.push_back(actiondata);
			}

		}
	}
}

