#include "gamegui.h"

#include <cmath>
#include <sstream>
#include "game.h"
#include "gui.h"
#include "dimension.h"
#include "unit.h"
#include "effect.h"
#include "environment.h"
#include "networking.h"
#include "aipathfinding.h"
#include "saveandload.h"

using namespace Window::GUI;

namespace Game
{
	namespace Rules
	{
		GameMenu::GameMenu()
		{
			pStart = new Window::GUI::TextButton();
			pStart->SetText("Start the game");
			pStart->SetTag((void*)new Internal(this, NEWGAME));
			pStart->AttachHandler(&GameMenu::ActionSelect);

			pLoad = new Window::GUI::TextButton();
			pLoad->SetText("Load a game");
			pLoad->SetTag((void*)new Internal(this, LOADGAME));
			pLoad->AttachHandler(&GameMenu::ActionSelect);

			pCredits = new Window::GUI::TextButton();
			pCredits->SetText("About");
			pCredits->SetTag((void*)new Internal(this, CREDITS));
			pCredits->AttachHandler(&GameMenu::ActionSelect);

			pQuit = new Window::GUI::TextButton();
			pQuit->SetText("Quit");
			pQuit->SetTag((void*)new Internal(this, QUIT));
			pQuit->AttachHandler(&GameMenu::ActionSelect);

			pMultiplayer = new Window::GUI::TextButton();
			pMultiplayer->SetText("Multiplayer");
			pMultiplayer->SetTag((void*)new Internal(this, MULTIPLAYER));
			pMultiplayer->AttachHandler(&GameMenu::ActionSelect);

			pMainPanel = new Window::GUI::Panel();
			SetPanel(pMainPanel);
			pMainPanel->SetTexture(Utilities::LoadTexture("textures/nightfall.png"));

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pStart), 0.1f, 0.05f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pLoad), 0.1f, 0.15f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pCredits), 0.1f, 0.25f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pMultiplayer), 0.1f, 0.35f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pQuit), 0.1f, 0.45f, 0.8f, 0.1f);
			returnState = QUIT;

			sleep = true;
			sleepms = 16;
		}

		GameMenu::~GameMenu()
		{
			delete pStart;
			delete pLoad;
			delete pCredits;
			delete pMultiplayer;
			delete pQuit;
//			delete pMainPanel;
		}

		void GameMenu::ActionSelect(Window::GUI::EventType evtType, void* tag)
		{
			GameMenu::Internal *pTag = (GameMenu::Internal*)tag;
			pTag->parent->go = false;
			switch(pTag->action)
			{
				case NEWGAME:
				{
					pTag->parent->pStart->ResetFade();
					break;
				}
				case LOADGAME:
				{
					pTag->parent->pLoad->ResetFade();
					break;
				}
				case MULTIPLAYER:
				{
					pTag->parent->pMultiplayer->ResetFade();
					break;
				}
				default:
					break;
			}
			pTag->parent->returnValue = pTag->action;
		}

		FileBrowserDialog::FileBrowserDialog(BrowserMode mode)
		{
			this->mode = mode;
			this->extension = FileBrowserDialog::ALL_FILES;
			this->path = ".";

			pHeader = new Window::GUI::Label();
			pHeader->SetText((mode == FileBrowserDialog::MODE_LOAD) ? "Load" : "Save");

			pMainPanel = new Panel();
			SetPanel(pMainPanel);

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pHeader), 0.1f, 0.05f, 0.8f, 0.1f);
			
			returnState = MENU;

			sleep = true;
			sleepms = 16;

			PopulateList();
		}

		FileBrowserDialog::~FileBrowserDialog()
		{
			delete pHeader;

			DeallocButtons();
		}

		void FileBrowserDialog::PopulateList()
		{
			if (!Utilities::FileExists(this->path))
				this->path = ".";

			DeallocButtons();

			Utilities::FSDataList files;			
			this->filesCount = Utilities::ListFilesInDirectory(this->path, &files);

			if (this->filesCount < 1)
				return;

			this->fileList = new FileEntry*[this->filesCount];
			for (int i = 0; i < this->filesCount; i++)
			{
				FileEntry* entry = this->fileList[i];
				
				entry->buttonId = 0;
				entry->file = files.at(i);
			}
		}

		void FileBrowserDialog::DeallocButtons()
		{
			if (!this->filesCount)
				return;

			for (int i = 0; i < this->filesCount; i++)
			{
				pMainPanel->Delete(this->fileList[i]->buttonId, true);
				delete this->fileList[i];
			}

			delete [] this->fileList;
		}

		GameInGameMenu::GameInGameMenu()
		{

			pContinue = new Window::GUI::TextButton();
			pContinue->SetText("Continue");
			pContinue->SetTag((void*)new Internal(this, GAME));
			pContinue->AttachHandler(&GameInGameMenu::ActionSelect);

			pEndGame = new Window::GUI::TextButton();
			pEndGame->SetText("End Game");
			pEndGame->SetTag((void*)new Internal(this, ENDGAME));
			pEndGame->AttachHandler(&GameInGameMenu::ActionSelect);

			pQuit = new Window::GUI::TextButton();
			pQuit->SetText("Quit To OS");
			pQuit->SetTag((void*)new Internal(this, QUIT));
			pQuit->AttachHandler(&GameInGameMenu::ActionSelect);

			pMainPanel = new Window::GUI::Panel();
			SetPanel(pMainPanel);

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pContinue), 0.1f, 0.1f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pEndGame), 0.1f, 0.2f, 0.8f, 0.1f);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pQuit), 0.1f, 0.3f, 0.8f, 0.1f);
			returnState = QUIT;

			sleep = true;
			sleepms = 16;
		}

		GameInGameMenu::~GameInGameMenu()
		{
			delete pContinue;
			delete pEndGame;
			delete pQuit;
		}

		void GameInGameMenu::ActionSelect(Window::GUI::EventType evtType, void* tag)
		{
			GameInGameMenu::Internal *pTag = (GameInGameMenu::Internal*)tag;
			pTag->parent->go = false;
			switch(pTag->action)
			{
				case GAME:
				{
					pTag->parent->pContinue->ResetFade();
					break;
				}
				case ENDGAME:
				{
					pTag->parent->pEndGame->ResetFade();
					break;
				}
				default:
					break;
			}
			pTag->parent->returnValue = pTag->action;
		}

		GameTest::GameTest()
		{
			pMainPanel = new Window::GUI::Panel();
			SetPanel(pMainPanel);
			int textbox = pMainPanel->Add(pTextBox = new Window::GUI::TextBox());
			int button = pMainPanel->Add(pExit = new Window::GUI::TextButton());

			pExit->AttachHandler(&GameTest::ActionSelect);
			pExit->SetTag(this);

			pMainPanel->SetConstraint(textbox, 0.2f, 0.2f, 0.5f, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraint(button, 0.1f, 0.5f, this->w - 0.2f, 0.1f);

			pExit->SetText("Exit");

			pMainPanel->SetFocusEnabled(true);
			pMainPanel->SetFocus(textbox);

			sleep = true;
			sleepms = 16;
		}

		void GameTest::ActionSelect(Window::GUI::EventType evt, void* arg)
		{
			GameTest *win = (GameTest*)arg;
			win->go = false;
		}

		GameInput::GameInput(GameWindow *ref)
		{
			pGame = ref;
			start_fx = start_fy = end_fx = end_fy = 0.0f;
		}

#ifdef WIN32
		float round(float value)
		{
			return floor(value + 0.5f);
		}
#endif

		string GetPower()
		{
			Game::Dimension::Player* player = Game::Dimension::currentPlayerView;
			std::stringstream sstr, sstr2;
			string change_str;

			double change = (int) round((player->resources.power - player->oldResources.power) * AI::aiFps * 10) / 10.0;
			sstr2 << change;

			if (change > -1e-3 && change < 1e-3)
				change_str = "+/-" + sstr2.str();
			else
				change_str = sstr2.str();

			sstr << (int) player->resources.power;
			return "Power: " + sstr.str() + " (" + change_str + ")";
		}

		string GetMoney()
		{
			Game::Dimension::Player* player = Game::Dimension::currentPlayerView;
			std::stringstream sstr, sstr2;
			string change_str;

			double change = (int) round((player->resources.money - player->oldResources.money) * AI::aiFps * 10) / 10.0;
			sstr2 << change;

			if (change > -1e-3 && change < 1e-3)
				change_str = "+/-" + sstr2.str();
			else
				change_str = sstr2.str();

			sstr << (int) player->resources.money;
			return "Money: " + sstr.str() + " (" + change_str + ")";
		}

		string GetTime()
		{
			Game::Dimension::Environment::FourthDimension* pDimension = Game::Dimension::Environment::FourthDimension::Instance();
			std::stringstream sstr, sstr2;

			sstr << setfill('0') << setw(2) << (int) pDimension->GetCurrentHour();
			sstr2 << setfill('0') << setw(2) << (int) (pDimension->GetCurrentHour() * 60) % 60;

			return "Time: " + sstr.str() + ":" + sstr2.str();
		}

		int GameInput::Paint()
		{
			if (pGame->is_pressing_lmb && sqrt(pow((float) pGame->start_drag_x - pGame->end_drag_x, 2) + pow((float)pGame->start_drag_y - pGame->end_drag_y, 2)) > 5.0f)
			{
				glBegin(GL_QUADS);
				glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
					glVertex2f(start_fx, start_fy);
					glVertex2f(start_fx, end_fy);
					glVertex2f(end_fx, end_fy);
					glVertex2f(end_fx, start_fy);
				glEnd();
			}

			return SUCCESS;
		}

		void GameInput::MouseDownLeft(SDL_Event* event,Window::GUI::TranslatedMouse* translatedMouse)
		{
			start_fx = translatedMouse->x;
			start_fy = translatedMouse->y;
			end_fx = translatedMouse->x;
			end_fy = translatedMouse->y;
			pGame->start_drag_x = (*event).button.x;
			pGame->start_drag_y = (*event).button.y;
			pGame->is_pressing_lmb = true;	
		}

		void GameInput::MouseDownRight(SDL_Event* event,Window::GUI::TranslatedMouse* translatedMouse)
		{
			/* ~~~ this shouldn't be neccessary! / Leo
			start_fx = translatedMouse->x;
			start_fy = translatedMouse->y;
			end_fx = translatedMouse->x;
			end_fy = translatedMouse->y;
			pGame->start_drag_x = (*event).button.x;
			pGame->start_drag_y = (*event).button.y;
			*/
			int map_x = 0;
			int map_y = 0;
			Dimension::GetApproximateMapPosOfClick((*event).button.x, (*event).button.y, map_x, map_y);
			console << "(" << map_x << ", " << map_y << ")" << Console::nl;
		}

		void GameInput::AddSelectedUnit(Dimension::Unit* unit)
		{
			if (unit->owner == Dimension::currentPlayerView)
			{
				for (unsigned i = 0; i < Dimension::unitsSelected.size(); i++)
				{
					if (Dimension::unitsSelected[i]->owner != Dimension::currentPlayerView)
					{
						Dimension::unitsSelected.erase(Dimension::unitsSelected.begin() + i--);
						continue;
					}
					if (unit == Dimension::unitsSelected[i])
					{
						return;
					}
				}
			}
			else
			{
				for (vector<Dimension::Unit*>::iterator it = Dimension::unitsSelected.begin(); it != Dimension::unitsSelected.end(); it++)
				{
					if (unit == *it || (*it)->owner == Dimension::currentPlayerView)
					{
						return;
					}
				}
			}
			Dimension::unitsSelected.push_back(unit);
		}

		void GameInput::MouseUpLeft(SDL_Event* event,Window::GUI::TranslatedMouse* translatedMouse)
		{
			int map_x, map_y;
			int ter_x, ter_y;
			int tmp;
			Dimension::Unit *unit;
			pGame->is_pressing_lmb = false;

			if (pGame->build_type)
			{
				Dimension::GetApproximateMapPosOfClick((*event).button.x, (*event).button.y, map_x, map_y);
				if (Dimension::GetTerrainPosClicked((*event).button.x, (*event).button.y, map_x, map_y, ter_x, ter_y))
				{
					if (Dimension::SquaresAreWalkable(pGame->build_type, Dimension::GetCurrentPlayer(), ter_x, ter_y, Dimension::SIW_IGNORE_OWN_MOBILE_UNITS))
					{
						if (Dimension::unitsSelected.size() == 1)
						{
							AI::CommandUnit(Dimension::unitsSelected.at(0), ter_x, ter_y, AI::ACTION_BUILD, (void*) pGame->build_type, true, false);
						}
						else
						{
							AI::CommandUnits(Dimension::unitsSelected, ter_x, ter_y, AI::ACTION_BUILD, (void*) pGame->build_type, true, false);
						}
						pGame->build_type = NULL;
					}
				}
			}
			else
			{
				if (sqrt(pow((float) pGame->start_drag_x - pGame->end_drag_x, 2) + pow((float)pGame->start_drag_y - pGame->end_drag_y, 2)) > 5.0f)
				{
					if (pGame->start_drag_x > pGame->end_drag_x)
					{
						tmp = pGame->start_drag_x;
						pGame->start_drag_x = pGame->end_drag_x;
						pGame->end_drag_x = tmp;
					}
					if (pGame->start_drag_y > pGame->end_drag_y)
					{
						tmp = pGame->start_drag_y;
						pGame->start_drag_y = pGame->end_drag_y;
						pGame->end_drag_y = tmp;
					}

					if (!pGame->input->GetKeyState(SDLK_LSHIFT))
					{
						Dimension::unitsSelected.clear();
					}

					Utilities::Vector3D win_coord;
					for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
					{
						win_coord = Dimension::GetUnitWindowPos(*it);
						if (win_coord.x >= pGame->start_drag_x && win_coord.x <= pGame->end_drag_x && win_coord.y >= pGame->start_drag_y && win_coord.y <= pGame->end_drag_y && Dimension::UnitIsVisible(*it, Dimension::currentPlayerView))
						{
							AddSelectedUnit(*it);
						}
					}						
				}
				else
				{
					Dimension::GetApproximateMapPosOfClick((*event).button.x, (*event).button.y, map_x, map_y);							
					if (!pGame->input->GetKeyState(SDLK_LSHIFT))
					{
						Dimension::unitsSelected.clear();
					}
					unit = Dimension::GetUnitClicked((*event).button.x, (*event).button.y, map_x, map_y);
					if (unit)
					{
						static Dimension::Unit* last_unit_clicked = NULL;
						static Uint32 time_of_last_click = 0;

						if (unit == last_unit_clicked && (SDL_GetTicks() - time_of_last_click) / 1000.0 < 0.33)
						{
							Utilities::Vector3D win_coord;
							vector<Dimension::Unit*>& units = Game::Dimension::currentPlayerView->vUnits;
							for (vector<Dimension::Unit*>::iterator it = units.begin(); it != units.end(); it++)
							{
								win_coord = Dimension::GetUnitWindowPos(*it);
								if (((win_coord.x >= 0 && win_coord.x <= Window::windowWidth && win_coord.y >= 0 && win_coord.y <= Window::windowHeight) || pGame->input->GetKeyState(SDLK_LSHIFT)) && (*it)->type == unit->type)
								{
									AddSelectedUnit(*it);
								}
							}
						}
						else
						{
							AddSelectedUnit(unit);
						}

						last_unit_clicked = unit;
						time_of_last_click = SDL_GetTicks();
					}
				}
			}

		}

		void GameInput::MouseUpRight(SDL_Event* event,Window::GUI::TranslatedMouse* translatedMouse)
		{
			int map_x = 0, map_y = 0;
			int ter_x = 0, ter_y = 0;
			bool clicked_on_unit = false, clicked_on_ground = false, shift_pressed = pGame->input->GetKeyState(SDLK_LSHIFT);
			void* arg = NULL;
			AI::UnitAction action = AI::ACTION_NONE;
			Dimension::Unit *unit = NULL;

			if (pGame->build_type)
			{
				pGame->build_type = NULL;
				return;
			}

			if (Dimension::unitsSelected.size())
			{
				Dimension::GetApproximateMapPosOfClick((*event).button.x, (*event).button.y, map_x, map_y);
				unit = Dimension::GetUnitClicked((*event).button.x, (*event).button.y, map_x, map_y);

				if (unit)
				{
					clicked_on_unit = true;
					if (Dimension::GetCurrentPlayer()->states[unit->owner->index] != Dimension::PLAYER_STATE_ALLY)
					{
						if (!pGame->input->GetKeyState(SDLK_LALT))
						{
							action = AI::ACTION_ATTACK;
						}
						else
						{
							action = AI::ACTION_MOVE_ATTACK_UNIT;
						}
					}
					else
					{
						bool canRepair = false;
						for (unsigned int i = 0; i < Dimension::unitsSelected.at(0)->type->canBuild.size(); i++)
						{
							if (Dimension::unitsSelected.at(0)->type->canBuild.at(i) == unit->type)
							{
								canRepair = true;
								break;
							}
						}
						if (!pGame->input->GetKeyState(SDLK_LSHIFT) && unit->health < unit->type->maxHealth && canRepair)
						{
							action = AI::ACTION_BUILD;
						}
						else
						{
							if (!unit->type->isMobile)
							{
								if (unit->rallypoint != NULL)
								{
									delete unit->rallypoint;
									unit->rallypoint = NULL;
								}
								return;
							}
							else
								action = AI::ACTION_FOLLOW;
						}
					}
				}
				else
				{
					if (Dimension::GetTerrainPosClicked((*event).button.x, (*event).button.y, map_x, map_y, ter_x, ter_y))
					{
						clicked_on_ground = true;
						pGame->goto_x = ter_x;
						pGame->goto_y = ter_y;
						pGame->goto_time = SDL_GetTicks();
						if (!pGame->input->GetKeyState(SDLK_LALT))
						{
							action = AI::ACTION_GOTO;
						}
						else
						{
							action = AI::ACTION_MOVE_ATTACK;
						}
					}
				}

				if (Dimension::unitsSelected.size() == 1)
				{
					if (clicked_on_unit)
					{
						AI::CommandUnit(Dimension::unitsSelected.at(0), unit, action, arg, shift_pressed, false);
					}
					else if (clicked_on_ground)
					{
						AI::CommandUnit(Dimension::unitsSelected.at(0), ter_x, ter_y, action, arg, shift_pressed, false);
					}
				}
				else
				{
					if (clicked_on_unit)
					{
						AI::CommandUnits(Dimension::unitsSelected, unit, action, arg, shift_pressed, false);
					}
					else if (clicked_on_ground)
					{
						AI::CommandUnits(Dimension::unitsSelected, ter_x, ter_y, action, arg, shift_pressed, false);
					}
				}
			}
		}

		void GameInput::SetTypeToBuild(unsigned int num)
		{
			if (Dimension::unitsSelected.size())
			{
				Dimension::Unit* unit = Dimension::unitsSelected.at(0);
				if (unit->type->canBuild.size() >= num)
				{
					pGame->build_type = unit->type->canBuild.at(num-1);
					if (!unit->type->isMobile)
					{
						if (Dimension::unitsSelected.size() == 1)
						{
							AI::CommandUnit(Dimension::unitsSelected.at(0), 0, 0, AI::ACTION_BUILD, (void*) pGame->build_type, true, false);
						}
						else
						{
							AI::CommandUnits(Dimension::unitsSelected, 0, 0, AI::ACTION_BUILD, (void*) pGame->build_type, true, false);
						}
						pGame->build_type = NULL;
					}
				}
			}
		}

		void GameInput::SetGroup(unsigned int num)
		{
			Dimension::unitGroups[num] = Dimension::unitsSelected;
		}

		void GameInput::RecallGroup(unsigned int num)
		{
			Dimension::unitsSelected = Dimension::unitGroups[num];
		}

		void GameInput::AddGroup(unsigned int num)
		{
			for (vector<Dimension::Unit*>::iterator it = Dimension::unitGroups[num].begin(); it != Dimension::unitGroups[num].end(); it++)
			{
				AddSelectedUnit(*it);
			}
		}

		int GameInput::HandleEvent(Window::GUI::EventType evtType, SDL_Event* event, Window::GUI::TranslatedMouse* translatedMouse)
		{
			/*
			*  Keyboard-event: key down - updates key status through
			*  (InputController) input
			*/
			switch(evtType)
			{
				case KB_DOWN:
				{
					switch ((*event).key.keysym.sym)
					{
						case SDLK_ESCAPE:
						{
							if (pGame->build_type == NULL)
							{
								pGame->go = false;
								pGame->returnValue = INGAMEMENU;
							}
							else
								pGame->build_type = NULL;

							break;
						}
						case SDLK_PRINT:
						{
							Window::MakeScreenshot();
							break;
						}
						case SDLK_TAB:
						{
/*							console << "This feature has been disabled." << Console::nl;
							break; */

							int num = Dimension::currentPlayerView->index;

							// switch player view
							if (!pGame->input->GetKeyState(SDLK_LSHIFT))
							{
								num++;
								if (num >= (int) Dimension::pWorld->vPlayers.size())
									num = 0;
							}
							else
							{
								num++;
								if (num < 0)
									num = Dimension::pWorld->vPlayers.size()-1;
							}
							Dimension::currentPlayerView = Dimension::pWorld->vPlayers.at(num);
							
//							Window::SetTitle(Dimension::currentPlayerView->name);
							break;
						}
						case SDLK_DELETE:
						{
							if(Dimension::GetCurrentPlayer()->vUnits.size() > 0)
							{
								for (unsigned i = 0; i < Dimension::unitsSelected.size(); i++)
								{
									Dimension::Unit* p = Dimension::unitsSelected.at(i);

									if (p->owner == Dimension::currentPlayer)
									{

										if (Networking::isNetworked)
											Networking::PrepareDamaging(p, p->type->maxHealth+1);
										else
											Dimension::KillUnit(p);
									}
								}
							}
							break;
						}
						case SDLK_1:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(0);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(0);
								else
									RecallGroup(0);
							break;
						}
						case SDLK_2:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(1);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(1);
								else
									RecallGroup(1);
							break;
						}
						case SDLK_3:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(2);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(2);
								else
									RecallGroup(2);
							break;
						}
						case SDLK_4:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(3);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(3);
								else
									RecallGroup(3);
							break;
						}
						case SDLK_5:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(4);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(4);
								else
									RecallGroup(4);
							break;
						}
						case SDLK_6:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(5);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(5);
								else
									RecallGroup(5);
							break;
						}
						case SDLK_7:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(6);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(6);
								else
									RecallGroup(6);
							break;
						}
						case SDLK_8:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(7);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(7);
								else
									RecallGroup(7);
							break;
						}
						case SDLK_9:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(8);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(8);
								else
									RecallGroup(8);
							break;
						}
						case SDLK_0:
						{
							if (pGame->input->GetKeyState(SDLK_LCTRL))
								SetGroup(9);
							else
								if (pGame->input->GetKeyState(SDLK_LSHIFT))
									AddGroup(9);
								else
									RecallGroup(9);
							break;
						}
						case SDLK_F1:
						{
							pGame->pMainGame->SetVisible(pGame->consoleID, true);
							break;
						}
						case SDLK_F2:
						{
							//Testing particle system
							//FX::pParticleSystems->InitEffect(160.0f, 40.0f, 1.0f, 1.0f, FX::PARTICLE_SPHERICAL_EXPLOSION);
							//console << Console::imp << "Particle System Start" << Console::nl;
							break;
						}
						case SDLK_BREAK:
						{
							if (Networking::isNetworked)
								console << "This feature has been disabled." << Console::nl;
							else
							{
								pGame->pause = !pGame->pause;
							}
							break;
						}
						case SDLK_s:
						{
							if (Networking::isNetworked)
							{
								Networking::PrepareSell(Dimension::GetCurrentPlayer(), 100);
							}
							else
							{
								Dimension::SellPower(Dimension::GetCurrentPlayer(), 100);
							}
							break;
						}
						case SDLK_w:
						{
							Dimension::SaveGame("save.xml");
							break;
						}
