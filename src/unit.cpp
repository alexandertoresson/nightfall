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
#include "unit.h"

#include "aipathfinding.h"
#include "luawrapper.h"
#include "effect.h"
#include "networking.h"
#include "textures.h"
#include "environment.h"
#include "game.h"
#include "camera.h"
#include "window.h"
#include "unittype.h"
#include "unitsquares.h"
#include "unitrender.h"
#include "utilities.h"
#include "research.h"
#include <cstdarg>
#include <set>
#include <list>
#include "hashmap.h"

using namespace std;

namespace Game
{
	namespace Dimension
	{
		static vector<gc_ptr<Unit> > unitsSelected;
		vector<gc_ptr<Unit> > unitGroups[10];
		hashmap<gc_ptr<Unit>, bool> displayedUnitPointers;
		int**         numUnitsPerAreaMap;
		int           nextPushID = 1;

		ActionQueueItem::~ActionQueueItem()
		{
		}
		
		void ActionQueueItem::CreateVisualRepresentation()
		{
			return; // DISABLE: Causes problems for network games, as it causes handles to be handed out when they shouldn't

			if (action == AI::ACTION_BUILD && args.unitType)
				ghost = CreateGhostUnit(args.unitType);
			
			if (ghost)
			{
				ghost->pos.x = goal.pos.x;
				ghost->pos.y = goal.pos.y;
				ghost->rotation = rotation;
			}
		}

		void PlayActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action)
		{
			if (!unit)
				return;

			if (action > Audio::SFX_ACT_COUNT)
				return;

			Audio::AudioFXInfo* fx = unit->type->actionSounds[action];
			if (fx == NULL)
				return;

			Utilities::Vector3D groundPos = Dimension::GetTerrainCoord(unit->pos.x, unit->pos.y);
			Audio::PlayOnceFromLocation(fx->pSound, &fx->channel, groundPos,
				Game::Dimension::Camera::instance.GetPosVector(),
				fx->strength);
		}
		
		void PlayRepeatingActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action)
		{				
			if (unit->soundNodes[action].active)
				return;
				
			if (!unit->type->actionSounds[action])
				return;
			
			Audio::AudioFXInfo* inf = unit->type->actionSounds[action];
			
			Utilities::Vector3D groundPos = Dimension::GetTerrainCoord(unit->pos.x, unit->pos.y);
			unit->soundNodes[action].node = Audio::CreateSoundNode(inf->pSound, groundPos, Utilities::Vector3D(), inf->strength, -1);
			Audio::SetSpeakerUnit(unit->soundNodes[action].node, unit);
			unit->soundNodes[action].active = true;
		}
		
		void StopRepeatingActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action)
		{
			if (!unit)
				return;
				
			if (!unit->soundNodes[action].active)
				return;
				
			unit->soundNodes[action].active = false;
			Audio::SetSpeakerUnit(unit->soundNodes[action].node, NULL);
			Audio::RemoveSoundNode(unit->soundNodes[action].node);
		}
		
		bool IsDisplayedUnitPointer(const gc_ptr<Unit>& unit)
		{
			return displayedUnitPointers.get(unit);
		}

		gc_ptr<Unit> GetUnitByID(int id)
		{
			if (HandleManager<Unit>::IsCorrectHandle(id))
			{
				return HandleManager<Unit>::InterpretHandle(id);
			}
			return NULL;
		}

		double GetIncomeAtNoon(const gc_ptr<Player>& player)
		{
			double income = 0;
			for (vector<gc_ptr<Unit> >::iterator it = player->vUnits.begin(); it != player->vUnits.end(); it++)
			{
				if ((*it)->isDisplayed)
				{
					gc_ptr<UnitType> unittype = (*it)->type;
					income += unittype->powerIncrement;
					income -= unittype->powerUsage + unittype->lightPowerUsage + (unittype->attackPowerUsage + unittype->movePowerUsage + unittype->buildPowerUsage) * 0.5;
				}
			}
			return income;
		}

		double GetIncomeAtNight(const gc_ptr<Player>& player)
		{
			double income = 0;
			for (vector<gc_ptr<Unit> >::iterator it = player->vUnits.begin(); it != player->vUnits.end(); it++)
			{
				if ((*it)->isDisplayed)
				{
					gc_ptr<UnitType> unittype = (*it)->type;
					if (unittype->powerType == POWERTYPE_TWENTYFOURSEVEN)
					{
						income += unittype->powerIncrement;
					}
					income -= unittype->powerUsage + unittype->lightPowerUsage + (unittype->attackPowerUsage + unittype->movePowerUsage + unittype->buildPowerUsage) * 0.5;
				}
			}
			return income;
		}

		double GetNightLength()
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			return 12 * pDimension->GetHourLength();
		}

		double GetDayLength()
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			return 12 * pDimension->GetHourLength();
		}

		double GetPower(const gc_ptr<Player>& player)
		{
			return player->resources.power;
		}

		double GetMoney(const gc_ptr<Player>& player)
		{
			return player->resources.money;
		}

		void SellPower(const gc_ptr<Player>& player, double amount)
		{
			if (amount > player->resources.power)
				amount = player->resources.power;
			player->resources.money += amount;
			player->resources.power -= amount;
			player->oldResources.money += amount;
			player->oldResources.power -= amount;
		}

		double GetPowerAtDawn(const gc_ptr<Player>& player)
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			double curPower = player->resources.power;
			double curTime = pDimension->GetCurrentHour();
			double baseIncrease = GetIncomeAtNight(player);
			if (curTime >= 6.0 && curTime < 18.0)
			{
				curPower = GetPowerAtDusk(player);
				curTime = 18.0;
			}
			double timeUntilDawn;
			if (curTime < 6.0)
			{
				timeUntilDawn = 6.0 - curTime;
			}
			else
			{
				timeUntilDawn = 6.0 + 24.0 - curTime;
			}
			curPower += baseIncrease * timeUntilDawn * pDimension->GetHourLength();
			return curPower;
		}

		double GetPowerAtDusk(const gc_ptr<Player>& player)
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			double curPower = player->resources.power;
			double curTime = pDimension->GetCurrentHour();
			double baseIncrease = GetIncomeAtNight(player);
			double solarIncrease = GetIncomeAtNoon(player) - baseIncrease;
			if (curTime < 6.0 || curTime >= 18.0)
			{
				curPower = GetPowerAtDawn(player);
				curTime = 6.0;
			}
			double step = 0.01;
			for (double time = curTime; time < 18.0; time += step)
			{
				curPower += (baseIncrease + solarIncrease * (pow(sin((time-6.0)/12*PI), 1.0/3) * 0.8 + 0.2)) * step * pDimension->GetHourLength();
			}
			return curPower;
		}

		void FacePos(gc_ptr<Unit>& unit, Position pos)
		{
			Utilities::Vector3D direction, zero_rot;
			direction.set(pos.x - unit->pos.x, 0.0, pos.y - unit->pos.y);
			direction.normalize();
			zero_rot.set(-1.0, 0.0, 0.0);
			unit->rotation = acos(zero_rot.dot(direction)) * (float) (180 / PI);
			if (direction.z < 0) unit->rotation = 180 - unit->rotation + 180;
		}

		void FaceUnit(gc_ptr<Unit>& unit, const gc_ptr<Unit>& targetUnit)
		{
			FacePos(unit, targetUnit->pos);
		}

		void PerformBuild(gc_ptr<Unit>& unit)
		{
			double build_cost;
			double power_usage = unit->type->buildPowerUsage / AI::aiFps;
			gc_ptr<UnitType> build_type = unit->pMovementData->action.args.unitType;

			if (unit->owner->resources.power < power_usage)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "BUILD WAIT NOPOWER " << AI::currentFrame << ": " << unit->GetHandle() << " " << power_usage << " " << unit->owner->resources.power << "\n";
#endif
				return;
			}

			if (!unit->pMovementData->action.goal.unit)
			{
				if (!build_type->requirements.creation.isSatisfied || !build_type->requirements.existance.isSatisfied)
				{
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "BUILD CANCEL NORESEARCH " << AI::currentFrame << ": " << unit->GetHandle() << "\n";
#endif
					AI::CancelAction(unit);
					return;
				}

				if (!unit->type->isMobile)
				{
					unit->pMovementData->action.goal.unit = CreateUnitNoDisplay(build_type, -1, false);
				}
				else
				{
					gc_ptr<Unit> pUnit = CreateUnit(build_type, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, -1, false);
					if (pUnit)
						pUnit->rotation = unit->pMovementData->action.rotation;
					
					unit->pMovementData->action.goal.unit = pUnit;
				}
				if (!unit->pMovementData->action.goal.unit)
				{
					const gc_ptr<Unit>& cur_unit = pppElements[unit->pMovementData->action.goal.pos.y][unit->pMovementData->action.goal.pos.x];

					if (cur_unit && cur_unit->type == build_type)
					{
						unit->pMovementData->action.goal.unit = cur_unit;
//						cout << "assign" << endl;
					}
					else
					{
						if (SquaresAreWalkable(build_type, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, SIW_IGNORE_OWN_MOBILE_UNITS | SIW_ALLKNOWING))
						{
							if (!unit->owner->isRemote)
							{
								int start_x, start_y;
								GetTypeUpperLeftCorner(build_type, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, start_x, start_y);
								Uint32 curtime = SDL_GetTicks();
								for (int y = start_y; y < start_y + build_type->heightOnMap; y++)
								{
									for (int x = start_x; x < start_x + build_type->widthOnMap; x++)
									{
										if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height && pppElements[y][x] && !pppElements[y][x]->isMoving)
										{
											if (curtime - pppElements[y][x]->lastCommand > AI::aiFps)
											{
												int goto_x, goto_y;
												NearestSquareFromBuildingPlace(pppElements[y][x], build_type, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, goto_x, goto_y);
		//										cout << "command " << pppElements[y][x]->type->GetHandle() << endl;
												CommandUnit(pppElements[y][x], goto_x, goto_y, AI::ACTION_GOTO, unit->pMovementData->action.args, true, true);
											}
										}
									}
								}
							}
//							cout << "wait" << endl;
						}
						else
						{
//							cout << "cancel" << endl;
							AI::CancelAction(unit);
						}
						return;
					}
				}
			}

			gc_ptr<Unit> newUnit = unit->pMovementData->action.goal.unit;
			ObjectRequirements &requirements = newUnit->type->requirements;

			if (newUnit->pMovementData->action.action == AI::ACTION_DIE)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "BUILD CANCEL TARGETDYING " << AI::currentFrame << ": " << unit->GetHandle() << "\n";
