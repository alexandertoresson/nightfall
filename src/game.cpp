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
#include "game.h"

#include "saveandload.h"
#include "environment.h"
#include "effect.h"
#include "vfs.h"
#include "levelhash.h"
#include "luawrapper.h"
#include "dimension.h"
#include "camera.h"
#include "aibase.h"
#include "textures.h"
#include "unitrender.h"
#include "terrain.h"
#include "gamewindow.h"
#include "unit.h"
#include "aipathfinding.h"

#include <iostream>

namespace Game
{
	namespace Rules
	{
		
		bool noGraphics = false;
		bool graphicsLoaded = true;
		bool noSound = false;
		SwitchState startState = MENU;
		int numPlayersGoal = 0;
		std::string host = "localhost";
		std::string checksumLog = "";

		std::string CurrentLevel = "default";
		std::string CurrentLevelScript;

		float time_since_last_render_frame;
		float time_passed_since_last_water_pass = 0.0;
		
		CurGame::CurGame() : last_frame(0), this_frame(SDL_GetTicks()), time_passed_since_last_ai_pass(0.0f), time_since_last_frame(0.0f), atLeastOneFrameCalculated(false), gameRunning(false), gameLogicThread(NULL) {}

		CurGame::~CurGame()
		{
			if (gameRunning)
				EndGame();
		}

		int _GameLogicThread(void* arg)
		{
			::Game::AI::InitAIThreads();

			while (CurGame::Instance()->RunGameLogicFrame())
			{
			}
			return 1;
		}

		bool CurGame::RunGameLogicFrame()
		{
			if (gameRunning)
			{
				PerformPreFrame();
				atLeastOneFrameCalculated = true;
				return true;
			}
			return false;
		}

		int CurGame::StartGame(std::string saveGame, bool isNetworked, Networking::NETWORKTYPE ntype)
		{
			gameRunning = true;

			if (saveGame.length())
				Dimension::LoadGameSaveFile(saveGame);

			if (InitGame(!saveGame.length(), isNetworked, ntype) != SUCCESS)
			{
				std::cout << _("Failed to start game, see errors above to find out why.") << std::endl;
				gameRunning = false;
				return ERROR_GENERAL;
			}
			
			if (saveGame.length())
				Dimension::LoadGame_PostLoad();

			return SUCCESS;
		}

		void CurGame::EndGame()
		{

			gameRunning = false;
			SDL_WaitThread(gameLogicThread, NULL);

			//Deletes terrain, water, player and unit.
			Dimension::UnloadWorld();
			Dimension::Environment::FourthDimension::Destroy();
			if (Networking::isNetworked)
			{
				Networking::ShutdownNetwork();
			}

			AI::QuitPathfindingThreading();

			delete FX::pParticleSystems;

			Utilities::VFS::PopState();
		}

