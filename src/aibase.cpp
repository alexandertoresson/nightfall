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

namespace Game
{
	namespace AI
	{
		int aiFps = 30;
		int resetFrame = 0;   // for determining when to reset Dimension::PositionSearch_NumStepsTaken
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

		void SendCommandUnitToLua(Dimension::Unit* pUnit, float x, float y, UnitAction action, void* argument)
		{
			pUnit->lastCommand = SDL_GetTicks();
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
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
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushnumber(pVM->GetVM(), x);
			lua_pushnumber(pVM->GetVM(), y);
			lua_pushnumber(pVM->GetVM(), action);
			lua_pushlightuserdata(pVM->GetVM(), argument);
			pVM->CallFunction(5);
		}

		void SendCommandUnitToLua(Dimension::Unit* pUnit, Dimension::Unit* destination, UnitAction action, void* argument)
		{
			pUnit->lastCommand = SDL_GetTicks();
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
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
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), destination);
			lua_pushnumber(pVM->GetVM(), action);
			lua_pushlightuserdata(pVM->GetVM(), argument);
			pVM->CallFunction(4);
		}

		void SendActionUnitEventToLua(Dimension::Unit* pUnit, std::string func)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);

			if (func.length() == 0)
				return;

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			pVM->SetFunction(func);

			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushinteger(pVM->GetVM(), pUnit->action);
			lua_pushnumber(pVM->GetVM(), pUnit->pMovementData->action.goal.pos.x);
			lua_pushnumber(pVM->GetVM(), pUnit->pMovementData->action.goal.pos.y);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.goal.unit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.arg);
			pVM->CallFunction(6);
		}

		void SendUnitEventToLua_CommandCompleted(Dimension::Unit* pUnit)
		{
			SendActionUnitEventToLua(pUnit, pUnit->unitAIFuncs.commandCompleted);
		}

		void SendUnitEventToLua_CommandCancelled(Dimension::Unit* pUnit)
		{
			SendActionUnitEventToLua(pUnit, pUnit->unitAIFuncs.commandCancelled);
		}

		void SendUnitEventToLua_NewCommand(Dimension::Unit* pUnit)
		{
			SendActionUnitEventToLua(pUnit, pUnit->unitAIFuncs.newCommand);
		}

		void SendUnitEventToLua_BecomeIdle(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			
			if (pUnit->unitAIFuncs.becomeIdle.length() == 0)
				return;

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			pVM->SetFunction(pUnit->unitAIFuncs.becomeIdle);

			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			pVM->CallFunction(1);
		}

		void SendUnitEventToLua_IsAttacked(Dimension::Unit* pUnit, Dimension::Unit* attacker)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			
			if (pUnit->unitAIFuncs.isAttacked.length() == 0)
				return;

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			pVM->SetFunction(pUnit->unitAIFuncs.isAttacked);

			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), attacker);
			pVM->CallFunction(2);
		}

		void SendUnitEventToLua_UnitCreation(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			
			if (pUnit->type->playerAIFuncs[pUnit->owner->index].unitCreation.length() == 0)
				return;

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			pVM->SetFunction(pUnit->type->playerAIFuncs[pUnit->owner->index].unitCreation);

			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->type);
			lua_pushnumber(pVM->GetVM(), pUnit->pos.x);
			lua_pushnumber(pVM->GetVM(), pUnit->pos.y);
			pVM->CallFunction(4);
		}

		void SendUnitEventToLua_UnitKilled(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			
			if (pUnit->unitAIFuncs.unitKilled.length() == 0)
				return;

			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;

			pVM->SetFunction(pUnit->unitAIFuncs.unitKilled);

			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			pVM->CallFunction(1);
		}

		void SendUnitEventToLua_BuildComplete(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("UnitEvent_BuildComplete_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("UnitEvent_BuildComplete_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("UnitEvent_BuildComplete_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.arg);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.goal.unit);
			pVM->CallFunction(3);
		}

		void SendUnitEventToLua_BuildCancelled(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("UnitEvent_BuildCancelled_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("UnitEvent_BuildCancelled_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("UnitEvent_BuildCancelled_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.arg);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.goal.unit);
			pVM->CallFunction(3);
		}

		void SendUnitEventToLua_ResearchComplete(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("UnitEvent_ResearchComplete_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("UnitEvent_ResearchComplete_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("UnitEvent_ResearchComplete_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.arg);
			pVM->CallFunction(2);
		}

		void SendUnitEventToLua_ResearchCancelled(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);
			if (pUnit->owner->type == Dimension::PLAYER_TYPE_REMOTE)
				return;
			switch (pUnit->owner->type)
			{
				case Dimension::PLAYER_TYPE_HUMAN:
					pVM->SetFunction("UnitEvent_ResearchCancelled_Human");
					break;
				case Dimension::PLAYER_TYPE_GAIA:
					pVM->SetFunction("UnitEvent_ResearchCancelled_Gaia");
					break;
				case Dimension::PLAYER_TYPE_AI:
					pVM->SetFunction("UnitEvent_ResearchCancelled_AI");
					break;
				case Dimension::PLAYER_TYPE_REMOTE:
					break;
			}
			lua_pushlightuserdata(pVM->GetVM(), pUnit);
			lua_pushlightuserdata(pVM->GetVM(), pUnit->pMovementData->action.arg);
			pVM->CallFunction(2);
		}

		void PerformSimpleAI(Dimension::Unit* pUnit)
		{
			UnitAction action;
			int should_move;
			
			action = pUnit->action;
			
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

		void PerformAI(Dimension::Unit* pUnit)
		{
			double power_usage;
			HandleProjectiles(pUnit);
			if (pUnit->isCompleted && pUnit->action != ACTION_DIE)
			{

				power_usage = (pUnit->type->powerUsage + pUnit->type->lightPowerUsage) / aiFps;
				
				if (pUnit->owner->resources.power < power_usage)
				{
					NotEnoughPowerForLight(pUnit);
					return;
				}
					
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

				if (pUnit->action == AI::ACTION_NONE || pUnit->action == AI::ACTION_NETWORK_AWAITING_SYNC)
				{
					pUnit->isMoving = false;
				}

				if (pUnit->type->hasAI) // _I_ has an AI!
				{

					pUnit->aiFrame++;

					PerformSimpleAI(pUnit);

					if (pUnit->aiFrame >= pUnit->unitAIFuncs.unitAIDelay && pUnit->owner->type != Dimension::PLAYER_TYPE_REMOTE)
					{
						Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(pUnit->owner->index);

						if (pUnit->unitAIFuncs.performUnitAI.length())
						{
							pVM->SetFunction(pUnit->unitAIFuncs.performUnitAI);

							lua_pushlightuserdata(pVM->GetVM(), pUnit);
							pVM->CallFunction(1);
						}
						pUnit->aiFrame = 0;
					}
				}
			}
			
		}

		void PerformAI(Dimension::Player* player)
		{
			player->oldResources = player->resources;
			player->aiFrame++;
			if (player->aiFrame >= player->playerAIFuncs.playerAIDelay && player->type != Dimension::PLAYER_TYPE_REMOTE)
			{
				Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::GetPlayerVMInstance(player->index);

				if (player->playerAIFuncs.performPlayerAI.length())
				{
					pVM->SetFunction(player->playerAIFuncs.performPlayerAI);

					lua_pushlightuserdata(pVM->GetVM(), player);
					pVM->CallFunction(1);
				}
				player->aiFrame = 0;
			}
		}

		void PerformAIFrame()
		{
			static bool may_run_ai = true;
			if (may_run_ai)
			{
				aiFramesPerformedSinceLastRender++;
				Dimension::Environment::FourthDimension::Instance()->RotateWorld(1.00 / aiFps);

				SDL_LockMutex(AI::GetMutex());

				for (set<Dimension::Unit*>::iterator it = doneUnits.begin(); it != doneUnits.end(); it++)
				{
					Dimension::Unit* pUnit = *it;
					if (!Dimension::IsValidUnitPointer(pUnit) || pUnit->action == ACTION_DIE)
					{
						continue;
					}
					PathState state = GetInternalPathState(pUnit);
					
					if (!Networking::isNetworked)
					{
						if (state == PATHSTATE_GOAL)
						{
							ApplyNewPath(pUnit);
							pUnit->pMovementData->pCurGoalNode = NULL;
						}
						else if (state == PATHSTATE_ERROR)
						{
							CancelAction(pUnit);
							pUnit->pMovementData->pCurGoalNode = NULL;
						}
					}
					else
					{
						if (state == PATHSTATE_GOAL)
						{
							Networking::PreparePath(pUnit, pUnit->pMovementData->_start, pUnit->pMovementData->_goal);
							DeallocPathfindingNodes(pUnit, AI::DPN_BACK);
						}
						else if (state == PATHSTATE_ERROR)
						{
							DeallocPathfindingNodes(pUnit, AI::DPN_BACK);
							ScheduleNextAction(pUnit);
						}
					}
				}

				doneUnits.clear();

				SDL_UnlockMutex(AI::GetMutex());

				for (vector<Dimension::Player*>::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
				{
					PerformAI(*it);
				}
				for (unsigned int i = 0; i < Dimension::pWorld->vUnits.size(); i++)
				{
					Dimension::Unit* pUnit = Dimension::pWorld->vUnits.at(i);
					PerformAI(pUnit);
					if (pUnit->action == ACTION_DIE && currentFrame - pUnit->lastAttacked > (unsigned) aiFps)
					{
						DeleteUnit(pUnit);
						i--;
					}
				}
				FX::pParticleSystems->Iterate(1.0f / aiFps);
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

			if (pUnit->isCompleted && pUnit->action != ACTION_DIE)
			{
				if (!queue)
				{
					while (pUnit->actionQueue.size())
					{
						delete pUnit->actionQueue.front();
						pUnit->actionQueue.pop_front();
					}
				}

				Dimension::ActionData* actiondata = new Dimension::ActionData;
				actiondata->action = action;
				actiondata->goal_pos.x = x;
				actiondata->goal_pos.y = y;
				actiondata->goal_unit = NULL;
				actiondata->arg = argument;

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
					SendCommandUnitToLua(pUnit, x, y, action, argument);
				}

			}
		}

		void CommandUnit(Dimension::Unit* pUnit, Dimension::Unit* destination, UnitAction action, void* argument, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, destination->curAssociatedSquare.x, destination->curAssociatedSquare.y))
				return;
			
			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted && pUnit->action != ACTION_DIE)
			{
				if (!queue)
				{
					while (pUnit->actionQueue.size())
					{
						delete pUnit->actionQueue.front();
						pUnit->actionQueue.pop_front();
					}
				}

				Dimension::ActionData* actiondata = new Dimension::ActionData;
				actiondata->action = action;
				actiondata->goal_pos.x = 0;
				actiondata->goal_pos.y = 0;
				actiondata->goal_unit = destination;
				actiondata->arg = argument;

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
					SendCommandUnitToLua(pUnit, destination, action, argument);
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
			if (pUnit->action == AI::ACTION_DIE)
			{
				return;
			}

			pUnit->action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			pUnit->pMovementData->action.goal.goal_id = 0xFFFF;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;

			if (pUnit->actionQueue.size())
			{
				delete pUnit->actionQueue.front();
				pUnit->actionQueue.pop_front();
			}

			if (pUnit->actionQueue.size())
			{
				if (pUnit->actionQueue.front()->goal_unit)
				{
					Dimension::ActionData *actiondata = pUnit->actionQueue.front();
					SendCommandUnitToLua(pUnit, actiondata->goal_unit, actiondata->action, actiondata->arg);
				}
				else
				{
					Dimension::ActionData *actiondata = pUnit->actionQueue.front();
					SendCommandUnitToLua(pUnit, actiondata->goal_pos.x, actiondata->goal_pos.y, actiondata->action, actiondata->arg);
				}
			}
			else
			{
				AI::SendUnitEventToLua_BecomeIdle(pUnit);
			}
		}

		void CancelAction(Dimension::Unit* pUnit)
		{
			if (pUnit->action == AI::ACTION_DIE)
			{
				return;
			}

			if (pUnit->action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			AI::SendUnitEventToLua_CommandCancelled(pUnit);
			IssueNextAction(pUnit);
		}
		
		void CancelAllActions(Dimension::Unit* pUnit)
		{
			if (pUnit->action == AI::ACTION_DIE)
			{
				return;
			}

			AI::SendUnitEventToLua_CommandCancelled(pUnit);
			if (pUnit->action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			pUnit->action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
			pUnit->actionQueue.clear();
		}
		
		void CompleteAction(Dimension::Unit* pUnit)
		{
			if (pUnit->action == AI::ACTION_DIE)
			{
				return;
			}

			if (pUnit->action == ACTION_BUILD)
			{
				AI::SendUnitEventToLua_BuildComplete(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				AI::SendUnitEventToLua_ResearchComplete(pUnit);
			}
			AI::SendUnitEventToLua_CommandCompleted(pUnit);
			IssueNextAction(pUnit);
		}
		
		void ScheduleNextAction(Dimension::Unit* pUnit)
		{
			if (pUnit->actionQueue.size())
			{
				delete pUnit->actionQueue.front();
				pUnit->actionQueue.pop_front();
				if (pUnit->actionQueue.size())
				{
					if (pUnit->actionQueue.front()->goal_unit)
					{
						Dimension::ActionData *actiondata = pUnit->actionQueue.front();
						SendCommandUnitToLua(pUnit, actiondata->goal_unit, actiondata->action, actiondata->arg);
					}
					else
					{
						Dimension::ActionData *actiondata = pUnit->actionQueue.front();
						SendCommandUnitToLua(pUnit, actiondata->goal_pos.x, actiondata->goal_pos.y, actiondata->action, actiondata->arg);
					}
				}
			}
		}
		
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, int goal_x, int goal_y, Dimension::Unit* target, void* arg)
		{
			if (pUnit->action == AI::ACTION_DIE)
			{
				return;
			}

			if (pUnit->action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			if (pUnit->action != ACTION_NONE)
			{
				AI::SendUnitEventToLua_CommandCancelled(pUnit);
			}
			
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
			
			pUnit->action = action;
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

				pUnit->actionQueue.push_back(actiondata);
			}

		}
	}
}

