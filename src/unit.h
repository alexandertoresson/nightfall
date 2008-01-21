#ifndef __UNIT_H_PRE__
#define __UNIT_H_PRE__

#ifdef DEBUG_DEP
#warning "unit.h-pre"
#endif

#include <map>
#include <string>
#include <deque>

using namespace std;

namespace Game
{
	namespace Dimension
	{
		struct Unit;
		struct ActionData;
		struct UnitType;
		struct Model;
		struct ProjectileType;
		struct Projectile;
		struct MorphAnim;
		struct TransformAnim;
		struct TransData;
		struct Animation;
		
		Unit *GetUnitByID(unsigned id);
		bool IsValidUnitPointer(Unit* unit); // May not be called from another thread
		bool IsDisplayedUnitPointer(Unit* unit); // May be calle from synced threads

		inline int GetTraversalTime(Unit *unit, int x, int y, int dx, int dy);
		int GetTraversalTimeAdjusted(Unit *unit, int x, int y, int dx, int dy);
		bool MoveUnit(Unit* unit);
		void ApplyScheduledBigSquareUpdates();
		bool SquareIsGoal(Unit *unit, int x, int y, bool use_internal = false);
		
		bool SquareIsWalkable(Unit *unit, int x, int y, int flags);
		bool SquaresAreWalkable(Unit *unit, int x, int y, int flags);
		inline bool SquareIsWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(Unit *unit, int x, int y);
		bool SquaresAreWalkable(UnitType *type, int x, int y, int flags);
		
		struct TransformData;
		struct TransformAnim;
		struct Animation;
		extern map<string, UnitType*> unitTypeMap;
		extern int** numUnitsPerAreaMap;
		extern int numSentCommands;
		
		void GenerateUnitTypeRanges(UnitType* type);
	}
}

#define __UNIT_H_PRE_END__

#include "dimension.h"
#include "vector3d.h"
#include "aipathfinding.h"
#include "aibase.h"
#include "terrain.h"
#include "audio.h"

#endif

#ifdef __AIBASE_H_PRE_END__
#ifdef __AIPATHFINDING_H_PRE_END__
#ifdef __DIMENSION_H_END__
#ifdef __VECTOR3D_H_END__
#ifdef __TERRAIN_H_END__
#ifdef __AUDIO_H_END__

#ifndef __UNIT_H__
#define __UNIT_H__

#ifdef DEBUG_DEP
#warning "unit.h"
#endif

namespace Game
{
	namespace Dimension
	{
		
		extern int PositionSearch_NumStepsTaken;
		extern Unit** unitByID;
		
		enum MovementType
		{
			MOVEMENT_HUMAN = 0,
			MOVEMENT_SMALLVEHICLE,
			MOVEMENT_MEDIUMVEHICLE,
			MOVEMENT_LARGEVEHICLE,
			MOVEMENT_BUILDING,
			MOVEMENT_AIRBORNE,
			MOVEMENT_SEA,
			MOVEMENT_TYPES_NUM
		};

		enum RangeType
		{
			RANGE_ATTACK,
			RANGE_SIGHT
		};

		const int SIW_DEFAULT = 0,
		          SIW_ALLKNOWING = 1,
		          SIW_IGNORE_MOVING = 2,
		          SIW_CONSIDER_WAITING = 4,
		          SIW_CONSIDER_PUSHED = 8,
		          SIW_IGNORE_OWN_MOBILE_UNITS = 16;

		struct VBOArrays
		{
			GLuint normalArray;
			GLuint vertexArray;
			GLuint texCoordArray;
			VBOArrays()
			{
				normalArray = 0;
				vertexArray = 0;
				texCoordArray = 0;
			}
		};

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

		struct MorphAnim
		{
			Model** models;            // models used for the keyframes
			float*  keyFramePositions; // position in time space of the keyframes
			int     numKeyFrames;
			float   length;            // total length of the animation
			float   lengthVariation;
			Model*  tempModel;        // model to store temporary interpolated vertices, normals, texCoords, and material parameter
		};

		struct TransformData
		{
			Utilities::Vector3D pretrans;
			Utilities::Vector3D rot;
			Utilities::Vector3D aftertrans;
			Utilities::Vector3D scale;
		};

		struct TransformAnim
		{
			MorphAnim* morphAnim;
			TransformData** transDatas;  // transformation data used
			float*  keyFramePositions; // position in time space of the keyframes
			int     numKeyFrames;
			float   length;            // total length of the animation
			float   lengthVariation;
			TransformAnim** children;
			int     numChildren;
		};

		struct Animation
		{
			TransformAnim** transAnim;
			int num_parts;
		};

		union CurAnim
		{
			Model* model;
			TransformData* transdata;
		};

		enum AnimType
		{
			ANIM_TRANSFORM,
			ANIM_MORPH
		};

		enum PowerType
		{
			POWERTYPE_DAYLIGHT,
			POWERTYPE_TWENTYFOURSEVEN
		};

		enum LightState
		{
			LIGHT_OFF,
			LIGHT_AUTO,
			LIGHT_ON
		};

