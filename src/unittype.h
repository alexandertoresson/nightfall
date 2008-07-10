#ifndef __UNITTYPE_H__
#define __UNITTYPE_H__

#ifdef DEBUG_DEP
#warning "unittype.h"
#endif

#include "unittype-pre.h"

#include "vector3d.h"
#include "unit-pre.h"
#include "aibase-pre.h"
#include "audio-pre.h"
#include "ogrexmlmodel.h"
#include "requirements.h"
#include "handle.h"

#include <vector>
#include <cmath>

namespace Game
{
	namespace Dimension
	{
		struct Scanline
		{
			int startX, endX;
		};

		struct RangeScanlines
		{
			int yOffset;
			int height;
			Scanline *scanlines;

			RangeScanlines() : scanlines(NULL)
			{
				
			}

			~RangeScanlines()
			{
				if (scanlines != NULL)
				{
					delete[] scanlines;
				}
			}
		
			void shade()
			{
				
			}
		};

		struct RangeArray
		{
			int offset;
			int size;
			char **array;

			RangeArray() : array(NULL)
			{
				
			}

			~RangeArray();

			void shade()
			{
			}
		};
		
		struct ProjectileType
		{
			gc_ptr<Utilities::OgreMesh> mesh;
			float               size;
			float               areaOfEffect;
			Utilities::Vector3D startPos;
			float               speed;
			bool                isHoming;
		
			ProjectileType()
			{
				
			}
			ProjectileType(char* model, float aoe, Utilities::Vector3D start_pos, float speed, float size);

			void shade()
			{
				mesh.shade();
			}
		};

		struct UnitType : public HasHandle<UnitType>
		{
			std::string name;
			std::string id;
			gc_ptr<Utilities::OgreMesh> mesh;
			float       armor;
			int         maxHealth;
			float       regenHealth;
			int         maxPower;
			float       regenPower;
			int         minAttack;     // in hitpoints
			int         maxAttack;
			bool        canAttack;
/*			int         containingCapacity;
			UnitType**  enterableUnitTypes;  // Not implemented
			bool        canEnterEnemyUnits;
			bool**      secondaryActionMatrix;*/
			float       attackAccuracy;  // accuracy of attack (0 - 100)
			float       attackMinRange;  // the minimum range of the unit's attack
			float       attackMaxRange;  // the maximum range of the unit's attack
			float       sightRange;      // how far the unit can see
			float       lightRange;      // how far the unit spreads light
			gc_ptr<RangeArray> attackRangeArray;
			gc_ptr<RangeArray> sightRangeArray;
			gc_ptr<RangeScanlines> sightRangeScanlines;
			gc_ptr<RangeArray> lightRangeArray;
			gc_ptr<RangeScanlines> lightRangeScanlines;
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
			std::vector<gc_ptr<UnitType> > canBuild;  // vector of what the unit can build, if anything at all
			std::vector<gc_ptr<Research> > canResearch;// vector of what the unit can research, if anything at all
			std::vector<std::string> canBuildIDs; // IDs of builds as string; this is used before loading is finished
			std::vector<std::string> canResearchIDs; // IDs of researches as string; this is used before loading is finished
			bool        hasAI;           // whether the unit has an AI
			bool        hasLuaAI;        // whether the unit has a lua AI
			bool        hurtByLight;
			float       size;            // size of unit -- how to scale it
			int         heightOnMap;     // this width and height only affect how much space the unit takes on the map.
			int         widthOnMap;
			float       height;          // this affects the actual height of the unit on screen (before being scaled by this->size)
			                             // and thus where the health meter should be placed
			ObjectRequirements requirements;

			gc_ptr<ProjectileType> projectileType; // set to NULL to make the unit have normal, non-ranged attacks.
			MovementType movementType;   // Type of movement the unit has
			
			GLuint	    Symbol;	     // Build symbol or Unit Symbol
			AI::UnitAIFuncs unitAIFuncs; // UnitAIFuncs
			AI::PlayerAIFuncs playerAIFuncs; // PlayerAIFuncs
			Audio::AudioFXInfo* actionSounds[Audio::SFX_ACT_COUNT];

			gc_ptr<Player> player;              // The player the unittype belongs so
			int numBuilt;
			int numExisting;

			UnitType() : attackRangeArray(NULL), sightRangeArray(NULL), sightRangeScanlines(NULL), lightRangeArray(NULL), lightRangeScanlines(NULL) 
			{
				
			}

			~UnitType()
			{
			}

			void GenerateRanges();

			void shade();

		};

	}
}

#ifdef DEBUG_DEP
#warning "unittype.h-end"
#endif

#endif
