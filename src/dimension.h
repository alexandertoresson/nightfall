#ifndef __DIMENSION_H__
#define __DIMENSION_H__

#ifdef DEBUG_DEP
#warning "dimension.h"
#endif

#include "dimension-pre.h"

#include "sdlheader.h"
#include "aibase.h"
#include "luawrapper.h"
#include "materialxml-pre.h"
#include "unittype.h"
#include "requirements.h"
#include "handle.h"

#include <vector>

namespace Game
{
	namespace Dimension
	{
		
		struct Research : public HasHandle<Research>
		{
			std::string id;
			std::string name;
			std::string description;
			bool isResearched;
			Unit* researcher;
			std::string luaEffectObj;
			ObjectRequirements requirements;
			GLuint icon;
			enc_ptr<Player> player;

			Research()
			{
				researcher = NULL;
			}
		};

		struct Player : public HasHandle<Player>
		{
			std::string       name;
			PlayerType        type;
			std::vector<Unit*>       vUnits;
			std::vector<Unit*>       vUnitsWithLuaAI;
			std::vector<ref_ptr<UnitType> >   vUnitTypes;
			std::vector<Projectile*> vProjectiles;
			std::vector<ref_ptr<Research> >   vResearchs;
			Uint16**          NumUnitsSeeingSquare;
			PlayerState*      states;
			Resources         resources;
			Resources         oldResources;
			std::vector<Utilities::Colour> colours;
			AI::PlayerAIFuncs playerAIFuncs;
			AI::UnitAIFuncs   unitAIFuncs;
			int               index;
			int               aiFrame;
			bool              isRemote;
			std::map<std::string, ref_ptr<UnitType> > unitTypeMap;
			std::map<std::string, ref_ptr<Research> > researchMap;
			
			std::string       raceScript;
			std::string       aiScript;

			Utilities::Scripting::LuaVMState raceState;
			Utilities::Scripting::LuaVMState aiState;

			Player(std::string name, PlayerType playertype, std::string raceScript, std::string aiScript, Utilities::Colour colour0, Utilities::Colour colour1, Utilities::Colour colour2);

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
			std::vector<ref_ptr<UnitType> > vAllUnitTypes;
			std::vector<ref_ptr<Player> > vPlayers;
			std::vector<ref_ptr<Research> > vAllResearchs;
			int             width;
			int             height;
		
			std::map<lua_State*, ref_ptr<Player> > luaStateToPlayer;
			std::map<lua_State*, Utilities::Scripting::LuaVMState*> luaStateToObject;

			~World();
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
		
		extern   ref_ptr<World>           pWorld;
		extern   Unit***                  pppElements;
		
		void GetApproximateMapPosOfClick(int clickx, int clicky, int &map_x, int &map_y);
		bool GetTerrainPosClicked(int clickx, int clicky, int map_x, int map_y, int &ter_x, int &ter_y);

		void InitPlayers(unsigned players_to_init);
		void InitPlayers(std::vector<PlayerType> playertypes);
		
		const ref_ptr<Player>& GetCurrentPlayer();
		void SetCurrentPlayer(const ref_ptr<Player>&);
		void SetCurrentPlayerView(const ref_ptr<Player>&);
		
		void UnloadUnitType(const ref_ptr<UnitType>& pUnitType);

		void RecheckAllRequirements(const ref_ptr<Player>& player);

	}
}

#ifdef DEBUG_DEP
#warning "dimension.h-end"
#endif

#endif
