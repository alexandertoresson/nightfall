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
		
		extern Unit** unitByID;
		
		struct ActionQueueItem : public BaseActionData
		{
			Unit* ghost;

			ActionQueueItem(int end_x, int end_y, Unit* goal, AI::UnitAction action, const ActionArguments& args, float rotation, bool visrep) : BaseActionData(UnitGoal(goal, end_x, end_y), action, rotation, args), ghost(NULL)
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
			ProjectileType*     type;
			Utilities::Vector3D pos;
			Utilities::Vector3D direction;
			Utilities::Vector3D goalPos;
			Unit*               goalUnit;
		};

		enum FaceTarget
		{
			FACETARGET_NONE = 0,
			FACETARGET_PATH,
			FACETARGET_TARGET
		};

		struct Unit
		{
			float               health;
			float               power;
			enc_ptr<UnitType>   type;
			enc_ptr<Player>     owner;
			Position            pos;
			IntPosition*        lastSeenPositions;
			IntPosition         curAssociatedSquare;
			IntPosition         curAssociatedBigSquare;
			float               rotation;  // how rotated the model is
			std::deque<ActionQueueItem*>  actionQueue;
			AI::MovementData*   pMovementData;
			std::vector<Projectile*> projectiles;
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
			IntPosition*        rallypoint;
			Uint16              id;
			AI::UnitAIFuncs     unitAIFuncs;
			int                 aiFrame;
			Audio::SoundListNode* soundNodes[Audio::SFX_ACT_COUNT];
			int                 usedInAreaMaps;
			Uint32              pushID;
			Unit*               pusher;
			FaceTarget          faceTarget;
		};

		extern std::vector<Unit*> **unitBigSquares;

		extern std::vector<Unit*> unitsDisplayQueue;
		extern std::vector<Unit*> unitGroups[10];

		void PlayActionSound(Unit* unit, Audio::SoundNodeAction action);
		void PlayRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action);
		void StopRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action);

		float Distance2D(float x, float y);
		float Distance2D(int x, int y);

		void PerformBuild(Unit* unit);
		void PerformResearch(Unit* unit);
		void CancelBuild(Dimension::Unit* pUnit);
		void CancelResearch(Dimension::Unit* pUnit);
		float CalcUnitDamage(Unit* target, Unit* attacker);
		bool CanAttack(Unit* attacker);
		bool Attack(Unit* target, float damage);
		void InitiateAttack(Unit* attacker, Unit* target);
		void HandleProjectiles(Unit* pUnit);
		bool CanReach(Unit* attacker, Unit* target);
		void ChangePath(Unit* pUnit, int goal_x, int goal_y, AI::UnitAction action, Unit* target, const ActionArguments& args, float rotation);

		void SelectUnit(Unit* unit);
		void DeselectUnit(Unit* unit);
		void DeselectAllUnits();
		const std::vector<Unit*> GetSelectedUnits();
		
		Unit* GetUnitClicked(int clickx, int clicky, int map_x, int map_y);

		void PrepareUnitEssentials(Unit* const unit, const ref_ptr<UnitType>& type);
		Unit* CreateUnitNoDisplay(const ref_ptr<UnitType>& type, int id = -1, bool complete = true);
		Unit* CreateUnitNoDisplay(unsigned type, const ref_ptr<Player>& owner, int id = -1, bool complete = true);
		Unit* CreateUnit(const ref_ptr<UnitType>& type, int x, int y, int id = -1, bool complete = true);
		Unit* CreateUnit(unsigned type, const ref_ptr<Player>& owner, int x, int y, int id = -1, bool complete = true);
		bool ScheduleDisplayUnit(Unit* unit, int x, int y);
		void DisplayScheduledUnits();
		void RemoveUnitFromLists(Unit* unit);
		void DeleteUnit(Unit* unit);
		void KillUnit(Unit* unit);
		void ScheduleUnitDeletion(Unit* unit);
		void DeleteScheduledUnits();

		Unit* CreateGhostUnit(const ref_ptr<UnitType>&);
		void DeleteGhostUnit(Unit*&);
		void AppendToActionDisplayQueueIfOK(Unit*);
		
		void NotEnoughPowerForLight(Unit* unit);
		void EnoughPowerForLight(Unit* unit);
		
		double GetIncomeAtNoon(const ref_ptr<Player>& player);
		double GetIncomeAtNight(const ref_ptr<Player>& player);
		double GetNightLength();
		double GetDayLength();
		double GetPower(const ref_ptr<Player>& player);
		double GetMoney(const ref_ptr<Player>& player);
		void SellPower(const ref_ptr<Player>& player, double amount);
		double GetPowerAtDawn(const ref_ptr<Player>& player);
		double GetPowerAtDusk(const ref_ptr<Player>& player);
		
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Unit* goal);
		
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Utilities::Vector3D goal);

		void InitUnits(void);
	}
}

#ifdef DEBUG_DEP
#warning "unit.h-end"
#endif

#endif
