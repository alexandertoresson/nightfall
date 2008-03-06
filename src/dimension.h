#ifndef __DIMENSION_H__
#define __DIMENSION_H__

#include "model-pre.h"
#include "aibase.h"
#include "luawrapper.h"

#ifdef DEBUG_DEP
#warning "dimension.h"
#endif

#include <vector>
#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		
		struct Resources
		{
			double power;
			double money;
			Resources()
			{
				money = 1000.0;
				power = 1000.0;
			}
		};
		
		struct Position
		{
			float x;
			float y;
			
			Position()
			{
				x = y = 0;
			}

			Position(float nx, float ny)
			{
				x = nx;
				y = ny;
			}
		};
		
		struct IntPosition
		{
			int x;
			int y;
		};

		struct UnitRequirement
		{
			UnitType *type;
			int minExisting, maxExisting;
			int minBuilt, maxBuilt;
		};

		struct ResearchRequirement
		{
			Research *research;
			bool desiredState;
		};

		struct ConjunctiveRequirements
		{
			std::vector<ResearchRequirement> researchs;
			std::vector<UnitRequirement> units;
		};

		struct DisjunctiveRequirements
		{
			std::vector<ConjunctiveRequirements> dreqs;
			std::string dReqString;
			bool isSatisfied;
			
			DisjunctiveRequirements()
			{
				isSatisfied = false;
			}
		};

		struct ObjectRequirements
		{
			DisjunctiveRequirements creation;
			DisjunctiveRequirements existance;
			int time;
			int money;
			int power;
		};

		struct Research
		{
			std::string id;
			std::string name;
			std::string description;
			bool isResearched;
			Unit* researcher;
			std::string luaEffectObj;
			ObjectRequirements requirements;
			GLuint icon;
			int index;
			Player *player;

			Research()
			{
				researcher = NULL;
			}
		};

		struct Player
		{
			std::string       name;
			PlayerType        type;
			std::vector<Unit*>       vUnits;
			std::vector<Unit*>       vUnitsWithLuaAI;
			std::vector<UnitType*>   vUnitTypes;
			std::vector<Projectile*> vProjectiles;
			std::vector<Research*>   vResearchs;
			Uint16**          NumUnitsSeeingSquare;
			PlayerState*      states;
			Resources         resources;
			Resources         oldResources;
			GLuint            texture;
			int               index;
			AI::PlayerAIFuncs playerAIFuncs;
			AI::UnitAIFuncs   unitAIFuncs;
			int               aiFrame;
			bool              isRemote;
			std::map<std::string, UnitType*> unitTypeMap;
			std::map<std::string, Research*> researchMap;
			
			std::string       raceScript;
			std::string       aiScript;

			Utilities::Scripting::LuaVMState raceState;
			Utilities::Scripting::LuaVMState aiState;

			Player(std::string name, PlayerType playertype, std::string playertexture, std::string raceScript, std::string aiScript);

			~Player();
		};
		
		struct World
		{
			float**         ppHeight;
			Uint16**        ppSteepness;
			float**         ppWater[3];
			Uint16**        NumLightsOnSquare;
			std::vector<Unit*>   vUnits;
			std::vector<Unit*>   vUnitsWithAI;
			std::vector<Player*> vPlayers;
			int             width;
			int             height;
		};
		
		class InputController
		{
			private:
				bool mKeys[SDLK_LAST];
			
			public:
				InputController(void);
			
				bool GetKeyState(SDLKey key) const       { return mKeys[key];  }
				void SetKeyState(SDLKey key, bool value) { mKeys[key] = value; }
		};
		
		extern   World*                   pWorld;
		extern   Unit***                  pppElements;
		extern   Utilities::ModelParser   modelLoader;
		
		void GetApproximateMapPosOfClick(int clickx, int clicky, int &map_x, int &map_y);
		bool GetTerrainPosClicked(int clickx, int clicky, int map_x, int map_y, int &ter_x, int &ter_y);

		// Ladda modell ur textfil
		Model*  LoadModel(const char* file);

		// Ladda unittype ur textfil
		UnitType*  LoadUnitType(const char* file);

		void InitPlayers(unsigned players_to_init);
		void InitPlayers(std::vector<PlayerType> playertypes);
		
		// Deallokerar modellen Model*
		void    UnloadModel(const Model*);
		
		Player* GetCurrentPlayer();
		void SetCurrentPlayer(Player*);
		void SetCurrentPlayerView(Player*);
		
		void UnloadUnitType(UnitType* pUnitType);
		bool IsValidPlayerPointer(Player* player);

		void RecheckAllRequirements(Player *player);

		void AddResearch(Player *player, Research *research);
	}
}

#ifdef DEBUG_DEP
#warning "dimension.h-end"
#endif

#endif
