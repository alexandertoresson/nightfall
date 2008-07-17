#include "dimension.h"

#include "sdlheader.h"
#include "ainode.h"
#include "errors.h"
#include "unit.h"
#include "textures.h"
#include "console.h"
#include "networking.h"
#include "game.h"
#include "utilities.h"
#include <cmath>
#include <set>
#include <iostream>

#define OVERLAY 1

using namespace std;

namespace Game
{
	namespace Dimension
	{	
		gc_root_ptr<World>::type pWorld     = NULL;
		gc_ptr<Unit>**    pppElements       = NULL;
		gc_ptr<Player> currentPlayerView;
		gc_ptr<Player> currentPlayer;

		InputController::InputController(void)
		{
			int length = sizeof(mKeys) / sizeof(bool);
			for (int i = 0; i < length; i++)
				mKeys[i] = false;
		}

		// Get the approximate position of a click on the map
		void GetApproximateMapPosOfClick(int clickx, int clicky, int &map_x, int &map_y)
		{
			Utilities::Vector3D meshpos;
			meshpos = Utilities::GetOGLPos(clickx, clicky);
			
			map_x = (int) floor((meshpos.x + Dimension::terrainOffsetX) * 8);
			map_y = (int) floor((meshpos.z + Dimension::terrainOffsetY) * 8);
		}

		// Get an accurate position on the terrain where the user clicked
		bool GetTerrainPosClicked(int clickx, int clicky, int map_x, int map_y, int &ter_x, int &ter_y)
		{
			int start_y, end_y;
			int start_x, end_x;
			int cur_x = 0, cur_y = 0;
			float cur_dist = 1e10, dist;
			Utilities::Vector3D tp1, tp2, tp3, near_plane, far_plane, hit_pos;

			start_y = map_y - 3 < 0 ? 0 : map_y - 3;
			start_x = map_x - 3 < 0 ? 0 : map_x - 3;

			end_y = map_y + 3 > pWorld->height-2 ? pWorld->height-2 : map_y + 3;
			end_x = map_x + 3 > pWorld->width-2 ? pWorld->width-2 : map_x + 3;

			WindowCoordToVector(clickx, clicky, near_plane, far_plane);

			for (int y = start_y; y <= end_y; y++)
			{
				for (int x = start_x; x <= end_x; x++)
				{
					tp1 = GetTerrainCoord((float)x, (float)y);
					tp2 = GetTerrainCoord((float)x, (float)y+1);
					tp3 = GetTerrainCoord((float)x+1, (float)y);
					if (CheckLineIntersectTri(tp1, tp3, tp2, near_plane, far_plane, hit_pos))
					{
						dist = near_plane.distance(hit_pos);
						if (dist < cur_dist)
						{
							cur_x = x;
							cur_y = y;
							cur_dist = dist;
						}
					}
					tp1 = GetTerrainCoord((float)x, (float)y+1);
					tp2 = GetTerrainCoord((float)x+1, (float)y+1);
					tp3 = GetTerrainCoord((float)x+1, (float)y);
					if (CheckLineIntersectTri(tp1, tp3, tp2, near_plane, far_plane, hit_pos))
					{
						dist = near_plane.distance(hit_pos);
						if (dist < cur_dist)
						{
							cur_x = x;
							cur_y = y;
							cur_dist = dist;
						}
					}
				}
			}
			if (cur_dist < 1e22)
			{
				ter_x = cur_x;
				ter_y = cur_y;
				return true;
			}
			else
			{
				return false;
			}
		}