#endif
				AI::CancelAction(unit);
				return;
			}

			if (unit->type->isMobile && unit->faceTarget != FACETARGET_TARGET && newUnit->isDisplayed)
			{
				FaceUnit(unit, newUnit);
				unit->faceTarget = FACETARGET_TARGET;
			}

			build_cost = (float) requirements.money / (float) (requirements.time * AI::aiFps);

			if (unit->owner->resources.money < build_cost)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "BUILD WAIT NOMONEY " << AI::currentFrame << ": " << unit->GetHandle() << " " << build_cost << " " << unit->owner->resources.power << "\n";
#endif
				return;
			}

#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "BUILD " << AI::currentFrame << ": " << unit->GetHandle() << " " << newUnit->GetHandle() << " " << build_cost << " " << power_usage << "\n";
#endif

			unit->owner->resources.power -= power_usage;

			unit->owner->resources.money -= build_cost;

			newUnit->completeness += 100.0f / float(requirements.time * AI::aiFps);
			newUnit->health += (float) newUnit->type->maxHealth / float(requirements.time * AI::aiFps);

			if (newUnit->health >= newUnit->type->maxHealth)
			{
				newUnit->health = (float) newUnit->type->maxHealth;
			}

			if (newUnit->completeness >= 100.0)
			{
				newUnit->completeness = 100.0;
				if (newUnit->isCompleted == false)
				{
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "BUILD DONE " << AI::currentFrame << ": " << unit->GetHandle() << " " << newUnit->GetHandle() << "\n";
#endif
					newUnit->isCompleted = true;

 					unit->type->numBuilt++;
 					unit->type->numExisting++;

 					RecheckAllRequirements(newUnit->owner);

					if (newUnit->type->isMobile)
					{
						int new_x = unit->curAssociatedSquare.x, new_y = unit->curAssociatedSquare.y;
						GetNearestUnoccupiedPosition(newUnit->type, new_x, new_y);
						ScheduleDisplayUnit(newUnit, new_x, new_y);
					}
					else
					{
						Complete(newUnit);
					}
				}

				AI::CompleteAction(unit);
				
				if (unit->rallypoint)
				{
					AI::CommandUnit(newUnit, unit->rallypoint->x, unit->rallypoint->y, AI::ACTION_GOTO, ActionArguments(), true, true);
				}
				
			}

		}
		
		void PerformResearch(gc_ptr<Unit>& unit)
		{
			double research_cost;
			const gc_ptr<Research>& research = unit->pMovementData->action.args.research;
			double power_usage = (unit->type->buildPowerUsage + research->requirements.power / research->requirements.time) / AI::aiFps;

			
			if (!research->requirements.creation.isSatisfied || !research->requirements.existance.isSatisfied)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "RESEARCH CANCEL NORESEARCH " << AI::currentFrame << ": " << unit->GetHandle() << "\n";
#endif
				AI::CancelAction(unit);
				return;
			}

			if ((research->researcher && research->researcher != unit) || research->isResearched)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				if (research->researcher && research->researcher != unit)
				{
					Networking::checksum_output << "RESEARCH CANCEL ISBEINGRESEARCHEDBY " << AI::currentFrame << ": " << unit->GetHandle() << " " << research->researcher << "\n";
				}
				else
				{
					Networking::checksum_output << "RESEARCH CANCEL ISRESEARCHED " << AI::currentFrame << ": " << unit->GetHandle() << "\n";
				}
#endif
				AI::CancelAction(unit);
				return;
			}

			research->researcher = unit;

			if (unit->owner->resources.power < power_usage)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "RESEARCH WAIT NOPOWER " << AI::currentFrame << ": " << unit->GetHandle() << " " << power_usage << " " << unit->owner->resources.power << "\n";
#endif
				return;
			}

			research_cost = (float) research->requirements.money / (research->requirements.time * AI::aiFps);

			if (unit->owner->resources.money < research_cost)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "RESEARCH WAIT NOMONEY " << AI::currentFrame << ": " << unit->GetHandle() << " " << research_cost << " " << unit->owner->resources.money << "\n";
#endif
				return;
			}