		int CurGame::InitGame(bool is_new_game, bool isNetworked, Networking::NETWORKTYPE ntype)
		{
			std::cout << GetLevelSHA1Sum(CurrentLevel) << std::endl;
			Dimension::outerGameVFSLevel = Utilities::VFS::PushState();
			Utilities::VFS::Mount("/data/levels/" + CurrentLevel + "/", "/data/");
			
			Dimension::pWorld = new Dimension::World;

			Utilities::Scripting::InitGlobalState();

			Utilities::Scripting::LuaVMState& pVM = *Utilities::Scripting::globalVMState;

			if (noGraphics)
			{
				graphicsLoaded = false;
			}
			else
			{
				graphicsLoaded = true;
			}

			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			::Game::AI::InitAIMiscMutexes();

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Utilities::InitTextures(256);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			CurrentLevelScript = "/data/scripts/level.lua";
			if (pVM.DoFile(CurrentLevelScript) != SUCCESS)
			{
				return ERROR_GENERAL;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM.SetFunction("SetPlayers");
			if (pVM.CallFunction(0, 1) != SUCCESS)
			{
				return ERROR_GENERAL;
			}

			Utilities::Scripting::StartPlayerStates();

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM.SetFunction("InitLevel");
			if (pVM.CallFunction(0, 1) != SUCCESS)
			{
				return ERROR_GENERAL;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			Dimension::UnitMainNode::Reset();

			// Please adjust doc/rendertree.txt if you modify this

			Scene::Graph::rootNode = new Scene::Graph::Node;
			Scene::Graph::rootNode->AddChild(gc_ptr<Dimension::Camera>(&Dimension::Camera::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::Environment::EnvironmentNode>(&Dimension::Environment::EnvironmentNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::Environment::SkyboxNode>(&Dimension::Environment::SkyboxNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(new Dimension::TerrainNode);
			Dimension::Camera::instance.AddChild(new Dimension::WaterNode);
			Dimension::Camera::instance.AddChild(Dimension::UnitMainNode::GetInstance());
			// cleanup node, for resetting stuff for fixed function stuff...
			Dimension::Camera::instance.AddChild(new Scene::Render::GLStateNode(new Scene::Render::GLState()));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::BuildOutlineNode>(&Dimension::BuildOutlineNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<FX::ParticleNode>(&FX::ParticleNode::instance, null_deleter));
			Scene::Graph::rootNode->AddChild(gc_ptr<Scene::Graph::Node>(&GameWindow::Instance()->GetGUINode(), null_deleter));

			Dimension::Camera::instance.SetYMinimum(1.0f);
			Dimension::Camera::instance.SetYMaximum(15.0f);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Utilities::Scripting::InitAI();
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Game::AI::InitPathfindingThreading();
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Dimension::InitUnits();

			if (isNetworked && ntype == Networking::SERVER)
			{
				if (Networking::StartNetwork(ntype) != SUCCESS)
				{
					return ERROR_GENERAL;
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (is_new_game && (!Networking::isNetworked || Networking::networkType == Networking::SERVER))
			{
				pVM.SetFunction("InitLevelUnits");
				if (pVM.CallFunction(0, 1) != SUCCESS)
				{
					return ERROR_GENERAL;
				}
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			FX::pParticleSystems = new FX::ParticleSystemHandler(100, 100);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pDimension->SetDefaultSun();
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			//  Enable smooth shading 
			glShadeModel( GL_SMOOTH );
			
			//  Depth buffer setup 
			glClearDepth( 1.0f );

			// Enable depth testing
			glEnable(GL_DEPTH_TEST);
		
			//  The Type Of Depth Test To Do 
			glDepthFunc( GL_LEQUAL );

			glEnable(GL_CULL_FACE);

			// Enable 2d texturing
			glEnable(GL_TEXTURE_2D);

			//  Really Nice Perspective Calculations 
			glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			gameLogicThread = SDL_CreateThread(_GameLogicThread, NULL);

			return SUCCESS;
			
		}

		void CurGame::PerformPreFrame()
		{
			last_frame = this_frame;

			this_frame = SDL_GetTicks();
			time_since_last_frame = (float) (this_frame - last_frame) / 1000.0f;

			time_passed_since_last_ai_pass += time_since_last_frame;

			if (!noGraphics)
			{
				if (time_passed_since_last_ai_pass < 1 / AI::aiFps)
				{
					SDL_Delay(1);
				}

				time_passed_since_last_water_pass += time_since_last_frame;
				while (time_passed_since_last_water_pass >= (1.0f / 3))
				{
					Dimension::CalculateWater();
					time_passed_since_last_water_pass -= (1.0f / 3);
				}
				
				while (time_passed_since_last_ai_pass >= (1 / AI::aiFps))
				{

					Uint32 before_ai_pass = SDL_GetTicks();
					AI::PerformAIFrame();
					Uint32 after_ai_pass = SDL_GetTicks();
					float time_ai_pass = (float) (after_ai_pass - before_ai_pass) / 1000.0f;

					time_passed_since_last_ai_pass -= (1 / (float) AI::aiFps);
					if (time_ai_pass > (1 / (float) AI::aiFps))
					{
						time_passed_since_last_ai_pass -= time_ai_pass - (1 / (float) AI::aiFps);
					}
				}

			}
			else
			{
				AI::PerformAIFrame();
				time_passed_since_last_ai_pass = 0;
			}

/*			//Empty current UnitBuild and execute building
			if(Dimension::GetSelectedUnits().size() == 0)
			{
				if(!buildingUnit)
				{
					buildingUnit = NULL;
					//buildingGUI = NULL;
					//Window::GUI::PanelWidget obj;
					//obj.pPanel = pPlayBar;
					pPlayBar->SwitchSelected(NULL);
					//pMainGame->SetElement(selection_panel, obj, typePanel);
				}
			}
			else
			{
				const std::vector<gc_ptr<Dimension::Unit> >& v = Dimension::GetSelectedUnits();
				const gc_ptr<Dimension::Unit>& unit = v.front();
				pPlayBar->pSelected->Update();
				pPlayBar->pActions->Update();

				if(buildingUnit != unit)
				{
					pPlayBar->SwitchSelected(unit);
					buildingUnit = unit;
				}
			}*/
		}

		
		CurGame* CurGame::Instance()
		{
			return instance;
		}

		CurGame* CurGame::New()
		{
			delete instance;
			return instance = new CurGame;
		}
		
		CurGame* CurGame::instance = NULL;

	}
}
