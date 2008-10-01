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

#include "dimension.h"
#include "font.h"
#include "aibase.h"
#include "vector3d.h"
#include "terrain.h"
#include "audio.h"
#include "unit.h"
#include "environment.h"
#include "console.h"
#include "gamegui.h"
#include "networking.h"
#include "textures.h"
#include "aipathfinding.h"
#include "saveandload.h"
#include "paths.h"
#include "camera.h"
#include "effect.h"
#include "unitrender.h"
#include "scenegraph.h"
#include "ogrexmlmodel.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <sstream>

#define OVERLAY 1
#define SKYBOX 0

using namespace Window::GUI;
using namespace std;

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

		GameWindow* GameWindow::pInstance = NULL;

		GameWindow* GameWindow::Instance()
		{
			if (GameWindow::pInstance == NULL)
				GameWindow::pInstance = new GameWindow;

			return GameWindow::pInstance;
		}
		
		bool GameWindow::IsNull()
		{
			if (GameWindow::pInstance == NULL)
				return true;
				
			return false;
		}

		void GameWindow::Destroy()
		{
			if (GameWindow::pInstance == NULL)
				return;

			delete GameWindow::pInstance;
			GameWindow::pInstance = NULL;
		}

		Window::GUI::ConsoleBuffer* GameWindow::GetConsoleBuffer() const
		{
			return pConsole;
		}

		float time_since_last_render_frame;
		float time_passed_since_last_water_pass = 0.0;
		
//////////////////////
//////////////////////
//////////////////////

//New GUI Code

