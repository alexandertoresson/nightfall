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
#ifndef __UNIT_H__
#define __UNIT_H__

#ifdef DEBUG_DEP
#warning "unit.h"
#endif

#include "unit-pre.h"

#include "aipathfinding-pre.h"
#include "terrain.h"
#include "audio.h"
#include "dimension.h"
#include "vector3d.h"
#include "unittype.h"
#include "unitrender-pre.h"
#include "action.h"

#include <vector>
#include <deque>

namespace Game
{
	namespace Dimension
	{
		
		struct ActionQueueItem : public BaseActionData
		{
			gc_ptr<Unit> ghost;

			ActionQueueItem(int end_x, int end_y, const gc_ptr<Unit>& goal, AI::UnitAction action, const ActionArguments& args, float rotation, bool visrep) : BaseActionData(UnitGoal(goal, end_x, end_y), action, rotation, args), ghost(NULL)
			{
				this->action = action;
				this->rotation = rotation;
				this->args = args;
				this->goal.pos.x = end_x;
				this->goal.pos.y = end_y;
				this->goal.unit = goal;
				if (visrep)
					CreateVisualRepresentation();
			}

			~ActionQueueItem();
			
			private:
				void CreateVisualRepresentation();
			
		};

		struct Projectile
		{
			gc_ptr<ProjectileType> type;
			Utilities::Vector3D pos;
			Utilities::Vector3D direction;
			Utilities::Vector3D goalPos;
			gc_ptr<Unit>       goalUnit;
			gc_ptr<Unit>       attacker;

			void shade()
			{
				type.shade();
				goalUnit.shade();
				attacker.shade();
			}
		};

		enum FaceTarget
		{
			FACETARGET_NONE = 0,
			FACETARGET_PATH,
			FACETARGET_TARGET
		};

		struct UnitSoundStatus
		{
			bool active;
			Audio::SoundListNode node;

			UnitSoundStatus() : active(false)
			{
				
			}
		};

		struct Unit : HasHandle<Unit>
		{
			float               health;
			float               power;
			gc_ptr<UnitType>   type;
			gc_ptr<Player>     owner;
			Position            pos;
			std::vector<IntPosition> lastSeenPositions;
			IntPosition         curAssociatedSquare;
			IntPosition         curAssociatedBigSquare;
			float               rotation;  // how rotated the model is
			std::deque<ActionQueueItem>  actionQueue;
			gc_ptr<AI::MovementData> pMovementData;
			std::vector<gc_ptr<Projectile> > vProjectiles;
			Uint32              lastAttack;    // frame of the last attack done by the unit
			Uint32              lastAttacked;    // frame of the last attack done at the unit
			Uint32              lastCommand;
			float               completeness;
			float               action_completeness;
			bool                isCompleted;
			bool                isDisplayed;
			bool                isMoving;
			bool                isLighted;   // the unit has squares added that are marked as lighted
			bool                hasSeen;     // the unit has squares added that are marked as seen
			bool                isWaiting;
			bool                isPushed;
			bool                hasPower;
			LightState          lightState;
			gc_ptr<IntPosition> rallypoint;
			AI::UnitAIFuncs     unitAIFuncs;
			int                 aiFrame;
			UnitSoundStatus soundNodes[Audio::SFX_ACT_COUNT];
			int                 usedInAreaMaps;
/*			Uint32              pushID;
			Unit*               pusher;*/
			FaceTarget          faceTarget;

			~Unit();

			void shade()
			{
				type.shade();
				owner.shade();
				pMovementData.shade();
				gc_shade_container(vProjectiles);
				rallypoint.shade();
			}
		};

		extern std::vector<gc_ptr<Unit> > **unitBigSquares;

		extern std::vector<gc_ptr<Unit> > unitsDisplayQueue;
		extern std::vector<gc_ptr<Unit> > unitGroups[10];

		void PlayActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action);
		void PlayRepeatingActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action);
		void StopRepeatingActionSound(const gc_ptr<Unit>& unit, Audio::SoundNodeAction action);

		float Distance2D(float x, float y);
		float Distance2D(int x, int y);

		void PerformBuild(gc_ptr<Unit>& unit);
		void PerformResearch(gc_ptr<Unit>& unit);
		void CancelBuild(const gc_ptr<Unit>& pUnit);
		void CancelResearch(const gc_ptr<Unit>& pUnit);
		float CalcUnitDamage(const gc_ptr<Unit>& target, const gc_ptr<Unit>& attacker);
		bool CanAttack(const gc_ptr<Unit>& attacker);
		bool Attack(gc_ptr<Unit>& target, float damage);
		void InitiateAttack(gc_ptr<Unit>& attacker, gc_ptr<Unit>& target);
		void HandleProjectiles(const gc_ptr<Player>& player);
		void HandleProjectiles(const gc_ptr<Unit>& unit);
		bool CanReach(const gc_ptr<Unit>& attacker, const gc_ptr<Unit>& target);
		void ChangePath(const gc_ptr<Unit>& pUnit, int goal_x, int goal_y, AI::UnitAction action, const gc_ptr<Unit>& target, const ActionArguments& args, float rotation);

		void SelectUnit(const gc_ptr<Unit>& unit);
		void DeselectUnit(const gc_ptr<Unit>& unit);
		void DeselectAllUnits();
		const std::vector<gc_ptr<Unit> >& GetSelectedUnits();
		
		void PrepareUnitEssentials(gc_ptr<Unit>& unit, const gc_ptr<UnitType>& type);
		gc_ptr<Unit> CreateUnitNoDisplay(const gc_ptr<UnitType>& type, int id = -1, bool complete = true);
		gc_ptr<Unit> CreateUnit(const gc_ptr<UnitType>& type, int x, int y, int id = -1, bool complete = true);
		bool ScheduleDisplayUnit(const gc_ptr<Unit>& unit, int x, int y);
		void DisplayScheduledUnits();
		void RemoveUnitFromLists(gc_ptr<Unit> unit);
		void KillUnit(gc_ptr<Unit> unit);
		void ScheduleUnitDeletion(const gc_ptr<Unit>& unit);
		void DeleteScheduledUnits();

		gc_ptr<Unit> CreateGhostUnit(const gc_ptr<UnitType>&);
		void AppendToActionDisplayQueueIfOK(const gc_ptr<Unit>&);
		
		void NotEnoughPowerForLight(const gc_ptr<Unit>& unit);
		void EnoughPowerForLight(const gc_ptr<Unit>& unit);
		
		double GetIncomeAtNoon(const gc_ptr<Player>& player);
		double GetIncomeAtNight(const gc_ptr<Player>& player);
		double GetNightLength();
		double GetDayLength();
		double GetPower(const gc_ptr<Player>& player);
		double GetMoney(const gc_ptr<Player>& player);
		void SellPower(const gc_ptr<Player>& player, double amount);
		double GetPowerAtDawn(const gc_ptr<Player>& player);
		double GetPowerAtDusk(const gc_ptr<Player>& player);
		
		gc_root_ptr<Projectile>::type CreateProjectile(const gc_ptr<ProjectileType>& type, Utilities::Vector3D start, const gc_ptr<Unit>& goal, const gc_ptr<Unit>& attacker);
		
		gc_root_ptr<Projectile>::type CreateProjectile(const gc_ptr<ProjectileType>& type, Utilities::Vector3D start, Utilities::Vector3D goal, const gc_ptr<Unit>& attacker);

		void InitUnits(void);
	}
}

#ifdef DEBUG_DEP
#warning "unit.h-end"
#endif

#endif
