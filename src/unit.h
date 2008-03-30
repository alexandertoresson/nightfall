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

#include <vector>
#include <deque>

namespace Game
{
	namespace Dimension
	{
		
		extern Unit** unitByID;
		
		struct Model
		{
			int tri_count;
			int pointCount;
			int texPointCount;
			float* vertices;
			float* normals;
			float* texCoords;
			int* tris;
			int* tex_tris;
			GLuint texture;
			GLfloat *Material_Ambient;
			GLfloat *Material_Diffuse;
			GLfloat *Material_Specular;
			GLfloat *Material_Shininess;
			GLfloat *Material_Emissive;
			VBOArrays staticArrays;
			VBOArrays dynamicArrays;
		};

		struct ActionQueueVisualRepresentation
		{
			Unit* ghost;
			Position pos;
		};

		struct ActionData
		{
			IntPosition goal_pos;
			Unit* goal_unit;
			AI::UnitAction action;
			void* arg;
			ActionQueueVisualRepresentation* visual_repr;
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
			UnitType*           type;
			Player*             owner;
			Position            pos;
			IntPosition*        lastSeenPositions;
			IntPosition         curAssociatedSquare;
			IntPosition         curAssociatedBigSquare;
			float               rotation;  // how rotated the model is
			UnitAnimData        animData;
			std::deque<ActionData*>  actionQueue;
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

		extern std::vector<Unit*> unitsSelected;
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
		void ChangePath(Unit* pUnit, int goal_x, int goal_y, AI::UnitAction action, Unit* target, void* arg);
		
		Unit* GetUnitClicked(int clickx, int clicky, int map_x, int map_y);

		void PrepareUnitEssentials(Unit* const unit, UnitType* const type);
		Unit* CreateUnitNoDisplay(UnitType* type, int id = -1, bool complete = true);
		Unit* CreateUnitNoDisplay(unsigned type, Player* owner, int id = -1, bool complete = true);
		Unit* CreateUnit(UnitType* type, int x, int y, int id = -1, bool complete = true);
		Unit* CreateUnit(unsigned type, Player *owner, int x, int y, int id = -1, bool complete = true);
		bool ScheduleDisplayUnit(Unit* unit, int x, int y);
		void DisplayScheduledUnits();
		void DeleteUnit(Unit* unit);
		void KillUnit(Unit* unit);
		void ScheduleUnitDeletion(Unit* unit);
		void DeleteScheduledUnits();

		Unit* CreateGhostUnit(UnitType*);
		void DeleteGhostUnit(Unit*&);
		void PrepareUnitGhosts(Unit*);
		void ClearUnitGhosts(Unit*&);
		void CheckGhostUnits(ActionData*&);
		ActionQueueVisualRepresentation* PrepareActionDataForVisualRepr(const Unit* unit, AI::UnitAction action, void* argument, int x, int y);

		void NotEnoughPowerForLight(Unit* unit);
		void EnoughPowerForLight(Unit* unit);
		
		double GetIncomeAtNoon(Player* player);
		double GetIncomeAtNight(Player* player);
		double GetNightLength();
		double GetDayLength();
		double GetPower(Player* player);
		double GetMoney(Player* player);
		void SellPower(Player* player, double amount);
		double GetPowerAtDawn(Player* player);
		double GetPowerAtDusk(Player* player);
		
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Unit* goal);
		
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Utilities::Vector3D goal);

		void InitUnits(void);
	}
}

#ifdef DEBUG_DEP
#warning "unit.h-end"
#endif

#endif
