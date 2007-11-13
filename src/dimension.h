#ifndef __DIMENSION_H_PRE__
#define __DIMENSION_H_PRE__

#ifdef DEBUG_DEP
#warning "dimension.h-pre"
#endif

namespace Game
{
	namespace Dimension
	{
		struct World;
		struct Player;
		struct Position;
		struct Resources;
		
		extern Player*       currentPlayerView;
		extern Player*       currentPlayer;
		
		class InputController;
		
		enum PlayerType
		{
			PLAYER_TYPE_HUMAN,
			PLAYER_TYPE_AI,
			PLAYER_TYPE_GAIA,
			PLAYER_TYPE_REMOTE
		};

		typedef int PlayerState;

		const int PLAYER_STATE_ALLY = 1,
		          PLAYER_STATE_NEUTRAL = 2,
		          PLAYER_STATE_ENEMY = 4;
		
		void UnloadAllUnitTypes();
	}
}

#define __DIMENSION_H_PRE_END__

#include "model.h"
#include "vector3d.h" // inclusions needed for __DIMENSION_H__
#include "aibase.h"

#endif

#ifdef __VRMLPARSER_H_PRE_END__

#ifdef __AIBASE_H_END__

#ifdef __VECTOR3D_H_END__

#ifndef __DIMENSION_H__
#define __DIMENSION_H__

#ifdef DEBUG_DEP
#warning "dimension.h"
#endif

#include <vector>
#include <cmath>
#include <cassert>
#include <iostream>
#include "sdlheader.h"

using namespace std;
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

		struct Player
		{
			std::string       name;
			PlayerType        type;
			vector<Unit*>     vUnits;
			vector<Unit*>     vUnitsWithLuaAI;
			vector<Projectile*> vProjectiles;
			Uint16**          NumUnitsSeeingSquare;
			PlayerState*      states;
			Resources         resources;
			Resources         oldResources;
			GLuint            texture;
			int               index;
			AI::PlayerAIFuncs playerAIFuncs;
			AI::UnitAIFuncs   unitAIFuncs;
			int               aiFrame;
		};
		
		struct World
		{
			float**         ppHeight;
			Uint16**        ppSteepness;
			float**         ppWater[3];
			Uint16**        NumLightsOnSquare;
			vector<Unit*>   vUnits;
			vector<Unit*>   vUnitsWithAI;
			vector<UnitType*>   vUnitTypes;
			vector<Player*> vPlayers;
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
		Player* AddPlayer(std::string name, PlayerType playertype, const char* playertexture);

		// Ladda modell ur textfil
		Model*  LoadModel(const char* file);

		// Ladda unittype ur textfil
		UnitType*  LoadUnitType(const char* file);

		void InitPlayers(unsigned players_to_init);
		void InitPlayers(vector<PlayerType> playertypes);
		
		// Deallokerar modellen Model*
		void    UnloadModel(const Model*);
		
		Player* GetCurrentPlayer();
		void SetCurrentPlayer(Player*);
		
		void UnloadUnitType(UnitType* pUnitType);
		bool IsValidPlayerPointer(Player* player);
	}
}

#ifdef DEBUG_DEP
#warning "dimension.h-end"
#endif

#define __DIMENSION_H_END__

#endif
#endif
#endif
#endif