#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "RESEARCH " << AI::currentFrame << ": " << unit->GetHandle() << " " << research_cost << " " << power_usage << "\n";
#endif

			unit->owner->resources.power -= power_usage;

			unit->owner->resources.money -= research_cost;

			unit->action_completeness += 100.0f / float(research->requirements.time * AI::aiFps);

			if (unit->action_completeness >= 100.0)
			{
				research->researcher = NULL;
				research->isResearched = true;
				AI::CompleteAction(unit);
 				RecheckAllRequirements(unit->owner);

 				if (research->luaEffectObj.length())
 				{
 					lua_State *pVM = unit->owner->aiState->GetState();

					// Make the luawrapper code believe that we're calling this function...
					unit->owner->aiState->SetCurFunction(research->luaEffectObj + ".apply");

					// Get the "apply" function from the user-supplied table
 					lua_getglobal(pVM, research->luaEffectObj.c_str());
 					lua_getfield(pVM, -1, "apply");
 					lua_pushlightuserdata(pVM, (void*) unit->owner->GetHandle());
 					lua_pushlightuserdata(pVM, (void*) unit->type->GetHandle());
 					lua_pushlightuserdata(pVM, (void*) unit->GetHandle());
 					unit->owner->aiState->CallFunction(3);
 				}
			}
			
		}
		
		void CancelBuild(const gc_ptr<Dimension::Unit>& pUnit)
		{	
			if (!pUnit)
				return;

			if (!pUnit->pMovementData)
				return;

			if (!pUnit->pMovementData->action.args.unitType)
				return;

			int cost;
			gc_ptr<UnitType> build_type = pUnit->pMovementData->action.args.unitType;

			if (pUnit->pMovementData->action.goal.unit && pUnit->pMovementData->action.goal.unit->pMovementData->action.action != AI::ACTION_DIE)
			{
				gc_ptr<Unit> target = pUnit->pMovementData->action.goal.unit;
				if (!pUnit->pMovementData->action.goal.unit->isDisplayed)
				{
					int new_x = pUnit->curAssociatedSquare.x, new_y = pUnit->curAssociatedSquare.y;

					cost = build_type->requirements.money;
					pUnit->owner->resources.money += (float) cost * target->completeness / 200;

					// The following is needed because DeleteUnit() expects the unit to be complete and visible.
					GetNearestUnoccupiedPosition(target->type, new_x, new_y);
					ScheduleDisplayUnit(target, new_x, new_y);

					int thread = AI::PausePathfinding(pUnit);

					pUnit->pMovementData->action.goal.unit = NULL; // Zero out target unit before calling DeleteUnit,
				                                        	// to prevent DeleteUnit from calling CancelAction which calls
				                                        	// CancelBuild which calls DeleteUnit which calls CancelAction....
					AI::ResumePathfinding(thread);

					ScheduleUnitDeletion(target);
				}
			}
		}

		void CancelResearch(const gc_ptr<Dimension::Unit>& pUnit)
		{
			int cost;
			gc_ptr<Research> research = pUnit->pMovementData->action.args.research;
			cost = research->requirements.money;
			pUnit->owner->resources.money += (float)cost * pUnit->action_completeness / 200;
		}

		// returns true when the unit can attack at the current time
		bool CanAttack(const gc_ptr<Unit>& attacker)
		{
			if (AI::currentFrame - attacker->lastAttack >= ((float)AI::aiFps / attacker->type->attackSpeed))
			{
				return true;
			}
			return false;
		}

		// perform an attack at target, returning true if the target has been eliminated
		bool Attack(gc_ptr<Unit>& target, float damage)
		{
#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "DAMAGE " << AI::currentFrame << ": " << target->GetHandle() << " " << damage << "\n";
#endif
			if (target->health > 1e-3)
			{
				target->lastAttacked = AI::currentFrame; // only update time of last attack if the unit is not already dead
			}
			target->health -= damage;
			if (target->pMovementData->action.action == AI::ACTION_DIE)
			{
				return true;
			}
			if (target->health <= 1e-3 && target->pMovementData->action.action != AI::ACTION_DIE)
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "DIE" << "\n";
#endif
				KillUnit(target);
				return true;
			}
			return false;
		}

		Uint16 a_seed = 23467;
		float attack_rand()
		{
			a_seed = (Uint16) ( ((int) a_seed + 82473) ^ 53672);
			return ((float)a_seed / 65535.0f);
		}

		// calculate how much damage a unit does
		float CalcUnitDamage(const gc_ptr<Unit>& target, const gc_ptr<Unit>& attacker)
		{
			return ((float) attacker->type->minAttack + float(attacker->type->maxAttack - attacker->type->minAttack) * attack_rand()) / target->type->armor * 100.0;
		}

		void InitiateAttack(gc_ptr<Unit>& attacker, gc_ptr<Unit>& target)
		{
			Position goto_pos;
			Utilities::Vector3D goal_pos;	
			double power_usage =  attacker->type->attackPowerUsage;
			
			if (attacker->owner->resources.power < power_usage)
			{
				return;
			}

			attacker->lastAttack = AI::currentFrame;

			attacker->owner->resources.power -= power_usage;

#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "ATTACK " << AI::currentFrame << ": " << attacker->GetHandle() << " " << target->GetHandle() << " " << power_usage << "\n";
#endif

			if (attacker->type->isMobile)
			{
				FaceUnit(attacker, target);
			}

			if (attack_rand() * 100 > attacker->type->attackAccuracy)
			{
				return;
			}

			if (!attacker->type->projectileType)
			{
				attacker->lastSeenPositions[target->owner->index] = attacker->curAssociatedSquare;

				if (target->pMovementData->action.action != AI::ACTION_DIE)
					AI::SendUnitEventToLua_IsAttacked(target, attacker);

				Attack(target, CalcUnitDamage(target, attacker));
			}
			else
			{
				goto_pos = target->pos;
				goal_pos = Utilities::Vector3D(goto_pos.x, goto_pos.y, Dimension::GetTerrainHeight(goto_pos.x, goto_pos.y));
				goal_pos.z += target->type->height * 0.25f * 0.0625f;
				gc_root_ptr<Projectile>::type proj = CreateProjectile(attacker->type->projectileType, Utilities::Vector3D(attacker->pos.x, attacker->pos.y, GetTerrainHeight(attacker->pos.x, attacker->pos.y)), goal_pos, attacker);
				proj->goalUnit = target;
				attacker->vProjectiles.push_back(proj);
				UnitMainNode::GetInstance()->ScheduleProjectileAddition(proj);
				PlayActionSound(attacker, Audio::SFX_ACT_FIRE_FNF);
			}
		}

		bool UnitBinPred(const gc_ptr<Unit>& unit01, const gc_ptr<Unit>& unit02)
		{
			return unit01->GetHandle() < unit02->GetHandle();
		}

		bool HandleProjectile(const gc_ptr<Projectile>& proj)
		{
			float max_radius = 0;
			vector<gc_ptr<Unit> >::iterator it;
			list<gc_ptr<Unit> > units_hit;
#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "PROJ " << AI::currentFrame << ": " << proj->pos.x << ", " << proj->pos.y << ", " << proj->pos.z << " " << proj->pos.distance(proj->goalPos) << " " << proj->type->speed * (1.0 / AI::aiFps) << "\n";
#endif
/*					if (proj->type->isHoming && proj->goalUnit != NULL)
				proj->goalPos = GetTerrainCoord(proj->goalUnit->pos.x, proj->goalUnit->pos.y);*/

			if (proj->pos.distance(proj->goalPos) < proj->type->speed * (1.0 / AI::aiFps))
			{
				
				max_radius = proj->type->areaOfEffect * 0.125f;
				proj->pos = proj->goalPos;

#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "HIT " << proj->pos.x << ", " << proj->pos.y << ", " << proj->pos.z << "\n";
#endif

				int big_start_x = (int) (proj->pos.x - ceil(max_radius) - 10) >> bigSquareRightShift;
				int big_start_y = (int) (proj->pos.y - ceil(max_radius) - 10) >> bigSquareRightShift;
				int big_end_x = (int) (proj->pos.x + ceil(max_radius) + 10) >> bigSquareRightShift;
				int big_end_y = (int) (proj->pos.y + ceil(max_radius) + 10) >> bigSquareRightShift;
		
				if (big_start_y < 0)
					big_start_y = 0;

				if (big_start_x < 0)
					big_start_x = 0;

				if (big_end_y >= bigSquareHeight)
					big_end_y = bigSquareHeight-1;

				if (big_end_x >= bigSquareWidth)
					big_end_x = bigSquareWidth-1;

				Utilities::Vector3D proj_pos = GetTerrainCoord(proj->goalPos.x, proj->goalPos.y);
				proj_pos.y = proj->goalPos.z;

				units_hit.clear();

				for (int y = big_start_y; y <= big_end_y; y++)
				{
					for (int x = big_start_x; x <= big_end_x; x++)
					{
						for (it = unitsInBigSquares[y][x]->begin(); it != unitsInBigSquares[y][x]->end(); it++)
						{
							const gc_ptr<Unit>& target = *it;
							if (target == proj->attacker)
								continue;

							Utilities::Vector3D unit_pos = GetTerrainCoord(target->pos.x, target->pos.y);
							if (proj_pos.distance(unit_pos) <= max_radius)
							{
								units_hit.push_back(target);
							}
						}
					}
				}

				units_hit.sort(UnitBinPred);

				for (list<gc_ptr<Unit> >::iterator it = units_hit.begin(); it != units_hit.end(); it++)
				{
					gc_ptr<Unit>& target = *it;

#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "HIT " << target->GetHandle() << "\n";
#endif
					if (proj->attacker->isDisplayed)
					{
						// Make the attacked player aware of where its attacker is
						proj->attacker->lastSeenPositions[target->owner->index] = proj->attacker->curAssociatedSquare;
						if (target->owner != proj->attacker->owner)
						{
							if (target->pMovementData->action.action != AI::ACTION_DIE)
								AI::SendUnitEventToLua_IsAttacked(target, proj->attacker);
						}
						Attack(target, CalcUnitDamage(target, proj->attacker));
					}
				}

				if (!Game::Rules::noGraphics)
				{
					FX::pParticleSystems->InitEffect(proj->pos.x, proj->pos.y, 0.0f, max_radius * 4, FX::PARTICLE_SPHERICAL_EXPLOSION);
				}

				UnitMainNode::GetInstance()->ScheduleProjectileDeletion(proj);

				return true;
			}
			else
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "MOVE " << proj->direction.x << ", " << proj->direction.y << ", " << proj->direction.z << "\n";
#endif
				proj->pos += proj->direction * proj->type->speed * (1.0f / (float) AI::aiFps);
			}
			return false;
		}

		void HandleProjectiles(const gc_ptr<Player>& player)
		{
			gc_ptr<Projectile> proj = NULL;

			for (unsigned index = 0; index < player->vProjectiles.size(); )
			{
				proj = player->vProjectiles.at(index);
				if (HandleProjectile(proj))
				{
					player->vProjectiles.erase(player->vProjectiles.begin() + index);
				}
				else
				{
					index++;
				}
			}
		}
		
		void HandleProjectiles(const gc_ptr<Unit>& unit)
		{
			gc_ptr<Projectile> proj = NULL;

			for (unsigned index = 0; index < unit->vProjectiles.size(); )
			{
				proj = unit->vProjectiles.at(index);
				if (HandleProjectile(proj))
				{
					unit->vProjectiles.erase(unit->vProjectiles.begin() + index);
				}
				else
				{
					index++;
				}
			}
		}
		
		void NotEnoughPowerForLight(const gc_ptr<Unit>& unit)
		{
			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (unit->isLighted)
			{
				UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0); // remove
			}
		}

		void EnoughPowerForLight(const gc_ptr<Unit>& unit)
		{
			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (!unit->isLighted)
			{
				UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1); // add
			}
		}

		void ChangePath(const gc_ptr<Unit>& pUnit, int goal_x, int goal_y, AI::UnitAction action, const gc_ptr<Unit>& target, const ActionArguments& args, float rotation)
		{
			if (pUnit->type->isMobile)
			{
				AI::CommandPathfinding(pUnit, pUnit->curAssociatedSquare.x, pUnit->curAssociatedSquare.y, goal_x, goal_y, action, target, args, rotation);
			}
		}

		bool CheckPath(const gc_ptr<Unit>& pUnit)
		{
			AI::Node *curnode = pUnit->pMovementData->pStart, *parent;
			bool invalid_path = false;

			// The code below checks whether the path is correct. It cannot, however, check whether pointers
			// are valid in any other way than checking whether they are NULL or do not match other pointers. Non-null
			// invalid pointers will cause _crashes_, and that is not the fault of this piece of code nor this routine.
			if (curnode->pParent)
			{
				invalid_path = true;
				cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pStart has pParent" << endl;
			}
			else
			{
				parent = curnode;
				curnode = curnode->pChild;
				while (curnode)
				{
					if (curnode->pParent != parent)
					{
						invalid_path = true;
						cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pNode->pChild->pParent != pNode" << endl;
						break;
					}
					parent = curnode;
					curnode = curnode->pChild;
				}
				if (parent->pChild == NULL && parent != pUnit->pMovementData->pGoal)
				{
					invalid_path = true;
					cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pGoal IS NOT THE LAST ITEM IN LINKED LIST OF NODES" << endl;
				}
			}
			return invalid_path;
		}

		int numSentCommands = 0;

