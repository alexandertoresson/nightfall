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
		int aiFps = 60;
		int callLuaEveryXFrame = 6;
		int aiFrame = 0;      // for determining when to run lua ai
		int resetFrame = 0;   // for determining when to reset Dimension::PositionSearch_NumStepsTaken
		Uint32 currentFrame = 0; // for tracking the number of the current frame
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
				aiFrame++;
				if (aiFrame > callLuaEveryXFrame)
				{
					aiFrame = 0; 
				}
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
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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

		void SendUnitEventToLua_BuildComplete(Dimension::Unit* pUnit)
		{
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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
			Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
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
			UnitAction action = pUnit->action;
			int should_move;
			
			if (!Networking::isNetworked)
			{
				if (GetInternalPathState(pUnit) == PATHSTATE_GOAL)
				{
					ApplyNewPath(pUnit);
					pUnit->pMovementData->pCurGoalNode = NULL;
				}
				else if (GetInternalPathState(pUnit) == PATHSTATE_ERROR)
				{
					CancelAction(pUnit);
					pUnit->pMovementData->pCurGoalNode = NULL;
				}
			}
			else
			{
				if (GetInternalPathState(pUnit) == PATHSTATE_GOAL)
				{
					Networking::PreparePath(pUnit, pUnit->pMovementData->_start, pUnit->pMovementData->_goal);
				}
				else if (GetInternalPathState(pUnit) == PATHSTATE_ERROR)
				{
					DeallocPathfindingNodes(pUnit, AI::DPN_BACK);
					ScheduleNextAction(pUnit);
				}
			}

			
			if (action == ACTION_GOTO || action == ACTION_FOLLOW || action == ACTION_ATTACK || action == ACTION_BUILD || action == ACTION_RESEARCH)
			{
	
				should_move = pUnit->type->isMobile;

				if (Dimension::CanAttack(pUnit) && action == ACTION_ATTACK)
				{
					Dimension::Unit* targetUnit = pUnit->pMovementData->action.goal.unit;
					if (Dimension::CanReach(pUnit, targetUnit))
					{
						Dimension::InitiateAttack(pUnit, targetUnit);
						should_move = false;
						pUnit->isMoving = false;
					}
				}

				if (action == ACTION_BUILD)
				{
					if (IsWithinRangeForBuilding(pUnit))
					{
						Dimension::Build(pUnit);
						should_move = false;
						pUnit->isMoving = false;
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

				if (pUnit->action == AI::ACTION_NONE)
				{
					pUnit->isMoving = false;
				}

				if (pUnit->type->hasAI)
				{

					PerformSimpleAI(pUnit);

					if (aiFrame == callLuaEveryXFrame && pUnit->owner->type != Dimension::PLAYER_TYPE_REMOTE)
					{
						Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
						switch (pUnit->owner->type)
						{
							case Dimension::PLAYER_TYPE_HUMAN:
								pVM->SetFunction("PerformAI_Unit_Human");
								break;
							case Dimension::PLAYER_TYPE_GAIA:
								pVM->SetFunction("PerformAI_Unit_Gaia");
								break;
							case Dimension::PLAYER_TYPE_AI:
								pVM->SetFunction("PerformAI_Unit_AI");
								break;
							case Dimension::PLAYER_TYPE_REMOTE:
								break;
						}
						lua_pushlightuserdata(pVM->GetVM(), pUnit);
						pVM->CallFunction(1);
					}
				}
			}
			
		}

		void PerformAI(Dimension::Player* player)
		{
			player->oldResources = player->resources;
			if (aiFrame == callLuaEveryXFrame && player->type != Dimension::PLAYER_TYPE_REMOTE)
			{
				Utilities::Scripting::LuaVirtualMachine* pVM = Utilities::Scripting::LuaVirtualMachine::Instance();
				switch (player->type)
				{
					case Dimension::PLAYER_TYPE_HUMAN:
						pVM->SetFunction("PerformAI_Player_Human");
						break;
					case Dimension::PLAYER_TYPE_GAIA:
						pVM->SetFunction("PerformAI_Player_Gaia");
						break;
					case Dimension::PLAYER_TYPE_AI:
						pVM->SetFunction("PerformAI_Player_AI");
						break;
					case Dimension::PLAYER_TYPE_REMOTE:
						break;
				}
				lua_pushlightuserdata(pVM->GetVM(), player);
				pVM->CallFunction(1);
			}
		}

		void PerformAIFrame()
		{
			static bool may_run_ai = true;
			if (may_run_ai)
			{
				Dimension::Environment::FourthDimension::Instance()->RotateWorld(1.00 / aiFps);
				for (vector<Dimension::Player*>::iterator it = Dimension::pWorld->vPlayers.begin(); it != Dimension::pWorld->vPlayers.end(); it++)
				{
					PerformAI(*it);
				}
				for (unsigned int i = 0; i < Dimension::pWorld->vUnits.size(); i++)
				{
					Dimension::Unit* pUnit = Dimension::pWorld->vUnits.at(i);
					PerformAI(pUnit);
					if (pUnit->action == ACTION_DIE && currentFrame - pUnit->lastAttacked > 60)
					{
						DeleteUnit(pUnit);
						i--;
					}
				}
				if(!Networking::isDedicatedServer)
					FX::pParticleSystems->Iterate(1.0f / aiFps);
			}

			may_run_ai = NextFrame();

#ifndef USE_MULTITHREADED_CALCULATIONS
			PerformPathfinding();
#endif				
		}

		bool ImmobilityCheck(Dimension::Unit* pUnit, UnitAction action, float x, float y)
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
							pUnit->rallypoint = new Dimension::Position;

						pUnit->rallypoint->x = x;
						pUnit->rallypoint->y = y;
					}
					return true;
				}
			}

			return false;
		}

		void CommandUnit(Dimension::Unit* pUnit, float x, float y, UnitAction action, void* argument, bool queue, bool insert)
		{
			if (ImmobilityCheck(pUnit, action, x, y))
				return;

			if (!pUnit->type->isMobile && action == ACTION_GOTO)
				return;

			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted)
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
			if (ImmobilityCheck(pUnit, action, destination->pos.x, destination->pos.y))
				return;
			
			if (pUnit->owner != Dimension::currentPlayer)
				return;

			if (pUnit->isCompleted)
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

		void CommandUnits(vector<Dimension::Unit*> pUnits, float x, float y, UnitAction action, void* argument, bool queue, bool insert)
		{
			for (vector<Dimension::Unit*>::iterator it = pUnits.begin(); it != pUnits.end(); it++)
			{
/*				x -= (*it)->type->widthOnMap+1;
				while (!SquareIsWalkable_AllKnowing((*it)->type, (int) x, (int) y))
				{
					x += 1.0f;
					y -= 1.0f;
				}*/
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
		
		void CancelAction(Dimension::Unit* pUnit)
		{
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
		
		void CancelAllActions(Dimension::Unit* pUnit)
		{
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
			if (pUnit->action == ACTION_BUILD)
			{
				AI::SendUnitEventToLua_BuildComplete(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				AI::SendUnitEventToLua_ResearchComplete(pUnit);
			}
			pUnit->action = ACTION_NONE;
			pUnit->pMovementData->action.goal.unit = NULL;
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
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
		
		void ApplyAction(Dimension::Unit* pUnit, UnitAction action, float goal_x, float goal_y, Dimension::Unit* target, void* arg)
		{
			if (pUnit->action == ACTION_BUILD)
			{
				Game::Dimension::CancelBuild(pUnit);
			}
			else if (pUnit->action == ACTION_RESEARCH)
			{
				Game::Dimension::CancelResearch(pUnit);
			}
			
			DeallocPathfindingNodes(pUnit);
			pUnit->pMovementData->pCurGoalNode = NULL;
			
			pUnit->action = action;
			pUnit->pMovementData->action.goal.unit = target;
			pUnit->pMovementData->action.goal.pos.x = goal_x;
			pUnit->pMovementData->action.goal.pos.y = goal_y;
			pUnit->pMovementData->action.arg = arg;
			pUnit->action_completeness = 0.0;
		}
	}
}