		struct SingleAnimData
		{

			float   animPos; // positions into the different morphing and transforming animations for the UnitType.
			CurAnim curFrames[4]; // current frames being interpolated between
			int     nextFrameIndex;
			float   curFrameLength; // length of current frame
			SingleAnimData()
			{
				animPos = 0;
				for (int i = 0; i < 4; i++)
					curFrames[i].model = NULL;
				nextFrameIndex = 0;
				curFrameLength = 0;
			}
		};

		struct UnitAnimData
		{
			SingleAnimData** sAnimData[2];
			float transitionPos;
			float transitionLength;
			Animation* anim[2];
			bool isTransition;

			UnitAnimData()
			{
				transitionPos = 0;
				transitionLength = 0;
				isTransition = false;
				anim[0] = NULL;
				anim[1] = NULL;
			}
		};

		struct ActionData
		{
			IntPosition goal_pos;
			Unit* goal_unit;
			AI::UnitAction action;
			void* arg;
		};

		enum PrerequisiteType
		{
			PREREQUISITETYPE_RESEARCH,
			PREREQUISITETYPE_UNIT_BUILT,
			PREREQUISITETYPE_UNIT_EXISTING,
			PREREQUISITETYPE_TIME_ELAPSED
		};

		struct Research;

		struct Prerequisite
		{
			PrerequisiteType type;
			union
			{
				Unit* unit;
				Research* research;
			} object;
			int num;
		};

		struct Research
		{
			vector<Prerequisite*> prerequisites;
			bool isResearched;
			Unit** researcher;
			std::string effectFunc;
		};

		struct Scanline;
		struct RangeScanlines;
		struct RangeArray;

		struct UnitType
		{
			const char* name;
			const char* id;
			Model*      model;
			Animation*  animations[AI::ACTION_NUM];    // one per action, NULL if an animation doesn't exist for that action
			float       armor;
			int         maxHealth;
			float       regenHealth;
			int         maxPower;
			float       regenPower;
			int         minAttack;     // in hitpoints
			int         maxAttack;
			bool        canAttack;
			int         containingCapacity;
			UnitType**  enterableUnitTypes;
			bool        canEnterEnemyUnits;
			bool**      secondaryActionMatrix;
			float       attackAccuracy;  // accuracy of attack (0 - 100)
			float       attackMinRange;  // the minimum range of the unit's attack
			float       attackMaxRange;  // the maximum range of the unit's attack
			float       sightRange;      // how far the unit can see
			float       lightRange;      // how far the unit spreads light
			RangeArray* attackRangeArray;
			RangeArray* sightRangeArray;
			RangeScanlines* sightRangeScanlines;
			RangeArray* lightRangeArray;
			RangeScanlines* lightRangeScanlines;
			bool        isMobile;        // whether the unit is moveable
			PowerType   powerType;       // defines how long the unit generates light.
			double      powerIncrement;  // the power generation quantity per second per unit.
			double      powerUsage;      // the power the unit uses per second
			double      lightPowerUsage; // the power the light uses per second
			double      attackPowerUsage;// the power an attack uses
			double      buildPowerUsage; // the power building uses per second
			double      movePowerUsage;  // the power moving uses per second
			float       movementSpeed;   // in squares per second
			float       attackSpeed;     // in times per second
			vector<UnitType*> canBuild;  // vector of what the unit can build, if anything at all
			vector<UnitType*> canResearch;// vector of what the unit can research, if anything at all
			bool        hasAI;           // whether the unit has an AI
			bool        hasLuaAI;        // whether the unit has a lua AI
			bool*       isResearched;    // whether the unit is researched, for each player
			bool        hurtByLight;
			Unit**      isBeingResearchedBy;
			float       size;            // size of unit -- how to scale it
			int         heightOnMap;     // this width and height only affect how much space the unit takes on the map.
			int         widthOnMap;
			float       height;          // this affects the actual height of the unit on screen (before being scaled by this->size)
			                             // and thus where the health meter should be placed
			int         buildTime;       // seconds to build
			int         researchTime;    // seconds to research
			int         buildCost;       // cost to build
			int         researchCost;    // cost to research
			ProjectileType*     projectileType; // set to NULL to make the unit have normal, non-ranged attacks.
			MovementType movementType;   // Type of movement the unit has
			int         index;           // index of the unit in vUnitTypes
			GLuint	    Symbol;	     // Build symbol or Unit Symbol
			AI::UnitAIFuncs *unitAIFuncs; // UnitAIFuncs, one for each player
			AI::PlayerAIFuncs *playerAIFuncs; // PlayerAIFuncs, one for each player
			Audio::AudioFXInfo* actionSounds[Audio::SFX_ACT_COUNT];
		};

		struct ProjectileType
		{
			Model*              model;
			float               size;
			float               areaOfEffect;
			Utilities::Vector3D startPos;
			float               speed;
			bool                isHoming;
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
			deque<ActionData*>  actionQueue;
			AI::MovementData*   pMovementData;
			vector<Projectile*> projectiles;
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

		extern vector<Unit*> **unitBigSquares;