		Player::Player(std::string name, PlayerType playertype, std::string raceScript, std::string aiScript, Utilities::Colour colour0, Utilities::Colour colour1, Utilities::Colour colour2) : raceState(NULL), aiState(NULL)
		{
			this->name = name;
			this->type = playertype;
			this->states = NULL;
			this->resources.money = 1000;
			this->resources.power = 1000;
			this->oldResources.money = 1000;
			this->oldResources.power = 1000;
			this->aiFrame = 0;
			this->isRemote = false;
			this->raceScript = raceScript;
			this->aiScript = aiScript;

			this->colours.push_back(colour0);
			this->colours.push_back(colour1);
			this->colours.push_back(colour2);

			this->playerAIFuncs.performPlayerAI.func = "PerformAI_Player";
			this->playerAIFuncs.performPlayerAI.delay = 6;
			this->playerAIFuncs.performPlayerAI.enabled = true;
			this->playerAIFuncs.unitCreation.func = "UnitEvent_UnitCreation";
			this->playerAIFuncs.commandUnit.func = "CommandUnit";
					
			this->unitAIFuncs.performUnitAI.func = "PerformAI_Unit";
			this->unitAIFuncs.performUnitAI.delay = 6;
			this->unitAIFuncs.performUnitAI.enabled = true;
			this->unitAIFuncs.commandCompleted.func = "UnitEvent_CommandCompleted";
			this->unitAIFuncs.commandCancelled.func = "UnitEvent_CommandCancelled";
			this->unitAIFuncs.newCommand.func = "UnitEvent_NewCommand";
			this->unitAIFuncs.becomeIdle.func = "UnitEvent_BecomeIdle";
			this->unitAIFuncs.isAttacked.func = "UnitEvent_IsAttacked";
			this->unitAIFuncs.unitKilled.func = "UnitEvent_UnitKilled";

			switch (playertype)
			{
				case PLAYER_TYPE_HUMAN:
					this->playerAIFuncs.performPlayerAI.func += "_Human";
					this->playerAIFuncs.unitCreation.func += "_Human";
					this->playerAIFuncs.commandUnit.func += "_Human";
					this->unitAIFuncs.performUnitAI.func += "_Human";
					this->unitAIFuncs.commandCompleted.func += "_Human";
					this->unitAIFuncs.commandCancelled.func += "_Human";
					this->unitAIFuncs.newCommand.func += "_Human";
					this->unitAIFuncs.becomeIdle.func += "_Human";
					this->unitAIFuncs.isAttacked.func += "_Human";
					this->unitAIFuncs.unitKilled.func += "_Human";
					break;
				case PLAYER_TYPE_AI:
					this->playerAIFuncs.performPlayerAI.func += "_AI";
					this->playerAIFuncs.unitCreation.func += "_AI";
					this->playerAIFuncs.commandUnit.func += "_AI";
					this->unitAIFuncs.performUnitAI.func += "_AI";
					this->unitAIFuncs.commandCompleted.func += "_AI";
					this->unitAIFuncs.commandCancelled.func += "_AI";
					this->unitAIFuncs.newCommand.func += "_AI";
					this->unitAIFuncs.becomeIdle.func += "_AI";
					this->unitAIFuncs.isAttacked.func += "_AI";
					this->unitAIFuncs.unitKilled.func += "_AI";
					break;
			}

		}

		void Player::CompleteConstruction()
		{
			raceState = new Utilities::Scripting::LuaVMState(GetRef());
			aiState = new Utilities::Scripting::LuaVMState(GetRef());

			this->index = pWorld->vPlayers.size();
			pWorld->vPlayers.push_back(this->GetRef());
			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				PlayerState* new_states = new PlayerState[pWorld->vPlayers.size()];
				if (pWorld->vPlayers.at(i)->states)
				{
					memcpy(new_states, pWorld->vPlayers.at(i)->states, sizeof(PlayerState) * (pWorld->vPlayers.size()-1));
					delete[] pWorld->vPlayers.at(i)->states;
					new_states[pWorld->vPlayers.size()-1] = PLAYER_STATE_ENEMY;
				}
				else
				{
					new_states[0] = PLAYER_STATE_NEUTRAL;
					for (unsigned int j = 1; j < pWorld->vPlayers.size()-1; j++)
					{
						new_states[j] = PLAYER_STATE_ENEMY;
					}
					new_states[pWorld->vPlayers.size()-1] = PLAYER_STATE_ALLY;
				}
				pWorld->vPlayers.at(i)->states = new_states;
			}

		}

