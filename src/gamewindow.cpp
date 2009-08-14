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
#include "gamewindow.h"

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
#include "camera.h"
#include "unitrender.h"
#include "scenegraph.h"
#include "ogrexmlmodel.h"
#include "game.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cassert>
#include <sstream>

using namespace Window::GUI;
using namespace std;

namespace Game
{
	namespace Rules
	{
		
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
						if (CurGame::New()->StartGame() == SUCCESS)
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
						if (CurGame::New()->StartGame("save.xml") == SUCCESS)
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
						CurGame::Instance()->EndGame();
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
						if (CurGame::New()->StartGame("", true, Networking::SERVER) == SUCCESS)
						{
							nextState = (SwitchState)networkCreate->RunLoop();
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
						if (Networking::StartNetwork(Networking::CLIENT) == SUCCESS)
						{
							nextState = (SwitchState)networkJoin->RunLoop();
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
			pLoading->SetMessage(_("Awaiting network..."));
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

			goto_x = 0.0f;
			goto_y = 0.0f;
			
			build_x = -1;
			build_y = -1;
				
			goto_time = 0;

			buildingUnit = NULL;
			pGameInput = NULL;

			renderMutex = SDL_CreateMutex();
			renderMutex2 = SDL_CreateMutex();
			pauseRendering = false;

			InitGUI();

		}

		int GameWindow::InitGUI()
		{
//			float increment = 0.1f / 3.0f; //0.1 (10%) divided on 1 update;

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
			p_lblMoney->SetTooltip(_("Resource: Money"));

			p_lblPower = new Window::GUI::InfoLabel();
			p_lblPower->AttachHandler(&GetPower);
			p_lblPower->SetTooltip(_("Resource: Power"));

			p_lblTime = new Window::GUI::InfoLabel();
			p_lblTime->AttachHandler(&GetTime);
			p_lblTime->SetTooltip(_("Time"));

			sell = new Window::GUI::TextButton();
			sell->SetText(_("Sell"));
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

//			pLoading->Increment(increment);

			pTopBar->init();
			
//			pLoading->Increment(increment);

			pPlayBar->init();
			pPlayBar->SwitchSelected(NULL);

			pConsole->SetType(0);
			pConsole->PrepareBuffer();
			console.WriteLine(_("Nightfall (Codename Twilight)"));
			
//			pLoading->Increment(increment);

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
			DestroyGUI();
		}
		
		bool GameWindow::ProcessEvents()
		{
			//SDL Events
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if(pMainPanel != NULL)
				{
					switch(event.type)
					{
						case SDL_KEYDOWN:
						{
							pMainPanel->HandleEvent(KB_DOWN, &event, NULL);
							break;
						}
						case SDL_KEYUP:
						{
							pMainPanel->HandleEvent(KB_UP, &event, NULL);
							break;
						}
						case SDL_MOUSEMOTION:
						{
							//Translate position
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_MOVE, &event, ptr = TranslateMouseCoords(event.motion.x, event.motion.y));
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONDOWN:
						{
							TranslatedMouse *ptr;
							if(event.button.button == SDL_BUTTON_WHEELUP || event.button.button == SDL_BUTTON_WHEELDOWN)
							{
								pMainPanel->HandleEvent(MOUSE_SCROLL, &event, ptr = TranslateMouseCoords(event.button.x, event.button.y));
							}
							else
							{
								pMainPanel->HandleEvent(MOUSE_DOWN, &event, ptr = TranslateMouseCoords(event.button.x, event.button.y));
							}
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONUP:
						{
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_UP, &event, ptr = TranslateMouseCoords(event.button.x, event.button.y));
							delete ptr;
							break;
						}
					}
				}

				switch(event.type)
				{
					case SDL_QUIT:
					{
						go = false;
						break;	
					}
				}
			}
			
			//Key States operations
			if (input.GetKeyState(SDLK_UP))
				Dimension::Camera::instance.Fly(-1.5f * time_since_last_render_frame);
					
			else if (input.GetKeyState(SDLK_DOWN))
				Dimension::Camera::instance.Fly(1.5f * time_since_last_render_frame);

				
			if (input.GetKeyState(SDLK_LEFT))
				Dimension::Camera::instance.FlyHorizontally(-Game::Dimension::cameraFlySpeed * time_since_last_render_frame);
			else if (input.GetKeyState(SDLK_RIGHT))
				Dimension::Camera::instance.FlyHorizontally(Game::Dimension::cameraFlySpeed * time_since_last_render_frame);
				
			if (input.GetKeyState(SDLK_PAGEUP))
				Dimension::Camera::instance.Zoom(Game::Dimension::cameraZoomSpeed * time_since_last_render_frame);
			else if (input.GetKeyState(SDLK_PAGEDOWN))
				Dimension::Camera::instance.Zoom(-Game::Dimension::cameraZoomSpeed * time_since_last_render_frame);

			if (input.GetKeyState(SDLK_HOME))
				Dimension::Camera::instance.Rotate(Game::Dimension::cameraRotationSpeed * time_since_last_render_frame);
			else if (input.GetKeyState(SDLK_END))
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

			GUINode::instance.SetParams(pMainPanel, w, h);

			Scene::Graph::Node::TraverseFullTree();

			SDL_GL_SwapBuffers();
			return true;
		}

		
		void GameWindow::PauseRendering()
		{
			SDL_LockMutex(renderMutex2);
			pauseRendering = true;
			SDL_LockMutex(renderMutex);
		}
		
		void GameWindow::ResumeRendering()
		{
			pauseRendering = false;
			SDL_UnlockMutex(renderMutex);
			SDL_UnlockMutex(renderMutex2);
		}

		int GameWindow::RunLoop()
		{
//			Uint32 last_save = SDL_GetTicks();

			if (!Game::Rules::noGraphics)
				glClearColor( 0.2f, 0.2f, 0.2f, 0.7f );

			CurGame::Instance()->StartGameLogicThread();

			while (!CurGame::Instance()->AtLeastOneFrameCalculated())
			{
				SDL_Delay(1);
			}

			go = true;

			Uint32 lastFrameTime = SDL_GetTicks();

			SDL_LockMutex(renderMutex);

			while(go)
			{
				time_since_last_render_frame = (SDL_GetTicks() - lastFrameTime) / 1000.0;
				lastFrameTime = SDL_GetTicks();

				if (Game::Rules::noGraphics)
					SDL_Delay(1);
				else
					PaintAll();
			
				ProcessEvents();

				if (pauseRendering)
				{
					SDL_UnlockMutex(renderMutex);

					SDL_LockMutex(renderMutex2); // Force a context switch; renderMutex2 is locked
					SDL_UnlockMutex(renderMutex2);

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
//			SDL_WaitThread(gameThread, NULL);
			return returnValue;
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
				
		GameWindow::GUINode& GameWindow::GetGUINode()
		{
			return GUINode::instance;
		}
	}
}