//////////////////////
//////////////////////
//////////////////////

		int GameMain()
		{
			GameMenu *pMenu = new GameMenu();
			GameInGameMenu *pInGameMenu = new GameInGameMenu();
			SwitchState nextState = startState;
			GameWindow *mainWindow = NULL;
			//GameTest *mainTestin = new GameTest();
			NetworkJoinOrCreate *multiplayer = new NetworkJoinOrCreate();
			NetworkJoin *networkJoin = new NetworkJoin();
			NetworkCreate *networkCreate = new NetworkCreate();

			while(nextState != QUIT)
			{
				switch(nextState)
				{
					case MENU:
					{
						Audio::PlayList("menu");
						nextState = (SwitchState)pMenu->RunLoop();
						Audio::StopAll();
						break;
					}
					case NEWGAME:
					{
						mainWindow = GameWindow::Instance();
						if (mainWindow->NewGame() == SUCCESS)
						{
							nextState = GAME;
						}
						else
						{
							nextState = QUIT;
						}
						break;
					}
					case LOADGAME:
					{
						mainWindow = GameWindow::Instance();
						if (mainWindow->LoadGame() == SUCCESS)
						{
							nextState = GAME;
						}
						else
						{
							nextState = QUIT;
						}
						break;
					}
					case ENDGAME:
					{
						mainWindow = GameWindow::Instance();
						mainWindow->EndGame();
						nextState = MENU;
						break;
					}
					case GAME:
					{
						Audio::PlayList("ingame");
						mainWindow = GameWindow::Instance();
						nextState = (SwitchState)mainWindow->RunLoop();
						Audio::StopAll();
						break;
					}
					case INGAMEMENU:
					{
						Audio::PlayList("menu");
						nextState = (SwitchState)pInGameMenu->RunLoop();
						Audio::StopAll();
						break;
					}
					case MULTIPLAYER:
					{
						Networking::isNetworked = true;
						nextState = (SwitchState)multiplayer->RunLoop();
						if(nextState == MENU)
							Networking::isNetworked = false;
						break;
					}
					case NETWORKCREATE:
					{
						Networking::isNetworked = true;
						mainWindow = GameWindow::Instance();
						if (mainWindow->NewGame(true, Networking::SERVER) == SUCCESS)
						{
							nextState = (SwitchState)networkCreate->RunLoop();
							if(nextState == MENU)
								mainWindow->EndGame();
						}
						else
						{
							nextState = QUIT;
						}
						break;
					}
					case NETWORKJOIN:
					{
						Networking::isNetworked = true;
						mainWindow = GameWindow::Instance();
						if (mainWindow->NewGame(true, Networking::CLIENT) == SUCCESS)
						{
							nextState = (SwitchState)networkJoin->RunLoop();
							if(nextState == MENU)
								mainWindow->EndGame();
						}
						else
						{
							nextState = QUIT;
						}
						break;
					}
					default:
					{
						nextState = QUIT;
						break;
					}
				}
			}
			GameWindow::Destroy();
			delete pMenu;
			return SUCCESS;
		}

		void WaitForNetwork()
		{
			Window::GUI::LoadWindow *pLoading = new Window::GUI::LoadWindow(1.0f);
			pLoading->SetMessage("Awaiting network...");
			pLoading->SetProgress(0.0f);
			float counter = 0.0f;
			while(Networking::isReadyToStart == false)
			{
				Networking::PerformPregameNetworking();
				counter += 0.05f;
				if(counter > 1.0f)
					counter = 0.0f;

				pLoading->SetProgress(counter);
				pLoading->Update();
				SDL_Delay(16);
			}
			delete pLoading;
		}

		GameWindow::GameWindow()
		{
			frames = 0;
			last_status_time = 0;
			start_drag_x = 0;
			start_drag_y = 0;
			end_drag_x = 0;
			end_drag_y = 0;

			is_pressing_lmb = false;
			last_frame = 0;
			this_frame = SDL_GetTicks();

			time_passed_since_last_ai_pass = 0.0f;
			goto_x = 0.0f;
			goto_y = 0.0f;
			
			build_x = -1;
			build_y = -1;
				
			goto_time = 0;
			pause = 0;

			buildingUnit = NULL;
			pGameInput = NULL;

			part_x = 160.0f;
			part_y = 70.0f;

			gameRunning = false;

			atLeastOneFrameCalculated = false;

			renderMutex = SDL_CreateMutex();
			pauseRendering = false;

		}

		int GameWindow::InitGUI()
		{
			float increment = 0.1f / 3.0f; //0.1 (10%) divided on 1 update;

			//Initate Panel and place GameInput
			pMainGame = new Window::GUI::Panel();
			pGameInput = new GameInput(this);

			SetPanel(pMainGame);
			float sizes[2] = {0.04f, 0.2f};
			
			pConsole = new Window::GUI::ConsoleBuffer();
			consoleID = pMainGame->Add(pConsole);

			pMainGame->SetConstraintPercent(consoleID, 0.0f, 0.0f, 1.0f, 1.0f);
			pMainGame->SetVisible(consoleID, false);

			int mainWidget = pMainGame->Add(pGameInput);
			pMainGame->SetConstraintPercent(mainWidget, 0.0f, sizes[0], 1.0f, 1.0f - sizes[0] - sizes[1]);
			pMainGame->SetFocus(mainWidget);
			pMainGame->SetFocusEnabled(false);

			pTopBar = new GameTopBar();
			pPlayBar = new GamePlayBar(this);

			int topBar = pMainGame->Add(pTopBar);
			pMainGame->SetConstraintPercent(topBar, 0.0f, 0.0f, 1.0f, sizes[0]);

			p_lblMoney = new Window::GUI::InfoLabel();
			p_lblMoney->AttachHandler(&GetMoney);
			p_lblMoney->SetTooltip("Resource: Money");

			p_lblPower = new Window::GUI::InfoLabel();
			p_lblPower->AttachHandler(&GetPower);
			p_lblPower->SetTooltip("Resource: Power");

			p_lblTime = new Window::GUI::InfoLabel();
			p_lblTime->AttachHandler(&GetTime);
			p_lblTime->SetTooltip("Time");

			sell = new Window::GUI::TextButton();
			sell->SetText("Sell");
			sell->AttachHandler(&Sell);
			sell->SetTag(this);
			sell->SetType(1);

			int lblmoney = pTopBar->Add(p_lblMoney);
			int lblpower = pTopBar->Add(p_lblPower);
			int lbltime = pTopBar->Add(p_lblTime);
			int sellid = pTopBar->Add(sell);

			pTopBar->SetConstraintPercent(lblpower, 0.0f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(lblmoney, 0.25f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(lbltime, 0.5f, 0.0f, 0.25f, 0.8f);
			pTopBar->SetConstraintPercent(sellid, 0.75f, 0.0f, 0.25f, 0.8f);

			int playBar = pMainGame->Add(pPlayBar);
			pMainGame->SetConstraintPercent(playBar, 0.0f, 1.0f - sizes[1], 1.0f, sizes[1]);

			pLoading->Increment(increment);

			pTopBar->init();
			
			pLoading->Increment(increment);

			pPlayBar->init();
			pPlayBar->SwitchSelected(NULL);

			pConsole->SetType(0);
			pConsole->PrepareBuffer();
			console.WriteLine("Nightfall (Codename Twilight)");
			
			pLoading->Increment(increment);

			return SUCCESS;
		}
		
		void GameWindow::Sell(Window::GUI::EventType evt, void* arg)
		{
			if (Networking::isNetworked)
			{
				Networking::PrepareSell(Dimension::GetCurrentPlayer(), 100);
			}
			else
			{
				Dimension::SellPower(Dimension::GetCurrentPlayer(), 100);
			}
		}

		void GameWindow::DestroyGUI()
		{
			//Initate Panel and place GameInput
			delete pMainGame;
			delete pGameInput;
			delete pConsole;
			delete pPlayBar;
			delete pTopBar;
			delete p_lblMoney;
			delete p_lblPower;
			delete p_lblTime;
			delete sell;
		}

		GameWindow::~GameWindow()
		{
			//GUI Elements
			if(gameRunning == true)
				DestroyGUI();
		}
		
		int GameWindow::NewGame(bool isNetworked, Networking::NETWORKTYPE ntype)
		{
			pLoading = new Window::GUI::LoadWindow(1.0f); //90% Game, 10% GUI
			pLoading->SetMessage("Loading...");
			pLoading->Update();

			if (InitGame(true, isNetworked, ntype) != SUCCESS)
			{
				cout << "Failed to start game, see errors above to find out why." << endl;
				gameRunning = false;
				return ERROR_GENERAL;
			}
			
			delete pLoading;
			gameRunning = true;
			return SUCCESS;
		}

		int GameWindow::LoadGame(bool isNetworked, Networking::NETWORKTYPE ntype)
		{
			pLoading = new Window::GUI::LoadWindow(1.0f); //90% Game, 10% GUI
			pLoading->SetMessage("Loading...");
			pLoading->Update();

			Dimension::LoadGameSaveFile("save.xml");
			
			if (InitGame(false, isNetworked, ntype) != SUCCESS)
			{
				cout << "Failed to start game, see errors above to find out why." << endl;
				gameRunning = false;
				return ERROR_GENERAL;
			}

			Dimension::LoadGame_PostLoad();
			
			delete pLoading;
			gameRunning = true;
			return SUCCESS;
		}

		void GameWindow::EndGame()
		{
			//Deletes terrain, water, player and unit.
			Dimension::UnloadWorld();
			Dimension::Environment::FourthDimension::Destroy();
			DestroyGUI();
			if (Networking::isNetworked)
			{
				Networking::ShutdownNetwork();
			}

#ifdef USE_MULTITHREADED_CALCULATIONS
			AI::QuitPathfindingThreading();
#endif

			delete this->input;
			delete FX::pParticleSystems;
			gameRunning = false;
		}

		int GameWindow::InitGame(bool is_new_game, bool isNetworked, Networking::NETWORKTYPE ntype)
		{
			input = new Dimension::InputController();
			float increment = 0.9f / 13.0f; //0.9 (90%) divided on 13 updates...
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
			Game::AI::InitAIMiscMutexes();
			pLoading->Increment(increment);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Utilities::InitTextures(256);
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			CurrentLevelScript = "levels/" + CurrentLevel + "_level.lua";
			if (pVM.DoFile(CurrentLevelScript) != SUCCESS)
			{
				return ERROR_GENERAL;
			}

			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM.SetFunction("SetPlayers");
			if (pVM.CallFunction(0, 1) != SUCCESS)
			{
				return ERROR_GENERAL;
			}
			pLoading->Increment(increment);

			Utilities::Scripting::StartPlayerStates();
			pLoading->Increment(increment);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pVM.SetFunction("InitLevel");
			if (pVM.CallFunction(0, 1) != SUCCESS)
			{
				return ERROR_GENERAL;
			}
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			// Please adjust doc/rendertree.txt if you modify this

			Scene::Graph::rootNode = new Scene::Graph::Node;
			Scene::Graph::rootNode->AddChild(gc_ptr<Dimension::Camera>(&Dimension::Camera::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::Environment::EnvironmentNode>(&Dimension::Environment::EnvironmentNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::Environment::SkyboxNode>(&Dimension::Environment::SkyboxNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(new Dimension::TerrainNode);
			Dimension::Camera::instance.AddChild(new Dimension::WaterNode);
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::UnitMainNode>(&Dimension::UnitMainNode::instance, null_deleter));
			// cleanup node, for resetting stuff for fixed function stuff...
			Dimension::Camera::instance.AddChild(new Scene::Render::GLStateNode(new Scene::Render::GLState()));
			Dimension::Camera::instance.AddChild(gc_ptr<Dimension::BuildOutlineNode>(&Dimension::BuildOutlineNode::instance, null_deleter));
			Dimension::Camera::instance.AddChild(gc_ptr<FX::ParticleNode>(&FX::ParticleNode::instance, null_deleter));
			Scene::Graph::rootNode->AddChild(gc_ptr<Scene::Graph::Node>(&GUINode::instance, null_deleter));

			Dimension::Camera::instance.SetYMinimum(1.0f);
			Dimension::Camera::instance.SetYMaximum(15.0f);

			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Game::Rules::GameWindow::Instance()->InitGUI();
			pLoading->Increment(increment);

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Utilities::Scripting::InitAI();
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_MULTITHREADED_CALCULATIONS
			Game::AI::InitPathfindingThreading();
#endif
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Dimension::InitUnits();
			pLoading->Increment(increment);

			if (isNetworked)
			{
				Networking::StartNetwork(ntype);
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
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			FX::pParticleSystems = new Game::FX::ParticleSystemHandler(100, 100);
			pLoading->Increment(increment);
			
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			pDimension->SetDefaultSun();
			
			pLoading->Increment(increment);
			
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

			return SUCCESS;
			
		}

		bool GameWindow::ProcessEvents()
		{
			//SDL Events
			SDL_Event *event = new SDL_Event;
			while (SDL_PollEvent(event))
			{
				if(pMainPanel != NULL)
				{
					switch(event->type)
					{
						case SDL_KEYDOWN:
						{
							pMainPanel->HandleEvent(KB_DOWN, event, NULL);
							break;
						}
						case SDL_KEYUP:
						{
							pMainPanel->HandleEvent(KB_UP, event, NULL);
							break;
						}
						case SDL_MOUSEMOTION:
						{
							//Translate position
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_MOVE, event, ptr = TranslateMouseCoords(event->motion.x, event->motion.y));
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONDOWN:
						{
							TranslatedMouse *ptr;
							if((*event).button.button == SDL_BUTTON_WHEELUP || (*event).button.button == SDL_BUTTON_WHEELDOWN)
							{
								pMainPanel->HandleEvent(MOUSE_SCROLL, event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							}
							else
							{
								pMainPanel->HandleEvent(MOUSE_DOWN, event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							}
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONUP:
						{
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_UP,event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							delete ptr;
							break;
						}
					}
				}

				switch(event->type)
				{
					case SDL_QUIT:
					{
						go = false;
						break;	
					}
				}
			}

			delete event;
			
			//Key States operations
			if (input->GetKeyState(SDLK_UP))
				Dimension::Camera::instance.Fly(-1.5f * time_since_last_render_frame);
					
			else if (input->GetKeyState(SDLK_DOWN))
				Dimension::Camera::instance.Fly(1.5f * time_since_last_render_frame);

				
			if (input->GetKeyState(SDLK_LEFT))
				Dimension::Camera::instance.FlyHorizontally(-Game::Dimension::cameraFlySpeed * time_since_last_render_frame);
			else if (input->GetKeyState(SDLK_RIGHT))
				Dimension::Camera::instance.FlyHorizontally(Game::Dimension::cameraFlySpeed * time_since_last_render_frame);
				
			if (input->GetKeyState(SDLK_PAGEUP))
				Dimension::Camera::instance.Zoom(Game::Dimension::cameraZoomSpeed * time_since_last_render_frame);
			else if (input->GetKeyState(SDLK_PAGEDOWN))
				Dimension::Camera::instance.Zoom(-Game::Dimension::cameraZoomSpeed * time_since_last_render_frame);

			if (input->GetKeyState(SDLK_HOME))
				Dimension::Camera::instance.Rotate(Game::Dimension::cameraRotationSpeed * time_since_last_render_frame);
			else if (input->GetKeyState(SDLK_END))
				Dimension::Camera::instance.Rotate(-Game::Dimension::cameraRotationSpeed * time_since_last_render_frame);
			
	
			return true;
		}

		bool GameWindow::PaintAll()
		{
			// nollställ backbufferten och depthbufferten
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// nollställ vyn
			glLoadIdentity();

			Audio::PlaceSoundNodes(Dimension::Camera::instance.GetPosVector());

			AI::aiFramesPerformedSinceLastRender = 0;

			GUINode::instance.SetParams(pMainPanel, w, h);

			Scene::Graph::Node::TraverseFullTree();

			SDL_GL_SwapBuffers();
			return true;
		}

		void GameWindow::PerformPreFrame()
		{
			if (last_frame != this_frame)
			{
				last_frame = this_frame;
			}
			this_frame = SDL_GetTicks();
			time_since_last_frame = (float) (this_frame - last_frame) / 1000.0f;

			time_passed_since_last_ai_pass += time_since_last_frame;

			if (!Game::Rules::noGraphics)
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

			//Empty current UnitBuild and execute building
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
			}
		}

		
		int _GameLogicThread(void* arg)
		{
			Game::AI::InitAIThreads();

			while (GameWindow::Instance()->RunGameLogicFrame())
			{
			}
			return 1;
		}

		int GameWindow::RunGameLogicFrame()
		{
			if (go)
			{
				PerformPreFrame();
				atLeastOneFrameCalculated = true;
				return 1;
			}
			return 0;
		}

		int GameWindow::RunLoop()
		{
//			Uint32 last_save = SDL_GetTicks();

			if (!Game::Rules::noGraphics)
				glClearColor( 0.2f, 0.2f, 0.2f, 0.7f );

			go = true;

			SDL_Thread* gameThread = SDL_CreateThread(_GameLogicThread, NULL);

			while (!atLeastOneFrameCalculated)
			{
				SDL_Delay(1);
			}

			Uint32 lastFrameTime = SDL_GetTicks();

			SDL_LockMutex(renderMutex);

			while(go)
			{
				time_since_last_render_frame = (SDL_GetTicks() - lastFrameTime) / 1000.0;
				lastFrameTime = SDL_GetTicks();

				if (Game::Rules::noGraphics)
					SDL_Delay(1);

				if (!Game::Rules::noGraphics)
					PaintAll();
			
				ProcessEvents();

				if (pauseRendering)
				{
					SDL_UnlockMutex(renderMutex);

					SDL_Delay(1); // Force a context switch; I found that just unlocking and locking didn't
					              // guarantee at all that the other thread manages to lock the mutex
					              
					SDL_LockMutex(renderMutex);
				}

				frames++;

				if (SDL_GetTicks() - last_status_time >= 1000)
				{
					int avgcount = 0;
					if (AI::numPaths)
					{
						avgcount = AI::cCount / AI::numPaths;
					}
//					if (Game::Rules::noGraphics)
						cout << "Fps: " << ((float) frames / (((float) (SDL_GetTicks() - last_status_time)) / 1000.0f)) << " " << Dimension::pWorld->vUnits.size() << " " << AI::currentFrame << " c: " << AI::cCount << " t: " << AI::tCount << " f: " << AI::fCount << " p: " << AI::pCount << " n: " << AI::numPaths << " q: " << AI::GetQueueSize() << " f: " << AI::numFailed << " a: " << avgcount << " nrf: " << AI::notReachedFlood << " nrp: " << AI::notReachedPath << " nsc: " << Dimension::numSentCommands << " ngs1: " << AI::numGreatSuccess1 << " ngs2: " << AI::numGreatSuccess2 << " ntf: " << AI::numTotalFrames << endl;
//					else
						console << "Fps: " << ((float) frames / (((float) (SDL_GetTicks() - last_status_time)) / 1000.0f)) << " " << Dimension::pWorld->vUnits.size() << Console::nl;
					AI::cCount = 0;
					AI::tCount = 0;
					AI::fCount = 0;
					AI::pCount = 0;
					AI::numPaths = 0;
					AI::numFailed = 0;
					AI::notReachedFlood = 0;
					AI::notReachedPath = 0;
					Dimension::numSentCommands = 0;
					AI::numGreatSuccess1 = 0;
					AI::numGreatSuccess2 = 0;
					AI::numTotalFrames = 0;

					frames = 0;
					last_status_time = SDL_GetTicks();
				}
/*				if (SDL_GetTicks() - last_save >= 300000)
				{
					bool exists;
					time_t rawtime;

					time(&rawtime);

					char* time_string = ctime(&rawtime);
					if (time_string[strlen(time_string)-1] == '\n')
					{
						time_string[strlen(time_string)-1] = '\0';
					}

					std::string filename = Utilities::GetWritableDataFile("autosaves/Autosave " + (std::string) time_string + ".xml", exists);
					if (filename.length())
					{
						Dimension::SaveGame(filename);
						system(("bzip2 \"" + filename + "\"").c_str());
					}
					last_save = SDL_GetTicks();
				}*/

			}
			SDL_UnlockMutex(renderMutex);
			SDL_WaitThread(gameThread, NULL);
			return returnValue;
		}

		void GameWindow::Stop()
		{
			go = false;
		}

		void GameWindow::PauseRendering()
		{
			pauseRendering = true;
			SDL_LockMutex(renderMutex);
		}
		
		void GameWindow::ResumeRendering()
		{
			pauseRendering = false;
			SDL_UnlockMutex(renderMutex);
		}
		
		GameWindow::GUINode::GUINode() : pMainPanel(NULL), w(0), h(0)
		{

		}

		void GameWindow::GUINode::Render()
		{
			matrices[MATRIXTYPE_MODELVIEW].Apply();
			if(pMainPanel != NULL)
			{
				Utilities::SwitchTo2DViewport(w, h);
				pMainPanel->Paint();
				pMainPanel->PaintTooltip();
				Utilities::RevertViewport();
			}
		}

		void GameWindow::GUINode::SetParams(Panel* pMainPanel, float w, float h)
		{
			this->pMainPanel = pMainPanel;
			this->w = w;
			this->h = h;
		}
		
		GameWindow::GUINode GameWindow::GUINode::instance;
	}
}