		extern vector<Unit*> unitsSelected;
		extern vector<Unit*> unitGroups[10];
		extern GLfloat unitMaterialAmbient[2][2][4];
		extern GLfloat unitMaterialDiffuse[2][2][4];
		extern GLfloat unitMaterialSpecular[2][2][4];
		extern GLfloat unitMaterialEmission[2][2][4];
		extern GLfloat unitMaterialShininess[2][2];
		extern GLfloat unitBuildingMaximumAltitude;

		void PlayActionSound(Unit* unit, Audio::SoundNodeAction action);
		void PlayRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action);
		void StopRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action);

		bool IsUnitSelected(Unit* unit, string action);
		void SetUnitCoordSpace(Unit* unit, bool ignoreCompleteness = false);
		void SetParticleCoordSpace(float x, float y, float z, float scale = 1.0f);
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance);
		Utilities::Vector3D GetUnitWindowPos(Unit* unit);
		void SetBillBoardCoordSystem(Unit* unit);
		void GetUnitUpperLeftCorner(Unit* unit, int& lx, int& uy);
		
		bool IsWithinRangeForBuilding(Unit* unit);
		void Build(Unit* unit);
		void Research(Unit* unit);
		void CancelBuild(Dimension::Unit* pUnit);
		void CancelResearch(Dimension::Unit* pUnit);
		float CalcUnitDamage(Unit* target, Unit* attacker);
		bool CanAttack(Unit* attacker);
		bool Attack(Unit* target, float damage);
		void InitiateAttack(Unit* attacker, Unit* target);
		void HandleProjectiles(Unit* pUnit);
		bool CanReach(Unit* attacker, Unit* target);
		void ChangePath(Unit* pUnit, int goal_x, int goal_y, AI::UnitAction action, Unit* target, void* arg);
		
		bool MovementTypeCanWalkOnSquare(MovementType mType, int x, int y);
		inline bool MovementTypeCanWalkOnSquare_UnGuarded(MovementType mType, int x, int y);
		bool MovementTypeCanWalkOnSquare_Pathfinding(MovementType mType, int size, int pos_x, int pos_y);

		inline bool SquareIsWalkable(UnitType *type, Player *player, int x, int y, int flags);
		bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y, int flags);
		inline bool SquareIsWalkable(UnitType *type, Player *player, int x, int y);
		bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y);

		bool SquareIsLighted(Player *player, int x, int y);
		bool SquareIsVisible(Player *player, int x, int y);
		inline bool SquareIsVisible_UnGuarded(Player *player, int x, int y);
		bool SquaresAreLighted(UnitType *type, Player *player, int x, int y);
		bool SquaresAreLightedAround(UnitType *type, Player *player, int x, int y);
		
		bool GetNearestSuitableAndLightedPosition(UnitType* type, Player* player, int& x, int& y);
		bool GetSuitablePositionForLightTower(UnitType* type, Player* player, int& x, int& y);
		
		Unit* GetNearestUnitInRange(Unit* unit, RangeType rangeType, PlayerState state);
		float GetLightAmountOnUnit(Unit* unit);
		bool UnitIsVisible(Unit *unit, Player*);
		bool UnitIsVisible(Unit *unit, Player *player);
		Unit* GetUnitClicked(int clickx, int clicky, int map_x, int map_y);

		Unit* CreateUnitNoDisplay(UnitType* type, Player* owner, int id = -1, bool complete = true);
		Unit* CreateUnit(UnitType* type, Player* owner, int x, int y, int id = -1, bool complete = true);
		bool ScheduleDisplayUnit(Unit* unit, int x, int y);
		void DisplayScheduledUnits();
		void DeleteUnit(Unit* unit);
		void KillUnit(Unit* unit);
		void ScheduleUnitDeletion(Unit* unit);
		void DeleteScheduledUnits();

		void UpdateSeenSquares(Unit* unit, int x, int y, int operation);
		void UpdateLightedSquares(Unit* unit, int x, int y, int operation);
		void SetLightState(Unit* unit, LightState lightState);
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

		void InitUnits(void);
		void RenderUnits(void);
		void RenderBuildOutline(UnitType* type, int start_x, int start_y);

		TransformData* CreateTransformData(Utilities::Vector3D pretrans, Utilities::Vector3D rot, Utilities::Vector3D aftertrans, Utilities::Vector3D scale);
		Animation* CreateAnimation(TransformAnim* transAnim);
		TransformAnim* CreateTransAnim(MorphAnim* morphAnim, TransformAnim** children, int numChildren, float length, int numKeyFrames, ...);
		MorphAnim* CreateMorphAnim(float length, int numKeyFrames, ...);

		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Unit* goal);
		
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Utilities::Vector3D goal);

		void InitUnits();
		void DeleteUnit(Unit* unit);

		void RenderUnits();
		void RenderHealthBars();
		
	}
}

#ifdef DEBUG_DEP
#warning "unit.h-end"
#endif

#define __UNIT_H_END__

#endif
#endif
#endif
#endif
#endif
#endif
#endif
