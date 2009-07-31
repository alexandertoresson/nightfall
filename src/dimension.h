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
#ifndef DIMENSION_H
#define DIMENSION_H

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
#include "lockfreequeue.h"
#include "action.h"
#include "vfs-pre.h"

#include <vector>

namespace Game
{
	namespace Dimension
	{
		
		struct Player : public HasHandle<Player>
		{
			std::string       name;
			PlayerType        type;
			std::vector<gc_ptr<Unit> >       vUnits;
			std::vector<gc_ptr<Unit> >       vUnitsWithLuaAI;
			std::vector<gc_ptr<UnitType> >   vUnitTypes;
			std::vector<gc_ptr<Projectile> > vProjectiles;
			std::vector<gc_ptr<Research> >   vResearchs;
			lockfreequeue<gc_ptr<UnitEvent> > scheduledUnitEvents;
			SDL_mutex *scheduleUnitEventMutex;
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
			std::map<std::string, gc_ptr<UnitType> > unitTypeMap;
			std::map<std::string, gc_ptr<Research> > researchMap;
			
			std::string       raceScript;
			std::string       aiScript;

			gc_ptr<Utilities::Scripting::LuaVMState> raceState;
			gc_ptr<Utilities::Scripting::LuaVMState> aiState;

			Player(std::string name, PlayerType playertype, std::string raceScript, std::string aiScript, Utilities::Colour colour0, Utilities::Colour colour1, Utilities::Colour colour2);

			~Player();

			void CompleteConstruction();

			void shade()
			{
				gc_shade_container(vUnits);
				gc_shade_container(vUnitsWithLuaAI);
				gc_shade_container(vUnitTypes);
				gc_shade_container(vResearchs);
				gc_shade_container(vProjectiles);
				gc_shade_container(scheduledUnitEvents);
				gc_shade_map(unitTypeMap);
				gc_shade_map(researchMap);
				raceState.shade();
				aiState.shade();
			}
		};
		
		struct World
		{
			Uint16**        NumLightsOnSquare;
			std::vector<gc_ptr<Unit> >   vUnits;
			std::vector<gc_ptr<Unit> >   vUnitsWithAI;
			std::vector<gc_ptr<UnitType> > vAllUnitTypes;
			std::vector<gc_ptr<Player> > vPlayers;
			std::vector<gc_ptr<Research> > vAllResearchs;
			int             width;
			int             height;
		
			std::map<lua_State*, gc_ptr<Player> > luaStateToPlayer;
			std::map<lua_State*, Utilities::Scripting::LuaVMState*> luaStateToObject;

			~World();

			void shade()
			{
				gc_shade_container(vUnits);
				gc_shade_container(vUnitsWithAI);
				gc_shade_container(vAllUnitTypes);
				gc_shade_container(vPlayers);
				gc_shade_container(vAllResearchs);

				gc_shade_map(luaStateToPlayer);
			}
		};
		
		extern   gc_root_ptr<World>::type        pWorld;
		extern   gc_ptr<Unit>**                  pppElements;
				
		extern Utilities::VFS::VFSLevel outerGameVFSLevel;
		
		void GetApproximateMapPosOfClick(int clickx, int clicky, int &map_x, int &map_y);
		bool GetTerrainPosClicked(int clickx, int clicky, int map_x, int map_y, int &ter_x, int &ter_y);

		void InitPlayers(unsigned players_to_init);
		void InitPlayers(std::vector<PlayerType> playertypes);
		
		const gc_ptr<Player>& GetCurrentPlayer();
		void SetCurrentPlayer(const gc_ptr<Player>&);
		void SetCurrentPlayerView(const gc_ptr<Player>&);
		
		void UnloadUnitType(const gc_ptr<UnitType>& pUnitType);

		void RecheckAllRequirements(const gc_ptr<Player>& player);

		void PrintPlayerRefs();
	}
}

#ifdef DEBUG_DEP
#warning "dimension.h-end"
#endif

#endif