		Player::~Player()
		{
			vUnitTypes.clear();
			vResearchs.clear();
			for(int y = 0; y < pWorld->height; y++)
			{
				delete [] this->NumUnitsSeeingSquare[y];
			}
			delete [] this->NumUnitsSeeingSquare;
			delete [] this->states;
		}

		World::~World()
		{
			vAllUnitTypes.clear();
			vPlayers.clear();
			vAllResearchs.clear();
		}

		void SetPlayerState(int player1, int player2, PlayerState state)
		{
			pWorld->vPlayers.at(player1)->states[player2] = state;
			pWorld->vPlayers.at(player2)->states[player1] = state;
		}

		void SplitString(const char* string, char splitchar, char*& string1, char*& string2)
		{
			unsigned int i;
			for (i = 0; i < strlen(string); i++)
			{
				if (string[i] == splitchar)
					break;
			}
			string1 = new char[i+1];
			memcpy(string1, string, i);
			string1[i] = 0;

			if (strlen(string) == i)
			{
				string2 = new char[1];
				string2[0] = 0;
			}
			else
			{
				string2 = new char[strlen(string)-i];
				memcpy(string2, string+i+1, strlen(string)-i-1);
				string2[strlen(string)-i-1] = 0;
			}
		}

		void UnloadUnitType(const gc_ptr<Player>& player, const gc_ptr<UnitType>& pUnitType)
		{
			// TODO
/*			int index = pUnitType->index;
			player->vUnitTypes.erase(player->vUnitTypes.begin() + index);
			unitTypeMap.erase(unitTypeMap.find(pUnitType->name));
			for (unsigned i = index; i < pWorld->vUnitTypes.size(); i++)
			{
				pWorld->vUnitTypes.at(i)->index = i;
			}*/
		}

		void UnloadAllUnitTypes()
		{
			// TODO
/*			pWorld->vUnitTypes.clear();
			unitTypeMap.clear();*/
		}

		const gc_ptr<Player>& GetCurrentPlayer()
		{
			return currentPlayer;
		}
		
		void SetCurrentPlayer(const gc_ptr<Player>& p)
		{
			currentPlayer = p;
		}
		
		void SetCurrentPlayerView(const gc_ptr<Player>& p)
		{
			currentPlayerView = p;
		}

		static struct
		{
			std::set<gc_ptr<UnitType> > sUnitTypes;
			std::set<gc_ptr<Research> > sResearchs;
		} notMeetingExistanceReqs;

		void CheckRequirements(const gc_ptr<Player>& player, ConjunctiveRequirements &reqs)
		{
			bool isSatisfied = true;
			for (std::vector<ResearchRequirement>::iterator it = reqs.researchs.begin(); it != reqs.researchs.end(); it++)
			{
				if (it->research->isResearched != it->desiredState)
				{
					isSatisfied = false;
					break;
				}
			}

			if (isSatisfied)
			{
				for (std::vector<UnitRequirement>::iterator it = reqs.units.begin(); it != reqs.units.end(); it++)
				{
					const gc_ptr<UnitType>& unitType = it->type;
					if (unitType->numBuilt > it->maxBuilt || unitType->numBuilt < it->minBuilt ||
					    unitType->numExisting > it->maxExisting || unitType->numExisting < it->minExisting)
					{
						isSatisfied = false;
						break;
					}
				}
			}
			reqs.isSatisfied = isSatisfied;
		}

		void CheckObjectRequirements(const gc_ptr<Player>& player, ObjectRequirements &requirements)
		{
			CheckRequirements(player, requirements.creation);
			CheckRequirements(player, requirements.existance);
		}