/*		bool PushUnits(Unit* pUnit)
		{
			int start_x_new, start_y_new;
			int start_x, start_y;
			int end_x, end_y;
			bool noloop = true;
			if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_MOVING))
			{
				if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_OWN_MOBILE_UNITS))
				{
					return true;
				}
			}
			else
			{
				return true;
			}
			GetUnitUpperLeftCorner(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, start_x_new, start_y_new);
			GetUnitUpperLeftCorner(pUnit, pUnit->curAssociatedSquare.x, pUnit->curAssociatedSquare.y, start_x, start_y);
			end_x = start_x + pUnit->type->widthOnMap - 1;
			end_y = start_y + pUnit->type->heightOnMap - 1;
			Uint32 curtime = SDL_GetTicks();
			bool found = false;
			for (int y = start_y_new; y < start_y_new + pUnit->type->heightOnMap; y++)
			{
				for (int x = start_x_new; x < start_x_new + pUnit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height && pppElements[y][x])
					{
						Dimension::Unit* curUnit = pppElements[y][x];
						pUnit->isWaiting = true;
						found = true;
						if (curtime - curUnit->lastCommand > (AI::aiFps >> 2) && !curUnit->isMoving && !curUnit->isPushed && !curUnit->isWaiting && !AI::IsUndergoingPathCalc(curUnit))
						{
							int diff_x = pUnit->pMovementData->pCurGoalNode->x - pUnit->curAssociatedSquare.x;
							int diff_y = pUnit->pMovementData->pCurGoalNode->y - pUnit->curAssociatedSquare.y;
							int goto_x = pUnit->pMovementData->pCurGoalNode->x, goto_y = pUnit->pMovementData->pCurGoalNode->y;
							int unit_x = curUnit->curAssociatedSquare.x, unit_y = curUnit->curAssociatedSquare.y;
							int start_x_path = start_x - ((curUnit->type->widthOnMap-1) << 1) - 1;
							int start_y_path = start_y - ((curUnit->type->heightOnMap-1) << 1) - 1;
							int end_x_path = end_x + (curUnit->type->widthOnMap << 1) + 1;
							int end_y_path = end_y + (curUnit->type->heightOnMap << 1) + 1;
							int middle_x, middle_y;
							bool dirs_possible[10] = {false, false, true, true, true, true, true, true, true, true};
							int trace_dirs[8][2] = {{-1,  0},
		                       					       {-1, -1},
		                       					       { 0, -1},
		                       					       { 1, -1},
		                       					       { 1,  0},
		                       					       { 1,  1},
		                       					       { 0,  1},
		                       					       {-1,  1}};
		                       			int dirmatrix[3][3] = {{1, 2, 3},
		                       			                       {0, 0, 4},
		                       			                       {7, 6, 5}};
		                       			int curdir = dirmatrix[diff_y+1][diff_x+1];

							int dirs[10][2];
							
							int dir = 2;
							int combo_order[] = {1, 4, 3, 2};
							for (int i = 0; i < 4; i++)
							{
								dirs[dir][0] = trace_dirs[(curdir+combo_order[i])&7][0];
								dirs[dir][1] = trace_dirs[(curdir+combo_order[i])&7][1];
								dir++;
								dirs[dir][0] = trace_dirs[(curdir-combo_order[i])&7][0];
								dirs[dir][1] = trace_dirs[(curdir-combo_order[i])&7][1];
								dir++;
							}
							dirs[9][0] = trace_dirs[(curdir)&7][0];
							dirs[9][1] = trace_dirs[(curdir)&7][1];
							if (diff_x > 0)
							{
								if (diff_y > 0)
								{
									middle_x = end_x_path;
									middle_y = end_y_path;
								}
								else if (diff_y < 0)
								{
									middle_x = end_x_path;
									middle_y = start_y_path;
								}
								else
								{
									middle_x = end_x_path;
									middle_y = goto_y;
								}
							}
							else if (diff_x < 0)
							{
								if (diff_y > 0)
								{
									middle_x = start_x_path;
									middle_y = end_y_path;
								}
								else if (diff_y < 0)
								{
									middle_x = start_x_path;
									middle_y = start_y_path;
								}
								else
								{
									middle_x = start_x_path;
									middle_y = goto_y;
								}
							}
							else
							{
								if (diff_y > 0)
								{
									middle_x = goto_x;
									middle_y = end_y_path;
								}
								else if (diff_y < 0)
								{
									middle_x = goto_x;
									middle_y = start_y_path;
								}
								else
								{
									middle_x = goto_x;
									middle_y = goto_y;
								}
							}
							int flags[] = {SIW_IGNORE_OWN_MOBILE_UNITS | SIW_CONSIDER_WAITING | SIW_CONSIDER_PUSHED};
							bool done = false;
							for (int j = 0; j < 1; j++)
							{
								int cur_x = unit_x, cur_y = unit_y;
								int last_x = unit_x, last_y = unit_y;
								while (1)
								{
									cur_x += middle_x > cur_x ? -1 : 1;
									cur_y += middle_y > cur_y  ? -1 : 1;
									if (!SquaresAreWalkable(curUnit, cur_x, cur_y, flags[j]))
									{
										dirs_possible[0] = false;
										break;
									}
									if (cur_y == start_y || cur_y == end_y)
									{
										if (cur_x == start_x || cur_x == end_x)
										{
											dirs[0][0] = cur_x - curUnit->curAssociatedSquare.x;
											dirs[0][1] = cur_y - curUnit->curAssociatedSquare.y;
											dirs_possible[0] = true;
											break;
										}
									}
									last_x = cur_x;
									last_y = cur_y;
								}

								cur_x = unit_x, cur_y = unit_y;
								last_x = unit_x, last_y = unit_y;
								while (1)
								{
									cur_x += middle_x > cur_x ? 1 : middle_x < cur_x ? -1 : 0;
									cur_y += middle_y > cur_y ? 1 : middle_y < cur_y ? -1 : 0;
									if (!SquaresAreWalkable(curUnit, cur_x, cur_y, flags[j]))
									{
										dirs_possible[1] = false;
										break;
									}
									if (cur_y == middle_y && cur_x == middle_x)
									{
										if (diff_x & diff_y)
										{
											if (last_x - cur_x)
											{
												cur_y += cur_y == end_y ? -1 : 1;
											}
											else if (last_y - cur_y)
											{
												cur_x += cur_x == end_x ? -1 : 1;
											}
										}
										else
										{
											cur_x += (cur_x - last_x);
											cur_y += (cur_y - last_y);
										}
										break;
									}
									last_x = cur_x;
									last_y = cur_y;
								}
								while (1)
								{
									if (!SquaresAreWalkable(curUnit, cur_x, cur_y, flags[j]))
									{
										dirs_possible[1] = false;
										break;
									}
									last_x = cur_x;
									last_y = cur_y;
									cur_x += middle_x > cur_x ? -1 : 1;
									cur_y += middle_y > cur_y  ? -1 : 1;
									if (cur_y == start_y || cur_y == end_y)
									{
										if (cur_x == start_x || cur_x == end_x)
										{
											dirs[1][0] = last_x - curUnit->curAssociatedSquare.x;
											dirs[1][1] = last_y - curUnit->curAssociatedSquare.y;
											dirs_possible[1] = true;
											break;
										}
									}
								}
								for (int i = 0; i < 10; i++)
								{
									int goto_new_x, goto_new_y;
									goto_new_x = curUnit->curAssociatedSquare.x + dirs[i][0];
									goto_new_y = curUnit->curAssociatedSquare.y + dirs[i][1];
									if (dirs_possible[i] && SquaresAreWalkable(curUnit, goto_new_x, goto_new_y, flags[j]))
									{
										if (DoesNotBlock(curUnit, pUnit->type, goto_x, goto_y, goto_new_x, goto_new_y) && (!pUnit->pusher || DoesNotBlock(curUnit, pUnit->pusher->type, pUnit->pusher->pMovementData->pCurGoalNode->x, pUnit->pusher->pMovementData->pCurGoalNode->y, goto_new_x, goto_new_y)))
										{
											cout << "Move " << curUnit << " " << goto_new_x << " " << goto_new_y << " " << flags[j] << " " << i << endl;
											numSentCommands++;
											CommandUnit(curUnit, goto_new_x, goto_new_y, AI::ACTION_GOTO, ActionArguments(), true, true);
											curUnit->isPushed = true;
											if (pUnit->pushID)
											{
												curUnit->pushID = pUnit->pushID;
											}
											else
											{
												curUnit->pushID = nextPushID++;
											}
											if (pUnit->pusher)
											{
												curUnit->pusher = pUnit->pusher;
											}
											else
											{
												curUnit->pusher = pUnit;
											}
											done = true;
											break;
										}
									}
								}
								if (done)
									break;
							}
							if (!done)
							{
								AI::CancelAction(pUnit);
								pUnit->isPushed = false;
								pUnit->isWaiting = false;
							}
						}
					}
				}
			}
			if (!found)
			{
				cout << "Non-found!" << endl;
			}
			return noloop;
		}*/

		void NewGoalNode(const gc_ptr<Unit>& pUnit)
		{
			float distance, distance_per_frame;
			Position goto_pos;
			Utilities::Vector3D move; // abused to calculate movement per axis in 2d and the rotation of the model when going in a specific direction...
			goto_pos.x = (float) pUnit->pMovementData->pCurGoalNode->x + 0.5f;
			goto_pos.y = (float) pUnit->pMovementData->pCurGoalNode->y + 0.5f;

			distance_per_frame = pUnit->type->movementSpeed / (float) AI::aiFps / 
				             ((float)GetTraversalTime(pUnit,
						               pUnit->pMovementData->pCurGoalNode->pParent->x,
							       pUnit->pMovementData->pCurGoalNode->pParent->y,
							       pUnit->pMovementData->pCurGoalNode->x - pUnit->pMovementData->pCurGoalNode->pParent->x,
							       pUnit->pMovementData->pCurGoalNode->y - pUnit->pMovementData->pCurGoalNode->pParent->y)
				              / 10.0f);

			distance = Distance2D(goto_pos.x - pUnit->pos.x, goto_pos.y - pUnit->pos.y);

			move.set(goto_pos.x - pUnit->pos.x, 0.0, goto_pos.y - pUnit->pos.y);
			move.normalize();
			move *= distance_per_frame;

			pUnit->pMovementData->movementVector = move;
			pUnit->pMovementData->distanceLeft = distance;
			pUnit->pMovementData->distancePerFrame = distance_per_frame;
			pUnit->faceTarget = FACETARGET_NONE;
		}

		bool MoveUnit(const gc_ptr<Unit>& pUnit)
		{
			float distance, distance_per_frame;
			Position goto_pos;
			Utilities::Vector3D move; // abused to calculate movement per axis in 2d and the rotation of the model when going in a specific direction...
			AI::UnitAction action = pUnit->pMovementData->action.action;
			Utilities::Vector3D zero_rot;
			Utilities::Vector3D goal_pos;
			bool should_move = true;

			double power_usage =  pUnit->type->movePowerUsage / AI::aiFps;

			if (pUnit->owner->resources.power < power_usage)
			{
				return true;
			}

			if (pUnit->pMovementData->pStart)
			{
				if ((action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK) && !pUnit->owner->isRemote)
				{
					if (pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x != pUnit->pMovementData->action.goal.pos.x ||
					    pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y != pUnit->pMovementData->action.goal.pos.y)
					{
						ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y, pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args, pUnit->pMovementData->action.rotation);
					}
				}
			
				if (!pUnit->pMovementData->pCurGoalNode)
				{
					AI::Node *curnode = pUnit->pMovementData->pStart;
					bool recalc_path = CheckPath(pUnit);
					pUnit->isWaiting = false;
					
					if (recalc_path)
					{
						cout << "TRYING TO RECOVER BY DEALLOCATING PATH AND CALCULATING IT AGAIN FOR UNIT " << pUnit << "..." << endl;
					}
					else
					{
						while (curnode)
						{
							if (curnode->x == pUnit->curAssociatedSquare.x &&
							    curnode->y == pUnit->curAssociatedSquare.y)
							{
#ifdef CHECKSUM_DEBUG_HIGH
								Networking::checksum_output << "START PATH GOAL " << AI::currentFrame << ": " << pUnit->GetHandle() << " " << curnode->x << " " << curnode->y << "\n";
#endif
								break;
							}
							curnode = curnode->pChild;
						}

						if (!curnode)
						{
							recalc_path = true;
							pUnit->pMovementData->pCurGoalNode = NULL;
						}
						else
						{
							pUnit->pMovementData->pCurGoalNode = curnode->pChild;
						}
					}

					if (recalc_path)
					{
						if (!pUnit->owner->isRemote)
						{
#ifdef CHECKSUM_DEBUG_HIGH
							Networking::checksum_output << "INIT RECALC " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
							// unit positions has changed enough since the path was calculated, making it needed to
							// recalculate it. another possibility is that the path was invalidated by CheckPath()...
							if (action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK)
							{
								ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, 
										  pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y,
										  pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args,
										  pUnit->pMovementData->action.rotation);
							}
							else
							{
								ChangePath(pUnit, pUnit->pMovementData->action.goal.pos.x, pUnit->pMovementData->action.goal.pos.y,
										  pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args,
										  pUnit->pMovementData->action.rotation);
							}
						}
						pUnit->pMovementData->pCurGoalNode = NULL;
						should_move = false;
						pUnit->isMoving = false;
					}
					else
					{
						pUnit->pMovementData->switchedSquare = false;
						if (!pUnit->pMovementData->pCurGoalNode || !pUnit->pMovementData->pCurGoalNode->pParent)
						{
#ifdef CHECKSUM_DEBUG_HIGH
							Networking::checksum_output << "ONE NODE PATH " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
							if (pUnit->pMovementData->action.goal.unit)
							{
								if (!CanReach(pUnit, pUnit->pMovementData->action.goal.unit))
								{
#ifdef CHECKSUM_DEBUG_HIGH
									Networking::checksum_output << "CANCEL CANNOTREACH " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
									AI::CancelAction(pUnit);
								}
							}
							else
							{
								if (pUnit->pMovementData->action.action != AI::ACTION_BUILD)
								{
#ifdef CHECKSUM_DEBUG_HIGH
									Networking::checksum_output << "COMPLETE " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
									AI::CompleteAction(pUnit);
								}
								else
								{
									if (!IsWithinRangeForBuilding(pUnit))
									{
#ifdef CHECKSUM_DEBUG_HIGH
										Networking::checksum_output << "CANCEL CANNOTBUILD " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
										AI::CancelAction(pUnit);
									}
								}
							}
							pUnit->isMoving = false;
							pUnit->isPushed = false;
							pUnit->pMovementData->pCurGoalNode = NULL;
							should_move = false;
							AI::DeallocPathfindingNodes(pUnit);
						}
						else
						{
							NewGoalNode(pUnit);
/*							PushUnits(pUnit);
							if (!pUnit->pMovementData->pStart)
							{
								return true;
							}*/
						}
					}
				}
			}
			else
			{
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "PATHWAIT " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
				should_move = false;
				pUnit->isMoving = false;
				if (!pUnit->owner->isRemote && !AI::IsUndergoingPathCalc(pUnit))
				{
					ChangePath(pUnit, pUnit->pMovementData->action.goal.pos.x, pUnit->pMovementData->action.goal.pos.y, pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args, pUnit->pMovementData->action.rotation);
				}
			}

			if (should_move)
			{
				move = pUnit->pMovementData->movementVector;
				distance_per_frame = pUnit->pMovementData->distancePerFrame;
				distance = pUnit->pMovementData->distanceLeft;
				
#ifdef CHECKSUM_DEBUG_HIGH
				Networking::checksum_output << "MOVE " << AI::currentFrame << ": " << pUnit->GetHandle() << " " << pUnit->pos.x << " " << pUnit->pos.y << " " << move.x << " " << move.y << " " << pUnit->type->movementSpeed << " " << AI::aiFps << " " << GetTraversalTime(pUnit, pUnit->pMovementData->pCurGoalNode->pParent->x, pUnit->pMovementData->pCurGoalNode->pParent->y, pUnit->pMovementData->pCurGoalNode->x - pUnit->pMovementData->pCurGoalNode->pParent->x, pUnit->pMovementData->pCurGoalNode->y - pUnit->pMovementData->pCurGoalNode->pParent->y) << " " << distance_per_frame << " " << distance << " " << power_usage << "\n";
#endif
				
				if (distance < distance_per_frame)
				{
					move *= (distance / distance_per_frame);
				}

				if (!pUnit->pMovementData->switchedSquare &&
				    Distance2D(pUnit->pos.x + move.x - (float) pUnit->pMovementData->pCurGoalNode->pParent->x - 0.5f,
					       pUnit->pos.y + move.y - (float) pUnit->pMovementData->pCurGoalNode->pParent->y - 0.5f) > 
				    Distance2D(pUnit->pos.x + move.x - (float) pUnit->pMovementData->pCurGoalNode->x - 0.5f,
					       pUnit->pos.y + move.y - (float) pUnit->pMovementData->pCurGoalNode->y - 0.5f))
				{
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "ATTEMPT " << AI::currentFrame << ": " << pUnit->GetHandle() << " " << pUnit->pMovementData->pCurGoalNode->x << " " << pUnit->pMovementData->pCurGoalNode->y << "\n";
#endif
					if (!UpdateAssociatedSquares(pUnit, pUnit->pMovementData->pCurGoalNode->x,
								            pUnit->pMovementData->pCurGoalNode->y,
								            pUnit->pMovementData->pCurGoalNode->pParent->x,
								            pUnit->pMovementData->pCurGoalNode->pParent->y))
					{
						should_move = false;
						pUnit->isMoving = false;
/*						pUnit->pushID = 0;
						pUnit->pusher = NULL;*/
						if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_MOVING | SIW_ALLKNOWING))
						{
							bool recalc = true;
							
/*							if (pUnit->isWaiting)
							{ 
								if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_OWN_MOBILE_UNITS))
								{
									recalc = true;
								}
							}
							else
							{
								if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_OWN_MOBILE_UNITS | SIW_CONSIDER_WAITING))
								{
									recalc = true;
								}
							}*/
							if (recalc)
							{
#ifdef CHECKSUM_DEBUG_HIGH
								Networking::checksum_output << "RECALC " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
								if (!pUnit->owner->isRemote && !AI::IsUndergoingPathCalc(pUnit))
								{
									if (action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK)
									{
										ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, 
											  	  pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y,
										  	  pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args,
										  pUnit->pMovementData->action.rotation);
									}
									else
									{
										ChangePath(pUnit, pUnit->pMovementData->action.goal.pos.x, pUnit->pMovementData->action.goal.pos.y,
										  	  pUnit->pMovementData->action.action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.args,
										  pUnit->pMovementData->action.rotation);
									}
								}
							}
							else
							{
//								PushUnits(pUnit);
//								return true;
							}
						}
						else
						{
							pUnit->isWaiting = true;
#ifdef CHECKSUM_DEBUG_HIGH
							Networking::checksum_output << "STOP " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
						}
					}
					else
					{
						pUnit->isWaiting = false;
						pUnit->pMovementData->switchedSquare = true;
						pUnit->isMoving = true;
					}

				}

				if (should_move)
				{
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "REALLYMOVE " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
					pUnit->owner->resources.power -= power_usage;
			
					pUnit->pos.x += move.x;
					pUnit->pos.y += move.z;
					pUnit->isMoving = true;

					if (pUnit->faceTarget != FACETARGET_PATH)
					{
						zero_rot.set(-1.0, 0.0, 0.0);
						move.normalize();
						pUnit->rotation = acos(zero_rot.dot(move)) * (float) (180 / PI);
						if (move.z < 0) pUnit->rotation = 180 - pUnit->rotation + 180;
						pUnit->faceTarget = FACETARGET_PATH;
					}

					if (distance < distance_per_frame)
					{
						pUnit->pMovementData->distanceLeft = 0;
						if (pUnit->pMovementData->pCurGoalNode->x == pUnit->pMovementData->pGoal->x &&
						    pUnit->pMovementData->pCurGoalNode->y == pUnit->pMovementData->pGoal->y)
						{
							pUnit->isWaiting = false;
							pUnit->isPushed = false;
/*							pUnit->pushID = 0;
							pUnit->pusher = NULL;*/
#ifdef CHECKSUM_DEBUG_HIGH
							Networking::checksum_output << "REACH " << AI::currentFrame << ": " << pUnit->GetHandle() << " " << pUnit->pMovementData->pGoal->x << " " << pUnit->pMovementData->pGoal->y << "\n";
#endif
							if (pUnit->pMovementData->action.goal.unit)
							{
								if (!CanSee(pUnit, pUnit->pMovementData->action.goal.unit))
								{
#ifdef CHECKSUM_DEBUG_HIGH
									Networking::checksum_output << "CANCEL CANNOTSEE " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
									AI::CancelAction(pUnit);
								}
							}
							else
							{
								if (pUnit->pMovementData->action.action != AI::ACTION_BUILD)
								{
#ifdef CHECKSUM_DEBUG_HIGH
									Networking::checksum_output << "COMPLETE " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
									AI::CompleteAction(pUnit);
								}
								else
								{
									if (!IsWithinRangeForBuilding(pUnit))
									{
#ifdef CHECKSUM_DEBUG_HIGH
										Networking::checksum_output << "CANCEL CANNOTBUILD " << AI::currentFrame << ": " << pUnit->GetHandle() << "\n";
#endif
										AI::CancelAction(pUnit);
									}
								}
							}
							pUnit->isMoving = false;
							pUnit->pMovementData->pCurGoalNode = NULL;
							AI::DeallocPathfindingNodes(pUnit);
						}
						else
						{
							if (pUnit->pMovementData->pCurGoalNode->pChild && pUnit->pMovementData->pCurGoalNode->pChild->pParent != pUnit->pMovementData->pCurGoalNode)
							{
								cout << "ERROR: pCurGoalNode != pCurGoalNode->pChild->pParent" << endl;
								pUnit->pMovementData->pCurGoalNode = NULL;
							}
							else
							{
								pUnit->pMovementData->pCurGoalNode = pUnit->pMovementData->pCurGoalNode->pChild;
								NewGoalNode(pUnit);
//								PushUnits(pUnit);
							}
#ifdef CHECKSUM_DEBUG_HIGH
							Networking::checksum_output << "NEXT GOAL " << AI::currentFrame << ": " << pUnit->GetHandle() << " " << pUnit->pMovementData->pCurGoalNode->x << " " << pUnit->pMovementData->pCurGoalNode->y << "\n";
#endif
							pUnit->pMovementData->switchedSquare = false;
						}
					}
					else
					{
						pUnit->pMovementData->distanceLeft -= distance_per_frame;
					}
				
				}
			}

			return true;
		}

		void DeselectUnit(const gc_ptr<Unit>& unit)
		{
			for (vector<gc_ptr<Unit> >::iterator it = unitsSelected.begin(); it != unitsSelected.end(); it++)
			{
				if (unit == *it)
				{
					unitsSelected.erase(it);
					UnitMainNode::GetInstance()->ScheduleDeselection(unit);
					return;
				}
			}
		}

		void DeselectAllUnits()
		{
			while (unitsSelected.size())
			{
				DeselectUnit(unitsSelected[0]);
			}
		}

		void SelectUnit(const gc_ptr<Unit>& unit)
		{
			if (unit->owner == currentPlayerView)
			{
				for (unsigned i = 0; i < unitsSelected.size(); i++)
				{
					if (unitsSelected[i]->owner != currentPlayerView)
					{
						DeselectUnit(unitsSelected[i]);
						i--;
						continue;
					}
					if (unit == unitsSelected[i])
					{
						return;
					}
				}
			}
			else
			{
				for (vector<gc_ptr<Unit> >::iterator it = unitsSelected.begin(); it != unitsSelected.end(); it++)
				{
					if (unit == *it || (*it)->owner == currentPlayerView)
					{
						return;
					}
				}
			}
			unitsSelected.push_back(unit);
			UnitMainNode::GetInstance()->ScheduleSelection(unit);
		}

		const std::vector<gc_ptr<Unit> >& GetSelectedUnits()
		{
			return unitsSelected;
		}

		Uint16 r_seed = 23467;
		float rotation_rand()
		{
			r_seed = Uint16(((int) r_seed + 82473) ^ 53672);
			return ((float)r_seed / 65535.0f);
		}

		void PrepareUnitEssentials(gc_ptr<Unit>& unit, const gc_ptr<UnitType>& type)
		{
			assert(unit);
			assert(type);

			unit->type = type;
			unit->power = (float) type->maxPower;
			unit->owner = type->player;
			unit->rotation = Utilities::RandomDegree();
			unit->lastAttack = 0;
			unit->lastAttacked = 0;
			unit->lastCommand = 0;
			unit->isDisplayed = false;
			unit->lightState = LIGHT_ON;
			unit->isLighted = false;
			unit->isMoving = false;
			unit->isWaiting = false;
			unit->isPushed = false;
			unit->hasSeen = false;
			unit->usedInAreaMaps = false;
/*			unit->pushID = 0;
			unit->pusher = NULL;*/
			unit->faceTarget = FACETARGET_NONE;
			unit->action_completeness = 0.0f;
			unit->hasPower = false;
			unit->curAssociatedSquare.x = -1;
			unit->curAssociatedSquare.y = -1;
			unit->curAssociatedBigSquare.x = -1;
			unit->curAssociatedBigSquare.y = -1;
			unit->rallypoint = NULL;
			unit->aiFrame = 0;

			unit->pMovementData = new AI::MovementData;
			AI::InitMovementData(unit);
		}

		set<gc_ptr<Unit> > unitsScheduledForDeletion;
		list<gc_ptr<Unit> > unitsScheduledForDisplay;

		SDL_mutex* unitCreationMutex = NULL;

		// create a unit, but don't display it
		gc_ptr<Unit> CreateUnitNoDisplay(const gc_ptr<UnitType>& type, int id, bool complete)
		{
			SDL_LockMutex(unitCreationMutex);
			if (pWorld->vUnits.size() >= 0xFFFF)
			{
				return NULL;
			}

			gc_ptr<Unit> unit = new Unit;
			unit->AssignHandle(id);

			PrepareUnitEssentials(unit, type);
//			PrepareAnimationData(unit);

			unit->unitAIFuncs = type->unitAIFuncs;

			if (!complete)
			{
				unit->completeness = 0.0;
				unit->isCompleted = false;
				unit->health = 0.0;
			}
			else
			{
				unit->completeness = 100.0;
				unit->isCompleted = true;
				unit->health = (float) type->maxHealth;
			}
			
#ifdef CHECKSUM_DEBUG_HIGH
			Networking::checksum_output << "CREATEUNIT " << type->id << "\n";
#endif
			
			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				unit->lastSeenPositions.push_back(IntPosition());
				unit->lastSeenPositions[i].x = -1000;
				unit->lastSeenPositions[i].y = -1000;
			}

			CheckPrecomputedArrays(type);
			
			SDL_UnlockMutex(unitCreationMutex);

