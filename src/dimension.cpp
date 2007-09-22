#include "dimension.h"

#include "sdlheader.h"
#include "ainode.h"
#include "errors.h"
#include "unit.h"
#include "textures.h"
#include "console.h"
#include "networking.h"

#define OVERLAY 1

namespace Game
{
	namespace Dimension
	{	
		World*     pWorld            = NULL;
		Unit***    pppElements       = NULL;
		Player*    currentPlayerView = NULL;
		Player*    currentPlayer     = NULL;

		Utilities::ModelParser modelLoader;

		InputController::InputController(void)
		{
			int length = sizeof(mKeys) / sizeof(bool);
			for (int i = 0; i < length; i++)
				mKeys[i] = false;
		}

		// Checks whether a unit selected or not
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
			float cur_dist = 1e23, dist;
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
					tp1 = GetTerrainCoord(x, y);
					tp2 = GetTerrainCoord(x, y+1);
					tp3 = GetTerrainCoord(x+1, y);
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
					tp1 = GetTerrainCoord(x, y+1);
					tp2 = GetTerrainCoord(x+1, y+1);
					tp3 = GetTerrainCoord(x+1, y);
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
		Player* AddPlayer(char* name, PlayerType playertype, char* playertexture)
		{
			Player* player = new Player;
			strncpy(player->name, name, 16);
			player->type = playertype;
			player->aiPlayerData = new AI::PlayerAIData;
			player->texture = Utilities::LoadGLTexture(playertexture);
			pWorld->vPlayers.push_back(player);
			player->index = pWorld->vPlayers.size()-1;
			player->states = NULL;
			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				PlayerState* new_states = new PlayerState[pWorld->vPlayers.size()];
				if (pWorld->vPlayers.at(i)->states)
				{
					memcpy(new_states, pWorld->vPlayers.at(i)->states, sizeof(PlayerState*) * (pWorld->vPlayers.size()-1));
					delete[] pWorld->vPlayers.at(i)->states;
					new_states[pWorld->vPlayers.size()-1] = PLAYER_STATE_NEUTRAL;
				}
				else
				{
					for (unsigned int j = 0; j < pWorld->vPlayers.size(); j++)
					{
						new_states[j] = PLAYER_STATE_NEUTRAL;
					}
				}
				pWorld->vPlayers.at(i)->states = new_states;
			}
			player->states[pWorld->vPlayers.size()-1] = PLAYER_STATE_ALLY;
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
			if (Game::Networking::isDedicatedServer)
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
			size_t players_initialized = pWorld->vPlayers.size();
		
			if (players_initialized > 0)
			{
				players_initialized -= 1;
				if (players_to_init > players_initialized)
				{
					players_to_init -= players_initialized;
				
					for (unsigned i = 1; i <= players_initialized; i++)
					{
						if (pWorld->vPlayers.at(i)->type == PLAYER_TYPE_HUMAN)
							human = pWorld->vPlayers.at(i);
							
						color++;
					}
				}
				else
				{
					SetCurrentPlayer(pWorld->vPlayers.at(1));
					return;
				}
			}
			else
			{
				AddPlayer((char*) "GAIA", PLAYER_TYPE_GAIA, (char*) "textures/player_gaia.png");
			}
			
			PlayerType next_type;
			
			if (human == NULL)
				next_type = PLAYER_TYPE_HUMAN;
			else
				next_type = PLAYER_TYPE_HUMAN;
			
			for (unsigned i = 1; i <= players_to_init; i++)
			{
				char texture[32];
				char player[16];
				sprintf(texture, "textures/player_%d.png", color);
				sprintf(player, "player_%d", players_initialized + i);
				Player* p = AddPlayer(player, next_type, texture);
				if (next_type == PLAYER_TYPE_HUMAN)
				{
					human = p;
					next_type = PLAYER_TYPE_AI;
				}
				
				color++;
			}
			
			for (unsigned i = 1; i <= players_to_init + players_initialized; i++)
			{
				for (unsigned j = 1; j <= players_to_init + players_initialized; j++)
				{
					if (j == i)
						continue;
					
					SetPlayerState(i, j, PLAYER_STATE_ENEMY);
				}
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