#ifdef FRAGREPORT
						case SDLK_f:
						{
							WriteFragmentationReport();
							break;
						}
#endif
						case SDLK_g:
						{
							if (graphicsLoaded)
							{
								noGraphics = !noGraphics;
							}
							break;
						}
						default:
						{
							pGame->input->SetKeyState((*event).key.keysym.sym, true);
							break;
						}
					}
					break;
				}
				case KB_UP:
				{
					switch ((*event).key.keysym.sym)
					{
						case SDLK_F1:
						{
							pGame->pMainGame->SetVisible(pGame->consoleID, false);
							break;
						}
						default:
							break;
					}
					/*
					*  Keyboard-event: key up - updates/resets key status through
					*  (InputController) input
					*/
					pGame->input->SetKeyState((*event).key.keysym.sym, false);
					break;
				}
				case MOUSE_MOVE:
				{
					/*
					*  Mouse motion - stores the current mouse location
					*/
					int map_x = 0, map_y = 0;
					pGame->end_drag_x = (*event).motion.x;
					pGame->end_drag_y = (*event).motion.y;
					end_fx = translatedMouse->x;
					end_fy = translatedMouse->y;
					if(pGame->build_type)
					{
						Dimension::GetApproximateMapPosOfClick((*event).motion.x, (*event).motion.y, map_x, map_y);
						Dimension::GetTerrainPosClicked((*event).button.x, (*event).button.y, map_x, map_y, pGame->build_x, pGame->build_y);
					}
					break;
				}
				case MOUSE_SCROLL:
				{
					/*
					*  Mouse event: middle mouse button (a.k.a. wheel/scroller) control of the camera
					*/
					if ((*event).button.button == SDL_BUTTON_WHEELUP)
					{
						pGame->worldCamera->Zoom(-60.0f * time_since_last_frame);
					}
					else if ((*event).button.button == SDL_BUTTON_WHEELDOWN)
					{
						pGame->worldCamera->Zoom(60.0f * time_since_last_frame);
					}
					break;
				}
				case MOUSE_DOWN:
				{
					switch((*event).button.button)
					{
						case SDL_BUTTON_MMASK:
						{
							// Follow the mouse-position
							break;
						}
						case SDL_BUTTON_LEFT:
						{
#ifdef MAC
							if (pGame->input->GetKeyState(SDLK_LCTRL))
							{
								MouseDownRight(event, translatedMouse);
								break;
							}
#endif
							MouseDownLeft(event, translatedMouse);
							break;
						}
						/*
						*  Unit control code
					 */

						case SDL_BUTTON_RIGHT:
						{
							MouseDownRight(event, translatedMouse);
							break;
						}
					}
					break;
				}
				case MOUSE_UP:
				{
					switch((*event).button.button)
					{
					case SDL_BUTTON_LEFT:
						{
#ifdef MAC
							if (pGame->input->GetKeyState(SDLK_LCTRL))
							{
								MouseUpRight(event, translatedMouse);
								break;
							}
#endif
							MouseUpLeft(event, translatedMouse);
							break;
						}
					case SDL_BUTTON_RIGHT:
						{
							MouseUpRight(event, translatedMouse);
							break;
						}
					}
					break;
				}
				default:
				{
					break;
				}
				return SUCCESS;
			}	
			//End

			return SUCCESS;
		}

		GameTopBar::GameTopBar()
		{
			bgColor[0] = 0.0f;
			bgColor[1] = 0.0f;
			bgColor[2] = 0.7f;
			bgColor[3] = 0.6f;

			textures[0] = Utilities::LoadTexture("textures/gui/gui_top_tile.png");
			textures[1] = Utilities::LoadTexture("textures/gui/gui_top_left.png");
			textures[2] = Utilities::LoadTexture("textures/gui/gui_top_right.png");
		}

		void GameTopBar::PaintBackground()
		{
			float size = 2.0f;
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[0]);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(size, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(this->w - size, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(this->w - size, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(size, 1.0f);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, textures[1]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(size, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(size, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, size);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, textures[2]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(this->w - size, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(this->w, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(this->w, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(this->w - size, size);
			glEnd();

			glDisable(GL_TEXTURE_2D);
		}

		void test(Window::GUI::EventType evType, void* tag)
		{
			cout << "CLICKED!" << endl;
		}

		GamePlayBar::GamePlayBar(GameWindow *ref, SDL_Surface *img)
		{
			bgColor[0] = 0.0f;
			bgColor[1] = 0.0f;
			bgColor[2] = 0.7f;
			bgColor[3] = 0.6f;
			pGame = ref;
			if (!Game::Rules::noGraphics)
				tmap = CreateMap(img, &Dimension::HeightMipmaps[0][0], Dimension::pWorld->width, Dimension::pWorld->height, 256,256);
			buildSelected = false;
		}

		GamePlayBar::~GamePlayBar()
		{
			glDeleteTextures(3, (GLuint*)texturesTop);
			glDeleteTextures(1, (GLuint*)&textureBack);

			delete pSelected;
			delete pActions;
			delete pMap;
			for (map<Dimension::UnitType*,UnitBuild*>::iterator iter = GUI_Build.begin(); iter != GUI_Build.end(); iter++)
			{
				delete (*iter).second;
			}
			if (!Game::Rules::noGraphics)
				glDeleteTextures(1, &tmap);
		}

		void GamePlayBar::PaintTop()
		{
			float size = 0.2f;
			float sizehalf = size / 2.0f;

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texturesTop[1]);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f,0.0f);
				glVertex2f(0.0f,0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(location1 - sizehalf, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location1 - sizehalf, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, size);

				//Second
				glTexCoord2f(0.0f,0.0f);
				glVertex2f(location1 + sizehalf,0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(location2 - sizehalf, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location2 - sizehalf, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location1 + sizehalf, size);

				//Third
				glTexCoord2f(0.0f,0.0f);
				glVertex2f(location2 + sizehalf,0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(this->w, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(this->w, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location2 + sizehalf, size);
			glEnd();
			
			glBindTexture(GL_TEXTURE_2D, texturesTop[2]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(location1 - sizehalf, size);

				glTexCoord2f(1.0f,0.0f);
				glVertex2f(location1 + sizehalf, size);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location1 + sizehalf, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location1 - sizehalf, 1.0f);

				//Secondary
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(location2 - sizehalf, size);

				glTexCoord2f(1.0f,0.0f);
				glVertex2f(location2 + sizehalf, size);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location2 + sizehalf, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location2 - sizehalf, 1.0f);
			glEnd();

			
			glBindTexture(GL_TEXTURE_2D, texturesTop[0]);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(location1 - sizehalf,0.0f);

				glTexCoord2f(1.0f,0.0f);
				glVertex2f(location1 + sizehalf, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location1 + sizehalf, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location1 - sizehalf, size);
				
				//Second
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(location2 - sizehalf,0.0f);

				glTexCoord2f(1.0f,0.0f);
				glVertex2f(location2 + sizehalf, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(location2 + sizehalf, size);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(location2 - sizehalf, size);
			glEnd();

			glDisable(GL_TEXTURE_2D);
		}

		void GamePlayBar::PaintBackground()
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureBack);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f,0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(this->w,0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(this->w, this->h);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f,this->h);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		}

		void GamePlayBar::SwitchSelected(Dimension::Unit* selected)
		{
			this->pUnit = selected;
			if(selected == NULL)
			{
				Window::GUI::PanelWidget obj;
				obj.pWidget = pSelected;
				SetElement(build_panel, obj, typeWidget);
				pActions->SetSelected(NULL);
				buildSelected = false;
			}
			else if(selected->owner == Dimension::currentPlayerView)
			{	
				Window::GUI::PanelWidget obj;
				obj.pWidget = pSelected;
				SetElement(build_panel, obj, typeWidget);
				pActions->SetSelected(selected);
				buildSelected = false;
				if(Dimension::unitsSelected.size() == 1)
					SwitchBuild();
			}
			else
			{
				pActions->SetSelected(selected);
				Window::GUI::PanelWidget obj;
				obj.pWidget = pSelected;
				SetElement(build_panel, obj, typeWidget);
				buildSelected = false;
			}
		}

		void GamePlayBar::SwitchBuild()
		{
			if(buildSelected == false)
			{
				map<Dimension::UnitType*, UnitBuild*>::iterator match = GUI_Build.find(pUnit->type);
				if(match != GUI_Build.end())
				{
					Window::GUI::PanelWidget obj;
					obj.pPanel = (*match).second;
					SetElement(build_panel, obj, typePanel);
					(*match).second->SetUnit(pUnit);
					buildSelected = true;
				}
			}
			else
			{
				buildSelected = false;
				Window::GUI::PanelWidget obj;
				obj.pWidget = pSelected;
				SetElement(build_panel, obj, typeWidget);
			}
		}

		void GamePlayBar::init()
		{
			GLuint nopic = Utilities::LoadTexture("textures/nopic.png");
			texturesTop[0] = Utilities::LoadTexture("textures/gui/gui_player_intersect.png");
			texturesTop[1] = Utilities::LoadTexture("textures/gui/gui_player_tile_x.png");
			texturesTop[2] = Utilities::LoadTexture("textures/gui/gui_player_tile_y.png");

			textureBack = Utilities::LoadTexture("textures/gui/gui_player_unitbar.png");

			pSelected = new UnitSelected(pGame, this, nopic);
			pActions = new UnitActions(nopic, pGame);
			pMap = new GameMap(tmap);

			build_panel = Add(pSelected);
			SetConstraint(build_panel, location1 = 0.3f * this->w, 0.05f, this->w - (this->w * 0.3f) - 1.0f, 0.95);
			info_panel = Add(pActions);
			SetConstraint(info_panel, 0.0f, 0.05f, 0.3f * this->w, 0.95f);

			int pic = Add(pMap);
			SetConstraint(pic, location2 = this->w - 0.95f, 0.05f, 0.95f, 0.95f);

			pActions->InitLayout();

			vector<Game::Dimension::UnitType*> buildingUnits = Game::Dimension::modelLoader.GetBuildUnits();
			for(vector<Game::Dimension::UnitType*>::iterator iter = buildingUnits.begin(); iter != buildingUnits.end(); iter++)
			{
				Game::Dimension::UnitType* unitTyp = (*iter);
				UnitBuild* unitBuilder = new UnitBuild(nopic, pGame);
				Window::GUI::PanelWidget obj;
				obj.pPanel = unitBuilder;
				CopyCoordianteSystem(build_panel, obj, typePanel); //Temporary Solution
				unitBuilder->SetUnitType(unitTyp);
				GUI_Build[unitTyp] = unitBuilder;
			}
		}

		UnitActions::UnitActions(GLuint nopic, GameWindow *ref)
		{
			MoveAttack = new Window::GUI::PicButton();
			Stop = new Window::GUI::PicButton();
			Build = new Window::GUI::PicButton();
			UnitPic = new Window::GUI::Picture();
			Health = new Window::GUI::Progressbar();

			Health->SetColor(0.0f, 1.0f, 0.0f, 0.5f);
			Health->SetShade(true, 0.6f);

			UnitActions::InternalHandler *intern = new UnitActions::InternalHandler();
			intern->parent = this;
			intern->ActionID = 1;

			MoveAttack->AttachHandler(&ActionSelected);
			MoveAttack->SetTag((void*)intern);
			MoveAttack->SetTooltip("Move - Attack");
			MoveAttack->SetPicture(nopic);

			intern = new UnitActions::InternalHandler();
			intern->parent = this;
			intern->ActionID = 2;

			Stop->AttachHandler(&ActionSelected);
			Stop->SetTag((void*)intern);
			Stop->SetTooltip("Stop");
			Stop->SetPicture(Utilities::LoadTexture("textures/symbol_stop.png"));

			intern = new UnitActions::InternalHandler();
			intern->parent = this;
			intern->ActionID = 3;

			Build->AttachHandler(&ActionSelected);
			Build->SetTag((void*)intern);
			Build->SetTooltip("Build");
			Build->SetPicture(Utilities::LoadTexture("textures/symbol_build.png"));

			this->nopic = nopic;
			this->pGame = ref;
			this->pUnit = NULL;
		}

		UnitActions::~UnitActions()
		{
			delete MoveAttack;
			delete Stop;
			delete Build;
			delete UnitPic;
		}

		void UnitActions::ActionSelected(Window::GUI::EventType evtType, void* tag)
		{
			UnitActions::InternalHandler *obj = (UnitActions::InternalHandler*)tag;
			switch(obj->ActionID)
			{
				case 1:
				{
					cout << "Unit Action: MOVE ATTACK" << endl;
					break;
				}
				case 2:
				{
					cout << "Unit Action: STOP" << endl;
					break;
				}
				case 3:
				{
					obj->parent->pGame->pPlayBar->SwitchBuild();
					cout << "Unit Action: SELECT BUILD MENU" << endl;
					break;
				}
				default:
				{
					cout << "ACTION: " << obj->ActionID << endl;
					break;
				}
			}
		}

		void UnitActions::SetSelected(Dimension::Unit* pUnit)
		{
			this->pUnit = pUnit;
			if(pUnit == NULL)
			{
				UnitPic->SetPicture(nopic);
				SetVisible(idBuild, false);
				SetVisible(idMoveAttack, false);
				SetVisible(idStop, false);
				SetVisible(idHealth, false);
			}
			else
			{
				if(pUnit->type->Symbol == 0)
				{
					UnitPic->SetPicture(nopic);
				}
				else
				{
					UnitPic->SetPicture(pUnit->type->Symbol);
				}

				Health->SetMax((float)pUnit->type->maxHealth);
				Health->SetValue(pUnit->health);
				SetVisible(idHealth, true);

				if(pUnit->owner->type == Dimension::PLAYER_TYPE_HUMAN)
				{
					if(pUnit->type->canBuild.size() > 0)
					{
						SetVisible(idBuild, true);
						SetVisible(idMoveAttack, true);
						SetVisible(idStop, true);
					}
					else
					{	
						SetVisible(idBuild, false);
						SetVisible(idMoveAttack, true);
						SetVisible(idStop, true);
					}
				}
				else
				{	
					SetVisible(idBuild, false);
					SetVisible(idMoveAttack, false);
					SetVisible(idStop, false);
				}
			}
		}

		void UnitActions::InitLayout()
		{
			SetConstraint(idHealth = Add(Health), 0.0f, 0.9f, 1.0f, 0.1f);
			SetConstraint(Add(UnitPic), 0.0f, 0.0f, 1.0f, 1.0f);
			SetConstraint(idMoveAttack = Add(MoveAttack), this->w - 1.0f, 0.0f, 0.5f, 0.5f);
			SetConstraint(idStop = Add(Stop), this->w - 0.5f, 0.0f, 0.5f, 0.5f);
			SetConstraint(idBuild = Add(Build), this->w - 0.5f, 0.5f, 0.5f, 0.5f);
		}

		void UnitActions::Update()
		{
			if(pUnit != NULL)
			{
				Health->SetValue(pUnit->health);
			}
		}

		UnitBuild::UnitBuild(GLuint pic,GameWindow* pRef)
		{
			this->nopic = pic;
			this->pGame = pRef;
		}

		void UnitBuild::SetUnit(Dimension::Unit *pUnit)
		{
			this->pUnit = pUnit;
		}

		void UnitBuild::SetUnitType(Dimension::UnitType *pUnitType)
		{
			this->pUnitType = pUnitType;
			SetLayout();
		}

		void UnitBuild::BuildSelected(Window::GUI::EventType evtType,void* pTag)
		{
			if(pTag != NULL)
			{
				UnitBuild::InternalHandler* obj = (UnitBuild::InternalHandler*)pTag;
				cout << "Build ID: " << obj->buildID << endl;
				if (obj->parent->pUnit->type->canBuild.size() <= (unsigned) obj->buildID)
				{
					Game::Dimension::UnitType* research_type = obj->parent->pUnit->type->canResearch.at(obj->buildID - obj->parent->pUnit->type->canBuild.size());
					if (Dimension::unitsSelected.size() == 1)
					{
						AI::CommandUnit(obj->parent->pGame->buildingUnit, 0, 0, AI::ACTION_RESEARCH, (void*)research_type, true, false);
					}
					else
					{
						AI::CommandUnits(Dimension::unitsSelected, 0, 0, AI::ACTION_RESEARCH, (void*)research_type, true, false);
					}
				}
				else
				{
					obj->parent->pGame->build_type = obj->parent->pUnit->type->canBuild.at(obj->buildID);
					if (!obj->parent->pGame->buildingUnit->type->isMobile)
					{
						if (Dimension::unitsSelected.size() == 1)
						{
							AI::CommandUnit(obj->parent->pGame->buildingUnit, 0, 0, AI::ACTION_BUILD, (void*)obj->parent->pGame->build_type, true, false);
						}
						else
						{
							AI::CommandUnits(Dimension::unitsSelected, 0, 0, AI::ACTION_BUILD, (void*)obj->parent->pGame->build_type, true, false);
						}
						obj->parent->pGame->build_type = NULL;
					}
				}
			}
		}

		UnitSelected::UnitSelected(GameWindow *pWinRef, GamePlayBar *pPlayBar, GLuint nopic)
		{
			pGame = pWinRef;
			pPlay = pPlayBar;
			mouseOver = -1;
			txBox = 1.0f;
			tyBox = 1.0f;
			wBox = 1.0f;
			hBox = 1.0f;
			colCount = 0;
			this->nopic = nopic;
		}

		void UnitSelected::Update()
		{
			int unitCount = Dimension::unitsSelected.size();
			if(unitCount == 0)
			{
				wBox = 1.0f;
				hBox = 1.0f;
				txBox = 1.0f;
				tyBox = 1.0f;
			}
			else
			{
				float tmpW = this->w / unitCount;
				if(tmpW < 0.33f)
				{
					//3 rows and possible wrapping
					wBox = 1.0f / 3.0f;
					hBox = 1.0f / 3.0f;
					colCount = (int)(this->w / (1.0f / 3.0f));

					if(unitCount > (colCount * 3))
					{ //wrapping
						colCount = (unitCount / 3) + (unitCount % 3);
						txBox = this->w / (colCount + 0.5f);
					}
					else
					{
						txBox = 1.0f / 3.0f;
					}

					tyBox = 1.0f / 3.0f;
				}
				else if(tmpW < 0.5f)
				{
					//2 rows
					wBox = 0.5f;
					hBox = 0.5f;

					txBox = 0.5f;
					tyBox = 0.5f;
				}
				else if(tmpW < 1.0f)
				{
					//1 row
					wBox = tmpW;
					hBox = wBox;
					txBox = tmpW;
					tyBox = tmpW;
					colCount = Dimension::unitsSelected.size();
				}
				else
				{
					//1 row
					wBox = 1.0f;
					hBox = 1.0f;
					txBox = 1.0f;
					tyBox = 1.0f;
					colCount = Dimension::unitsSelected.size();
				}
			}
		}

		int UnitSelected::HandleEvent(Window::GUI::EventType evtType, SDL_Event *evt, Window::GUI::TranslatedMouse *mouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_MOVE:
				{
					int x = (int)(mouseCoord->x / txBox);
					int y = (int)(mouseCoord->y / tyBox);
					if(x >= colCount)
					{
						mouseOver = -1;
					}
					else
					{
						mouseOver = colCount * y + x;
					}
					break;
				}
				case MOUSE_OUT:
				{
					mouseOver = -1;
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		int UnitSelected::Paint()
		{
			float cx = 0.0f;
			float cy = 0.0f;
			int colc = 0;
			int i = 0;

			glEnable(GL_TEXTURE_2D);
			vector<Dimension::Unit*>::iterator iter =  Dimension::unitsSelected.begin();
			while(iter != Dimension::unitsSelected.end())
			{
				if(colc == this->colCount)
				{
					colc = 0;
					cy += tyBox;
					cx = 0;
				}
				//Draw
				Dimension::Unit* pCurUnit = (*iter);

				if(pCurUnit->type->Symbol == 0)
					glBindTexture(GL_TEXTURE_2D, nopic);
				else
					glBindTexture(GL_TEXTURE_2D, pCurUnit->type->Symbol);

				glBegin(GL_QUADS);
					if(mouseOver == i)
					{
						glColor4f(1.0f, 1.0f,1.0f, 0.5f);
					}
					else
					{
						glColor4f(1.0f, 1.0f,1.0f, 1.0f);
					}

					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(cx, cy);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(cx + wBox, cy);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(cx + wBox, cy + hBox);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(cx, cy + hBox);
				glEnd();
				iter++;
				colc++;
				i++;
				cx += txBox;
			}
			glDisable(GL_TEXTURE_2D);
			return SUCCESS;
		}

		int BuildButton::HandleEvent(Window::GUI::EventType evtType, SDL_Event *evt, Window::GUI::TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_UP:
				{
					if(mouseDown == true)
					{
						if(MouseCoord->x > 0 && MouseCoord->y > 0 && MouseCoord->x <= this->w && MouseCoord->y <= this->h)
						{
							if(onEvent)
							{
								if(evtType == MOUSE_UP)
								{
									(*onEvent)(evtType, tag);
								}
							}
						}
						mouseDown = false;
					}
					break;
				}
				case MOUSE_MOVE:
				{
					painttooltip = true;
					mouseOver = true;
					mx = MouseCoord->x;
					my = MouseCoord->y;
					break;
				}
				case MOUSE_DOWN:
				{
					mouseDown = true;
					fadeValue = 0.7f;
					break;
				}
				case MOUSE_OUT:
				{
					mouseOver = false;
					mouseDown = false;
					painttooltip = false;
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		int BuildButton::Paint()
		{
			this->pBuildPanel->GetBuildPercentage(this->id, this->value, this->text);

			PaintBackground();
			PaintPicture();
			PaintProgressbar();

			if(this->text != "")
			{
				glEnable(GL_TEXTURE_2D);
				Window::GUI::FontCache::RenderedText RenderedInfo;
				Window::GUI::Fonts.SetFontType(this->type);
				Window::GUI::Fonts.RenderText(this->text , RenderedInfo, xUnit);
				float xPos = floor((w - RenderedInfo.w - 0.05f) / xUnit + 0.5f) * xUnit;
				float yPos = floor((1.0f - RenderedInfo.h - 0.05f) / yUnit + 0.5f) * yUnit;

				PaintLabel(RenderedInfo, xPos, yPos);
				glDisable(GL_TEXTURE_2D);
			}

			PaintOverlay();
			return SUCCESS;
		}

		void UnitBuild::GetBuildPercentage(int id, float& value, string& lbl)
		{
			if (pUnit->pMovementData->action.action == AI::ACTION_BUILD)
			{
				if (pUnit->pMovementData->action.goal.unit != NULL)
				{
					Dimension::Unit* target = pUnit->pMovementData->action.goal.unit;
					if((unsigned) id < pUnit->type->canBuild.size() && pUnit->type->canBuild.at(id) == target->type)
					{
						if (!target->isCompleted)
						{
							stringstream status;
							int size = pUnit->actionQueue.size();

							if (size > 100)
							{
								size = 100;
							}

							status << (int)target->completeness << "% / " << size;
							lbl = status.str().c_str();
							value = target->completeness / 100.0f;
							return;
						}
					}
				}
			}
			else if (pUnit->pMovementData->action.action == AI::ACTION_RESEARCH)
			{
				Dimension::UnitType* research_type = (Dimension::UnitType*) pUnit->pMovementData->action.arg;
				if((unsigned) id >= pUnit->type->canBuild.size() && pUnit->type->canResearch.at(id - pUnit->type->canBuild.size()) == research_type)
				{
					stringstream status;
					int size = pUnit->actionQueue.size();

					if (size > 100)
					{
						size = 100;
					}

					status << (int)pUnit->action_completeness << "% / " << size;
					lbl = status.str().c_str();
					value = pUnit->action_completeness / 100.0f;
					return;
				}
			}
			lbl = "";
			value = 0.0f;
		}

		void UnitBuild::SetLayout()
		{
			int count = this->pUnitType->canBuild.size() + this->pUnitType->canResearch.size();
			//			int maxw = (int)floor(this->w); <--- unused

			float ow = 0.0f;
			float oh = 0.0f;

			int nxt = 0;

			if(this->w > (float)count) //H == 1
			{
				ow = 1.0f;
				oh = 1.0f;
				nxt = count;
			}
			else
			{
				ow = this->w / (float)count;
				if(ow < 0.5f)
				{
					ow = 0.5f;
					nxt = (int)(this->w / 0.5f);
				}
				else
				{
					nxt = count;
				}

				oh = ow;
			}
			float cx = 0.0f;
			float cy = 0.0f;
			int c = 0;
			for(vector<Dimension::UnitType*>::iterator iter = this->pUnitType->canBuild.begin(); iter != this->pUnitType->canBuild.end(); iter++, cx += ow, c++)
			{
				if(c == nxt)
				{
					cy += oh;
					cx = 0.0f;
				}
				BuildButton *picbtn = new BuildButton(this, c);
				SetConstraint(Add(picbtn), cx, cy, ow, oh);

				UnitBuild::InternalHandler *pTag = new UnitBuild::InternalHandler();
				pTag->parent = this;
				pTag->buildID = c;
				picbtn->SetTag((void*)pTag);
				picbtn->AttachHandler(&BuildSelected);
				picbtn->SetColor(1.0f, 0.75f, 0.0f, 0.25f);
				if((*iter)->Symbol == 0)
					picbtn->SetPicture(this->nopic);
				else
					picbtn->SetPicture((*iter)->Symbol);

				picbtn->SetTooltip((*iter)->name);
				objects.push_back(picbtn);
			}
			for(vector<Dimension::UnitType*>::iterator iter = this->pUnitType->canResearch.begin(); iter != this->pUnitType->canResearch.end(); iter++, cx += ow, c++)
			{
				if(c == nxt)
				{
					cy += oh;
					cx = 0.0f;
				}
				BuildButton *picbtn = new BuildButton(this, c);
				SetConstraint(Add(picbtn), cx, cy, ow, oh);

				UnitBuild::InternalHandler *pTag = new UnitBuild::InternalHandler();
				pTag->parent = this;
				pTag->buildID = c;
				picbtn->SetTag((void*)pTag);
				picbtn->AttachHandler(&BuildSelected);
				picbtn->SetColor(1.0f, 0.0f, 0.75f, 0.25f);
				if((*iter)->Symbol == 0)
					picbtn->SetPicture(this->nopic);
				else
					picbtn->SetPicture((*iter)->Symbol);

				picbtn->SetTooltip("Research " + (const string) (*iter)->name);
				objects.push_back(picbtn);
			}
		}

		UnitBuild::~UnitBuild()
		{
			//DeAlloc
			for(vector<BuildButton*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
			{
				delete (*iter);
			}
		}

		GameMap::GameMap(GLuint themap)
		{
			map = themap;
			mapoverlay = 0;
			mouseDown = false;
			frames = 7;
		}

		int GameMap::HandleEvent(Window::GUI::EventType evtType,SDL_Event* evt,Window::GUI::TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_DOWN:
				{
					mouseDown = true;
					break;
				}
				case MOUSE_MOVE:
				{
					if(mouseDown == true)
					{
						float terrainX = MouseCoord->x * (Dimension::pWorld->width - 1);
						float terrainY = MouseCoord->y * (Dimension::pWorld->height - 1);
						GameWindow::Instance()->GetCamera()->SetFocus(terrainX, terrainY);
					}
					break;
				}
				case MOUSE_UP:
				{
					mouseDown = false;
					float terrainX = MouseCoord->x * (Dimension::pWorld->width - 1);
					float terrainY = MouseCoord->y * (Dimension::pWorld->height - 1);
					GameWindow::Instance()->GetCamera()->SetFocus(terrainX, terrainY);
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		void GameMap::Update()
		{
			int closest_value_w = 1;
			int closest_value_h = 1;

			int pixel_width = (int)(1.0f / (this->w * this->xUnit));
			int pixel_height = (int)(1.0f / (this->h * this->xUnit));

			while(closest_value_w <= pixel_width)
				closest_value_w <<= 1;

			while(closest_value_h <= pixel_height)
				closest_value_h <<= 1;

			int worldWidth = Dimension::pWorld->width - 1;
			int worldHeight = Dimension::pWorld->height - 1;

			int xIncrement = worldWidth / closest_value_w;
			if(xIncrement == 0)
				xIncrement = 1;

			int yIncrement = worldWidth / closest_value_w;
			if(yIncrement == 0)
				yIncrement = 1;

			Uint8 *ptrMap = new Uint8[closest_value_w * closest_value_h * 4];
			Uint8 *ptrMapLocation = ptrMap;

			for(int y = 0, y2 = 0; y < worldHeight; y += yIncrement, y2++)
			{
				for (int x = 0, x2 = 0; x < worldWidth; x += xIncrement, x2++)
				{
					int is_seen = Dimension::SquareIsVisible(Dimension::currentPlayerView, x, y);
					int is_lighted = Dimension::SquareIsLighted(Dimension::currentPlayerView, x, y);

					Uint8 alpha;

					if(is_lighted)
						alpha = 0; //100%
					else if(is_seen)
						alpha = 63; //75%
					else
						alpha = 127; //50% genomskinnlighet

					*ptrMapLocation = 0;
					ptrMapLocation++;

					*ptrMapLocation = 0;
					ptrMapLocation++;

					*ptrMapLocation = 0;
					ptrMapLocation++;

					*ptrMapLocation = alpha;
					ptrMapLocation++;
				}	
			}

			if(mapoverlay != 0)
				glDeleteTextures(1, &mapoverlay);

			glGenTextures(1, &mapoverlay);
			glBindTexture(GL_TEXTURE_2D, mapoverlay);

			// use bilinear scaling for scaling down and trilinear scaling for scaling up
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate mipmap using GLU
			gluBuild2DMipmaps(GL_TEXTURE_2D, 4, closest_value_w, closest_value_h, GL_RGBA, GL_UNSIGNED_BYTE, ptrMap);

			delete [] ptrMap;
		}

		int GameMap::Paint()
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, this->map);
			glColor4f(1.0f,1.0f,1.0f,1.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(1.0f, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(1.0f, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, 1.0f);
			glEnd();
			glDisable(GL_TEXTURE_2D);

			glPushMatrix();

			glEnable(GL_TEXTURE_2D);
			frames++;
			if(frames == 8)
			{
				frames = 0;
				Update();
			}
			glBindTexture(GL_TEXTURE_2D, mapoverlay);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex2f(1.0f, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex2f(1.0f, 1.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, 1.0f);
			glEnd();

			for(vector<Dimension::Unit*>::iterator iter = Dimension::pWorld->vUnits.begin(); iter != Dimension::pWorld->vUnits.end(); iter++)
			{
				if(UnitIsVisible(*iter, Dimension::currentPlayerView))
				{
					Dimension::Unit* pUnit = (*iter);
					int lx = 0;
					int ly = 0;
					GetUnitUpperLeftCorner(pUnit, lx, ly);
					float x = PixelAlign((float)lx / Dimension::pWorld->width, xUnit);
					float y = PixelAlign((float)ly / Dimension::pWorld->height, xUnit);
					float w = PixelAlign((float)pUnit->type->widthOnMap / Dimension::pWorld->width, xUnit) + xUnit;
					float h = PixelAlign((float)pUnit->type->heightOnMap / Dimension::pWorld->height, xUnit) + xUnit;
					glPushMatrix();
					glTranslatef(PixelAlign(x, xUnit), PixelAlign(y, xUnit), 0);
					glBindTexture(GL_TEXTURE_2D, pUnit->owner->texture);
					glColor4f(1.0f,1.0f,1.0f, 0.5f);
					glBegin(GL_QUADS);
						glVertex2f(0.0f, 0.0f);
						glVertex2f(w, 0.0f);
						glVertex2f(w, h);
						glVertex2f(0.0f, h);
					glEnd();
					glPopMatrix();
				}
			}
			
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
			return SUCCESS;
		}
		
		NetworkJoin::NetworkJoin()
		{
			pIPAdress = new Window::GUI::Label();
			pIPText = new Window::GUI::TextBox();
			pConnect = new Window::GUI::TextButton();
			pStatus = new Window::GUI::Label();
			pQuit = new Window::GUI::TextButton();

			pMainPanel = new Window::GUI::Panel();

			this->SetPanel(pMainPanel);

			pConnect->SetText("Koppla upp");
			pConnect->AttachHandler(&Connect);
			pConnect->SetTag(this);

			pQuit->SetText("Avsluta");
			pQuit->AttachHandler(&Quit);
			pQuit->SetTag(this);

			pStatus->SetText("Awaiting input...");

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pIPAdress), 0.1, 0.1, 0.4, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pIPText), 0.5, 0.1, 0.4, Fonts.GetLineHeight(xUnit));

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pConnect), 0.3 , 0.3, 0.4, 0.1);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pStatus), 0.1 , 0.5, 0.8, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pQuit), 0.0, 0.9, 0.5, 0.1);

			pIPAdress->SetText("Host:");
			pIPText->SetText(Game::Rules::host);
			pIPText->SetMaxLen(60);

			this->sleep = true;
			this->sleepms = 16;
		}

		void NetworkJoin::PerformPreFrame()
		{
			Networking::PerformPregameNetworking();

			if(Networking::isClientConnected() == true && requestSent == false)
			{
				pStatus->SetText("Uppkopplad, joinar...awaiting confirmation");
				Networking::JoinGame();
				requestSent = true;
			}

			Networking::JoinStatus stat = Networking::GetJoinStatus();
			if(stat == Networking::JOIN_ACCEPTED)
			{
				pStatus->SetText("Join accepted.");
				if(Networking::isReadyToStart == true)
				{
					this->go = false;
					this->returnValue = GAME;
				}
			}
			else if(stat == Networking::JOIN_WAITING)
			{
				if (Game::Rules::startState == Game::Rules::NETWORKJOIN && Networking::isClientConnected() == false)
				{
					this->Connect(MOUSE_PRESS, this);
				}
			}
			else if(stat == Networking::JOIN_FAILED)
			{
				pStatus->SetText("Join failed.");
			}
			else if(stat == Networking::JOIN_CANCELLED)
			{
				pStatus->SetText("Join cancelled.");
			}
			else if(stat == Networking::JOIN_REJECTED)
			{
				pStatus->SetText("Join rejected.");
			}
		}

		void NetworkJoin::Reset()
		{
			requestSent = false;
		}

		void NetworkJoin::Connect(Window::GUI::EventType evtType, void* info)
		{
			NetworkJoin *pParent = (NetworkJoin*)info;
			pParent->Reset();
			if(Networking::SetClientConnect((char*)pParent->pIPText->GetText().c_str()) == SUCCESS)
			{
				pParent->pStatus->SetText("Attempting to connect.");
			}
			else
			{
				pParent->pStatus->SetText("Failed to resolve");
			}
		}

		void NetworkJoin::Quit(Window::GUI::EventType evtType, void* info)
		{
			NetworkJoin *pParent = (NetworkJoin*)info;
			pParent->go = false;
			pParent->returnValue = MENU;
			Networking::ShutdownNetwork();
		}

		NetworkJoinOrCreate::NetworkJoinOrCreate()
		{
			pJoin = new Window::GUI::TextButton();
			pCreate = new Window::GUI::TextButton();
			pQuit = new Window::GUI::TextButton();
			pPort = new Window::GUI::TextBox();
			pPortText = new Window::GUI::Label();
			pNick = new Window::GUI::TextBox();
			pNickText = new Window::GUI::Label();
			pMainPanel = new Window::GUI::Panel();

			this->SetPanel(pMainPanel);
			pJoin->SetText("Anslut till spel");
			pJoin->SetTag(this);
			pJoin->AttachHandler(&Join);

			pCreate->SetText("Skapa spel");
			pCreate->SetTag(this);
			pCreate->AttachHandler(&Create);

			pQuit->SetText("Avsluta");
			pQuit->SetTag(this);
			pQuit->AttachHandler(&Quit);

			pPortText->SetText("Port: ");
			pNickText->SetText("Nickname: ");

			pNick->SetMaxLen(60);

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pJoin), 0.1, 0.1, 0.4, 0.1);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pCreate), 0.5, 0.1, 0.4, 0.1);

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pPortText), 0.1f, 0.5, 0.35f, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pPort), 0.45, 0.5, 0.1, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pNickText), 0.1f, 0.525f + Fonts.GetLineHeight(xUnit), 0.2f, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pNick), 0.3, 0.525f + Fonts.GetLineHeight(xUnit), 0.5, Fonts.GetLineHeight(xUnit));

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pQuit), 0.0, 0.9, 0.5, 0.1);

			pNick->SetText("NoName");

			stringstream ss;
			ss << Networking::netPort;
			pPort->SetText(ss.str());
			pPort->SetMaxLen(5);

			sleep = true;
			sleepms = 16;
		}

		void NetworkJoinOrCreate::Join(Window::GUI::EventType evt, void* arg)
		{
			NetworkJoinOrCreate *pParent = (NetworkJoinOrCreate*)arg;
			pParent->go = false;
			pParent->returnValue = NETWORKJOIN;
			Networking::netPort = atoi(pParent->pPort->GetText().c_str());
			Networking::nickname = pParent->pNick->GetText();
		}

		void NetworkJoinOrCreate::Create(Window::GUI::EventType evt, void* arg)
		{
			NetworkJoinOrCreate *pParent = (NetworkJoinOrCreate*)arg;
			pParent->go = false;
			pParent->returnValue = NETWORKCREATE;
			Networking::netPort = atoi(pParent->pPort->GetText().c_str());
			Networking::nickname = pParent->pNick->GetText();
		}
		void NetworkJoinOrCreate::Quit(Window::GUI::EventType evt, void* arg)
		{
			NetworkJoinOrCreate *pParent = (NetworkJoinOrCreate*)arg;
			pParent->go = false;
			pParent->returnValue = MENU;
		}

		NetworkCreate::NetworkCreate()
		{
			pStatus = new Window::GUI::Label();
			pStart = new Window::GUI::TextButton();
			pQuit = new Window::GUI::TextButton();
			pMainPanel = new Window::GUI::Panel();

			this->SetPanel(pMainPanel);

			pStatus->SetText("Awaiting connections");

			pStart->SetText("Starta spelet");
			pStart->AttachHandler(&Start);
			pStart->SetTag(this);

			pQuit->SetText("Avsluta");
			pQuit->AttachHandler(&Quit);
			pQuit->SetTag(this);

			pMainPanel->SetConstraintPercent(pMainPanel->Add(pStatus), 0.1, 0.1, 0.8, Fonts.GetLineHeight(xUnit));
			pMainPanel->SetConstraintPercent(startGame = pMainPanel->Add(pStart), 0.4, 0.2, 0.2, 0.1);
			pMainPanel->SetConstraintPercent(pMainPanel->Add(pQuit), 0.0, 0.9, 0.5, 0.1);

			sleep = true;
			sleepms = 16;

			awaitingGo = false;
		}

		void NetworkCreate::Reset()
		{
			pMainPanel->SetVisible(startGame, true);
			awaitingGo = false;
		}

		void NetworkCreate::Start(Window::GUI::EventType evt, void* arg)
		{
			NetworkCreate *pParent = (NetworkCreate*)arg;
			pParent->awaitingGo = true;
			Networking::StartGame();
		}

		void NetworkCreate::PerformPreFrame()
		{
			Networking::PerformPregameNetworking();

			if(awaitingGo)
			{
				pStatus->SetText("Awaiting start.");
			}
			else
			{
				stringstream str;
				str << "Antal spelare: " << Networking::playerCounter;
				pStatus->SetText(str.str());
				if (Game::Rules::numPlayersGoal && Networking::playerCounter == Game::Rules::numPlayersGoal)
				{
					this->Start(MOUSE_PRESS, this);
				}
			}

			if(Networking::isReadyToStart == true)
			{
				this->go = false;
				this->returnValue = GAME;
			}
		}

		void NetworkCreate::Quit(Window::GUI::EventType evt, void* arg)
		{
			NetworkCreate *pParent = (NetworkCreate*)arg;
			pParent->go = false;
			pParent->returnValue = MENU;
			pParent->pMainPanel->SetVisible(pParent->startGame, false);
			Networking::ShutdownNetwork();
		}
	}
}