//			std::cout << "Create " << unit->GetHandle() << std::endl;

			return unit;
		}
		
		SDL_mutex* unitsScheduledForDisplayMutex = NULL;

		bool ScheduleDisplayUnit(const gc_ptr<Unit>& unit, int x, int y)
		{
			if (!SquaresAreWalkable(gc_ptr<UnitType>(unit->type), x, y, SIW_ALLKNOWING))
			{
				return false;
			}
		
			unit->curAssociatedSquare.x = x;
			unit->curAssociatedSquare.y = y;

			if (unit->type->isMobile)
				numUnitsPerAreaMap[unit->type->heightOnMap-1][unit->type->movementType]++;
			else
				AI::AddUnitToAreaMap(unit);

			SDL_LockMutex(unitsScheduledForDisplayMutex);
			unitsScheduledForDisplay.push_back(unit);
			SDL_UnlockMutex(unitsScheduledForDisplayMutex);

			return true;
		}

		bool DisplayUnit(const gc_ptr<Unit>& unit)
		{
			if (!SquaresAreWalkable(gc_ptr<UnitType>(unit->type), unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, SIW_ALLKNOWING))
			{
				return false;
			}
		
//			std::cout << "display " << unit->GetHandle() << " (" << unit << ")" << std::endl;

			pWorld->vUnits.push_back(unit);
			if (unit->type->hasAI)
			{
				pWorld->vUnitsWithAI.push_back(unit);
			}
			unit->owner->vUnits.push_back(unit);
			if (unit->type->hasLuaAI)
			{
				unit->owner->vUnitsWithLuaAI.push_back(unit);
			}

			unit->pos.x = (float) unit->curAssociatedSquare.x + 0.5;
			unit->pos.y = (float) unit->curAssociatedSquare.y + 0.5;

			unit->isDisplayed = true;

			SetAssociatedSquares(unit, (int) unit->pos.x, (int) unit->pos.y);

			if (!unit->isCompleted)
			{
				Incomplete(unit);
			}
			else
			{
 				unit->type->numBuilt++;
 				unit->type->numExisting++;
 				RecheckAllRequirements(unit->owner);
			}

 			displayedUnitPointers.set(unit, true);

			AI::SendUnitEventToLua_UnitCreation(unit);
			AI::SendUnitEventToLua_BecomeIdle(unit);

			UnitMainNode::GetInstance()->ScheduleUnitNodeAddition(unit);

			return true;
		}

		// create a unit
		gc_ptr<Unit> CreateUnit(const gc_ptr<UnitType>& type, int x, int y, int id, bool complete)
		{
			if (!SquaresAreWalkable(type, x, y, SIW_ALLKNOWING))
			{
//				cout << "buildfail" << endl;
				return NULL;
			}
			gc_ptr<Unit> unit = CreateUnitNoDisplay(type, id, complete);
			ScheduleDisplayUnit(unit, x, y);
			return unit;
		}
		
		void DisplayScheduledUnits()
		{
			unitsScheduledForDisplay.sort(UnitBinPred);
			for (list<gc_ptr<Unit> >::iterator it = unitsScheduledForDisplay.begin(); it != unitsScheduledForDisplay.end(); it++)
			{
				DisplayUnit(*it);
			}
			unitsScheduledForDisplay.clear();
		}

		void KillUnit(gc_ptr<Unit> unit)
		{
//			unsigned int i;
			
//			std::cout << "Kill " << unit->GetHandle() << std::endl;

			AI::CancelAllActions(unit);

			AI::SendUnitEventToLua_UnitKilled(unit);
			
			unit->health = 0;
			unit->pMovementData->action.action = AI::ACTION_DIE;

			PlayActionSound(unit, Audio::SFX_ACT_DEATH_FNF);

			if (!Game::Rules::noGraphics)
			{
				//Start an explosion
				if(UnitIsVisible(unit, Dimension::currentPlayerView))
				{
					FX::pParticleSystems->InitEffect(unit->pos.x, unit->pos.y, 0.0f, unit->type->size, FX::PARTICLE_SPHERICAL_EXPLOSION);
				}
			}

		}

		void RemoveUnitFromLists(gc_ptr<Unit> unit)
		{
			unsigned int i, j;
			
//			std::cout << "remove " << unit->GetHandle() << " (" << unit << ")" << std::endl;

			DeleteAssociatedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y);

			if (unit->isCompleted)
			{
				unit->type->numExisting--;
 				RecheckAllRequirements(unit->owner);
			}

 			displayedUnitPointers.remove(unit);
 			unit->isDisplayed = false;