		void RecheckAllRequirements(const gc_ptr<Player>& player)
		{
			for (std::vector<gc_ptr<Research> >::iterator it = player->vResearchs.begin(); it != player->vResearchs.end(); it++)
			{
				const gc_ptr<Research>& research = *it;
				CheckObjectRequirements(player, research->requirements);
				if (!research->requirements.existance.isSatisfied && research->isResearched)
				{
					notMeetingExistanceReqs.sResearchs.insert(research);
				}
			}

			for (std::vector<gc_ptr<UnitType> >::iterator it = player->vUnitTypes.begin(); it != player->vUnitTypes.end(); it++)
			{
				const gc_ptr<UnitType>& unitType = *it;
				CheckObjectRequirements(player, unitType->requirements);
				if (!unitType->requirements.existance.isSatisfied && unitType->numExisting)
				{
					notMeetingExistanceReqs.sUnitTypes.insert(unitType);
				}
			}
			
		}
		
		void EnforceMinimumExistanceRequirements()
		{
			int n = 0;
			while ((notMeetingExistanceReqs.sResearchs.size() || notMeetingExistanceReqs.sUnitTypes.size()) && n < 5)
			{
				std::set<gc_ptr<Research> > &researchs = notMeetingExistanceReqs.sResearchs;
				std::set<gc_ptr<UnitType> > &unitTypes = notMeetingExistanceReqs.sUnitTypes;
				for (std::set<gc_ptr<Research> >::iterator it = researchs.begin(); it != researchs.end(); it++)
				{
					const gc_ptr<Research>& research = *it;
					research->isResearched = false;

 					if (research->luaEffectObj.length())
 					{
 						lua_State *pVM = research->player->aiState->GetState();

						// Make the luawrapper code believe that we're calling this function...
						research->player->aiState->SetCurFunction(research->luaEffectObj + ".undo");

						// Get the "apply" function from the user-supplied table
 						lua_getglobal(pVM, research->luaEffectObj.c_str());
 						lua_getfield(pVM, -1, "undo");
 						lua_pushlightuserdata(pVM, (void*) research->player->GetHandle());
 						research->player->aiState->CallFunction(1);
 					}
				}
				researchs.clear();
				for (std::set<gc_ptr<UnitType> >::iterator it = unitTypes.begin(); it != unitTypes.end(); it++)
				{
					for (unsigned i = 0; i < pWorld->vUnits.size(); )
					{
						const gc_ptr<Unit>& unit = pWorld->vUnits[i];
						if (unit->type == *it && unit->isDisplayed)
						{
							KillUnit(unit);
						}
						else
						{
							i++;
						}
					}
				}
				unitTypes.clear();
				for (std::vector<gc_ptr<Player> >::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
				{
					RecheckAllRequirements(*it);
				}
				n++;
			}
		}
		
		void UnloadWorld(void)
		{
			if (!pWorld)
				return;
			
			//Deallocate Units
			pWorld->vUnits.clear();

			pWorld->vPlayers.clear();

			//Deallocate Terrain & Water & Heightmap
			UnloadTerrain();
			glDeleteTextures(1, &Dimension::terraintexture);
		
			pWorld.reset();
		}
		
		gc_ptr<Research> GetResearchByID(unsigned i)
		{
			if (i < pWorld->vAllResearchs.size())
			{
				return pWorld->vAllResearchs[i];
			}
			return gc_ptr<Research>();
		}

		void PrintPlayerRefs()
		{
/*			for (std::vector<gc_ptr<Player> >::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
			{
				std::cout << (*it).c->numrefs << " ";
			}
			std::cout << std::endl;
			for (std::vector<gc_ptr<UnitType> >::iterator it = pWorld->vAllUnitTypes.begin(); it != pWorld->vAllUnitTypes.end(); it++)
			{
				if ((*it).c->numrefs < 3)
					std::cout << (*it).ref->name << ", " << (*it).ref->player->index << ": " << (*it).c->numrefs << " ";
			}
			std::cout << std::endl;*/
		}

	}
}
