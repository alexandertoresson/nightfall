#include "dimension.h"

#include "sdlheader.h"
#include "ainode.h"
#include "errors.h"
#include "unit.h"
#include "textures.h"
#include "console.h"
#include "networking.h"
#include "game.h"

#define OVERLAY 1

namespace Game
{
	namespace Dimension
	{	
		World*     pWorld            = NULL;
		Unit***    pppElements       = NULL;
		Player*    currentPlayerView = NULL;
		Player*    currentPlayer     = NULL;

		map<Player*, bool> validPlayerPointers;

		Utilities::ModelParser modelLoader;

		bool IsValidPlayerPointer(Player* player)
		{
			return validPlayerPointers[player];
		}

		InputController::InputController(void)
		{
			int length = sizeof(mKeys) / sizeof(bool);
			for (int i = 0; i < length; i++)
				mKeys[i] = false;
		}

		// Checks whether a unit is selected or not
		bool IsUnitSelected(Unit* unit)
		{
			for (vector<Unit*>::iterator it = unitsSelected.begin(); it != unitsSelected.end(); it++)
			{
				if (unit == *it)
				{
					return true;
				}
			}
			return false;
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

		// add a player
		Player* AddPlayer(std::string name, PlayerType playertype, const char* playertexture)
		{
			Player* player = new Player;
			player->name = name;
			player->type = playertype;
			player->texture = Utilities::LoadGLTexture(playertexture);
			player->index = pWorld->vPlayers.size();
			player->states = NULL;
			player->resources.money = 1000;
			player->resources.power = 1000;
			player->oldResources.money = 1000;
			player->oldResources.power = 1000;
			player->aiFrame = 0;

			player->playerAIFuncs.performPlayerAI.func = "PerformAI_Player";
			player->playerAIFuncs.performPlayerAI.delay = 6;
			player->playerAIFuncs.performPlayerAI.enabled = true;
			player->playerAIFuncs.unitCreation.func = "UnitEvent_UnitCreation";
					
			player->unitAIFuncs.performUnitAI.func = "PerformAI_Unit";
			player->unitAIFuncs.performUnitAI.delay = 6;
			player->unitAIFuncs.performUnitAI.enabled = true;
			player->unitAIFuncs.commandCompleted.func = "UnitEvent_CommandCompleted";
			player->unitAIFuncs.commandCancelled.func = "UnitEvent_CommandCancelled";
			player->unitAIFuncs.newCommand.func = "UnitEvent_NewCommand";
			player->unitAIFuncs.becomeIdle.func = "UnitEvent_BecomeIdle";
			player->unitAIFuncs.isAttacked.func = "UnitEvent_IsAttacked";
			player->unitAIFuncs.unitKilled.func = "UnitEvent_UnitKilled";

			switch (playertype)
			{
				case PLAYER_TYPE_HUMAN:
					player->playerAIFuncs.performPlayerAI.func += "_Human";
					player->playerAIFuncs.unitCreation.func += "_Human";
					player->unitAIFuncs.performUnitAI.func += "_Human";
					player->unitAIFuncs.commandCompleted.func += "_Human";
					player->unitAIFuncs.commandCancelled.func += "_Human";
					player->unitAIFuncs.newCommand.func += "_Human";
					player->unitAIFuncs.becomeIdle.func += "_Human";
					player->unitAIFuncs.isAttacked.func += "_Human";
					player->unitAIFuncs.unitKilled.func += "_Human";
					break;
				case PLAYER_TYPE_GAIA:
					player->playerAIFuncs.performPlayerAI.func += "_Gaia";
					player->playerAIFuncs.unitCreation.func += "_Gaia";
					player->unitAIFuncs.performUnitAI.func += "_Gaia";
					player->unitAIFuncs.commandCompleted.func += "_Gaia";
					player->unitAIFuncs.commandCancelled.func += "_Gaia";
					player->unitAIFuncs.newCommand.func += "_Gaia";
					player->unitAIFuncs.becomeIdle.func += "_Gaia";
					player->unitAIFuncs.isAttacked.func += "_Gaia";
					player->unitAIFuncs.unitKilled.func += "_Gaia";
					break;
				case PLAYER_TYPE_AI:
					player->playerAIFuncs.performPlayerAI.func += "_AI";
					player->playerAIFuncs.unitCreation.func += "_AI";
					player->unitAIFuncs.performUnitAI.func += "_AI";
					player->unitAIFuncs.commandCompleted.func += "_AI";
					player->unitAIFuncs.commandCancelled.func += "_AI";
					player->unitAIFuncs.newCommand.func += "_AI";
					player->unitAIFuncs.becomeIdle.func += "_AI";
					player->unitAIFuncs.isAttacked.func += "_AI";
					player->unitAIFuncs.unitKilled.func += "_AI";
					break;
				case PLAYER_TYPE_REMOTE:
					return NULL;
			}

			pWorld->vPlayers.push_back(player);
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
			validPlayerPointers[player] = true;
			return player;
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

		Model* LoadModel(const char* model)
		{
			Model* data;
			char *file, *modelname;
			if (Game::Rules::noGraphics)
				return (Model*) 0x1;
			SplitString(model, '#', file, modelname);
			if((data = modelLoader.GetModel(modelname)) != NULL)
			{
				delete[] file;
				delete[] modelname;
				return data;
			}

			int ret = modelLoader.Parse(file); 
			if(ret != SUCCESS)
			{
				string errmess = "";
				switch(ret)
				{
					case MODEL_ERROR_FILE_NOT_FOUND:
					{
						errmess = "File not found";
						break;
					}
					case MODEL_ERROR_PARSE:
					{
						errmess = "Parse error";
						break;
					}
					case MODEL_ERROR_UNEXPECTED_ERROR:
					{
						errmess = "Unexpected error";
						break;
					}
					case MODEL_ERROR_INVALID_FORMAT:
					{
						errmess = "Invalid format";
						break;
					}
					case MODEL_ERROR_NAME_CONFLICT:
					{
						errmess = "Name Conflict";
						break;
					}
					default:
					{
						errmess = "Unknown";
						break;
					}
				}
				cout << "Failed to load '" << file << "' ModelLoader says: " << errmess << endl;
				delete[] file;
				delete[] modelname;
				return NULL;
			}
			if(modelLoader.GetModelCount() != 0)
			{
				Model* model = modelLoader.GetModel(modelname);
				if (!model)
				{
					console << Console::err << "Failed to get model \"" << modelname << "\", falling back to getting latest model loaded..." << Console::nl;
					model = modelLoader.GetModel(modelLoader.GetModelCount() - 1);
				}
				console << "Triangles: " << model->tri_count << Console::nl;
				delete[] file;
				delete[] modelname;
				return model;
			}
			delete[] file;
			delete[] modelname;
			return NULL;
		}
		
		UnitType* LoadUnitType(const char* unittype) //Can't handle animations yet
		{
			UnitType* data;
			char *file, *utname;
			SplitString(unittype, '#', file, utname);
			if((data = modelLoader.GetUnit(utname)) != NULL)
			{
				delete[] file;
				delete[] utname;
				return data;
			}

			int ret = modelLoader.Parse(file); 
			if(ret != SUCCESS)
			{
				string errmess = "";
				switch(ret)
				{
					case MODEL_ERROR_FILE_NOT_FOUND:
					{
						errmess = "File not found";
						break;
					}
					case MODEL_ERROR_PARSE:
					{
						errmess = "Parse error";
						break;
					}
					case MODEL_ERROR_UNEXPECTED_ERROR:
					{
						errmess = "Unexpected error";
						break;
					}
					case MODEL_ERROR_INVALID_FORMAT:
					{
						errmess = "Invalid format";
						break;
					}
					case MODEL_ERROR_NAME_CONFLICT:
					{
						errmess = "Name Conflict";
						break;
					}
					case MODEL_ERROR_INVALID_SOUND_FORMAT:
					{
						errmess = "Invalid Sound Format";
						break;
					}
					default:
					{
						errmess = "Unknown";
						break;
					}
				}
				console << Console::err << "Failed to load '" << file << "' UnitTypeLoader says: " << errmess << Console::nl;
				delete[] file;
				delete[] utname;
				return NULL;
			}
			for (int i = 0; i < modelLoader.GetUnitCount(); i++)
			{
				pWorld->vUnitTypes.push_back(modelLoader.GetUnit(i));
				pWorld->vUnitTypes.at(pWorld->vUnitTypes.size()-1)->index = pWorld->vUnitTypes.size()-1;
			}
			if(modelLoader.GetUnitCount() != 0)
			{
				UnitType* unitType = modelLoader.GetUnit(utname);
				if (!unitType)
				{
//					cout << "Failed to get unittype \"" << utname << "\", falling back to getting latest unittype loaded..." << endl;
					unitType = modelLoader.GetUnit(modelLoader.GetUnitCount() - 1);
				}
				delete[] file;
				delete[] utname;
				return unitType;
			}
			delete[] file;
			delete[] utname;
			return NULL;
		}

		void UnloadUnitType(UnitType* pUnitType)
		{
			int index = pUnitType->index;
			pWorld->vUnitTypes.erase(pWorld->vUnitTypes.begin() + index);
			unitTypeMap.erase(unitTypeMap.find(pUnitType->name));
			for (unsigned i = index; i < pWorld->vUnitTypes.size(); i++)
			{
				pWorld->vUnitTypes.at(i)->index = i;
			}
		}

		void UnloadAllUnitTypes()
		{
			pWorld->vUnitTypes.clear();
			unitTypeMap.clear();
		}

		void InitPlayers(unsigned players_to_init)
		{
			Player* human = NULL;
			int     color = 1;
		
			AddPlayer("GAIA", PLAYER_TYPE_GAIA, "textures/player_gaia.png");
			
			PlayerType next_type = PLAYER_TYPE_HUMAN;
			
			for (unsigned i = 1; i <= players_to_init; i++)
			{
				std::stringstream texture;
				std::stringstream player;
				texture << "textures/player_" << color << ".png";
				player << "player_" << i;
				Player* p = AddPlayer(player.str(), next_type, texture.str().c_str());
				if (!human)
				{
					human = p;
				}
				if (next_type == PLAYER_TYPE_HUMAN)
				{
					next_type = PLAYER_TYPE_AI;
				}
				
				color++;
			}
			
			currentPlayerView = human;
			SetCurrentPlayer(human);
		
		}
		
		void InitPlayers(vector<PlayerType> playertypes)
		{
			Player* human = NULL;
			int     color = 1;
		
			AddPlayer("GAIA", PLAYER_TYPE_GAIA, "textures/player_gaia.png");
			
			for (unsigned i = 0; i <= playertypes.size(); i++)
			{
				std::stringstream texture;
				std::stringstream player;
				texture << "textures/player_" << color << ".png";
				player << "player_" << i+1;
				Player* p = AddPlayer(player.str(), playertypes[i], texture.str().c_str());
				if (!human)
				{
					human = p;
				}
				
				color++;
			}
			
			currentPlayerView = human;
			SetCurrentPlayer(human);
		
		}
		
		Player* GetCurrentPlayer()
		{
			return currentPlayer;
		}
		
		void SetCurrentPlayer(Player* p)
		{
			currentPlayer = p;
		}
	}
}