//			std::cout << "Delete " << unit->GetHandle() << std::endl;

			for (vector<gc_ptr<Unit> >::iterator it = pWorld->vUnits.begin(); it != pWorld->vUnits.end(); it++)
			{
				if (*it == unit)
				{
					pWorld->vUnits.erase(it);
					break;
				}
			}

			if (unit->type->hasAI)
			{
				for (vector<gc_ptr<Unit> >::iterator it = pWorld->vUnitsWithAI.begin(); it != pWorld->vUnitsWithAI.end(); it++)
				{
					if (*it == unit)
					{
						pWorld->vUnitsWithAI.erase(it);
						break;
					}
				}
			}
			
			if (unit->owner == GetCurrentPlayer())
			{
				vector<gc_ptr<Unit> >::iterator it = unitsDisplayQueue.begin();
				while (it != unitsDisplayQueue.end())
				{
					if (*it == unit)
					{
						unitsDisplayQueue.erase(it);
						break;
					}
					it++;
				}
			}

			for (vector<gc_ptr<Unit> >::iterator it = unit->owner->vUnits.begin(); it != unit->owner->vUnits.end(); it++)
			{
				if (*it == unit)
				{
					unit->owner->vUnits.erase(it);
					break;
				}
			}
			if (unit->type->hasLuaAI)
			{
				for (vector<gc_ptr<Unit> >::iterator it = unit->owner->vUnitsWithLuaAI.begin(); it != unit->owner->vUnitsWithLuaAI.end(); it++)
				{
					if (*it == unit)
					{
						unit->owner->vUnitsWithLuaAI.erase(it);
						break;
					}
				}
			}
			
			for (vector<gc_ptr<Projectile> >::iterator it = unit->vProjectiles.begin(); it != unit->vProjectiles.end(); it++)
			{
				unit->owner->vProjectiles.push_back(*it);
			}

			unit->vProjectiles.clear();

			if (unitsScheduledForDeletion.find(unit) != unitsScheduledForDeletion.end())
				unitsScheduledForDeletion.erase(unit);

			for (j = 0; j < 10; j++)
			{
				for (i = 0; i < unitGroups[j].size(); i++)
				{
					if (unitGroups[j].at(i) == unit)
					{
						unitGroups[j].erase(unitGroups[j].begin() + i);
						break;
					}
				}
			}

			DeselectUnit(unit);

			RemoveUnitFromBigSquare(unit);
			
			if (unit->usedInAreaMaps)
			{
				AI::DeleteUnitFromAreaMap(unit);
			}
			
			for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
			{
				if (unit->soundNodes[i].active)
				{
					StopRepeatingActionSound(unit, (Audio::SoundNodeAction)i);
				}
			}

			if (unit->type->isMobile)
				numUnitsPerAreaMap[unit->type->heightOnMap-1][unit->type->movementType]--;

			AI::PausePathfinding();

			for (i = 0; i < pWorld->vUnits.size(); i++)
			{
				const gc_ptr<Unit>& curUnit = pWorld->vUnits.at(i);
				while (curUnit->pMovementData->action.goal.unit == unit)
				{
					AI::CancelAction(curUnit);
				}
				if (curUnit->pMovementData->_action.goal.unit == unit)
				{
					AI::CancelUndergoingProc(curUnit);
				}
				if (curUnit->pMovementData->_newAction.goal.unit == unit)
				{
					AI::DequeueNewPath(curUnit);
				}
			}

			UnitMainNode::GetInstance()->ScheduleUnitNodeDeletion(unit);

			if (AI::IsUndergoingPathCalc(unit))
			{
				AI::QuitUndergoingProc(unit);
				AI::ResumePathfinding();
				return;
			}
			
			AI::ResumePathfinding();

			unit->RevokeHandle();

		}

		Unit::~Unit()
		{

		}

		vector<gc_ptr<Unit> > unitsDisplayQueue;
		gc_ptr<Unit> CreateGhostUnit(const gc_ptr<UnitType>& type)
		{
			gc_ptr<Player> owner = Game::Dimension::currentPlayer;

			if (!owner)
				return NULL;

			gc_ptr<Unit> unit = new Unit;
			unit->AssignHandle();

			unit->completeness = 100.0;
			unit->isCompleted = true;
			unit->health = (float) type->maxHealth;

			PrepareUnitEssentials(unit, type);
//			PrepareAnimationData(unit);

			return unit;
		}

		void AppendToActionDisplayQueueIfOK(const gc_ptr<Unit>& unit)
		{
			if (!unit->actionQueue.size())
				return;
		
			unitsDisplayQueue.push_back(unit);
		}

		void ScheduleUnitDeletion(const gc_ptr<Unit>& unit)
		{
			if (unit->pMovementData->action.action != AI::ACTION_DIE)
			{
				AI::CancelAllActions(unit);

				AI::SendUnitEventToLua_UnitKilled(unit);
			}
			unitsScheduledForDeletion.insert(unit);
		}

		void DeleteScheduledUnits()
		{
			while (unitsScheduledForDeletion.size())
			{
				RemoveUnitFromLists(*unitsScheduledForDeletion.begin());
			}
		}

		// create a projectile
		gc_root_ptr<Projectile>::type CreateProjectile(const gc_ptr<ProjectileType>& type, Utilities::Vector3D start, const gc_ptr<Unit>& goal, const gc_ptr<Unit>& attacker)
		{
			gc_root_ptr<Projectile>::type proj = new Projectile;
			proj->type = type;
			proj->pos = type->startPos;
			proj->goalUnit = goal;
			proj->attacker = attacker;
			std::cout << type->speed << std::endl;
			return proj;
		}

		gc_root_ptr<Projectile>::type CreateProjectile(const gc_ptr<ProjectileType>& type, Utilities::Vector3D start, Utilities::Vector3D goal, const gc_ptr<Unit>& attacker)
		{
			gc_root_ptr<Projectile>::type proj = new Projectile;
			proj->type = type;
			proj->pos = start;
			proj->goalPos = goal;
			proj->direction = goal - start;
			proj->direction.normalize();
			proj->attacker = attacker;
			return proj;
		}

		void InitUnits()
		{

			unitCreationMutex = SDL_CreateMutex();
			unitsScheduledForDisplayMutex = SDL_CreateMutex();

			numUnitsPerAreaMap = new int*[4];

			for (int j = 0; j < 4; j++)
			{
				numUnitsPerAreaMap[j] = new int[Game::Dimension::MOVEMENT_TYPES_NUM];
				for (int i = 0; i < Game::Dimension::MOVEMENT_TYPES_NUM; i++)
				{
					numUnitsPerAreaMap[j][i] = 0;
				}
			}
		
			InitUnitSquares();

//			InitRenderUnits();

			a_seed = 23467;
			r_seed = 23467;

		}

	}
}


