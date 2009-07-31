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
#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#ifdef DEBUG_DEP
#warning "gamewindow.h"
#endif

#include "gamewindow-pre.h"

#include "dimension-pre.h"
#include "unit-pre.h"
#include "gui.h"
#include "gamegui.h"
#include "effect-pre.h"
#include "camera-pre.h"
#include "gamegui-pre.h"
#include "networking-pre.h"

#include <vector>
#include <queue>
#include <sstream>
#include <iomanip>

namespace Game
{
	namespace Rules
	{
		void WaitForNetwork();
		int GameMain(void);

		class InputController
		{
			private:
				bool mKeys[SDLK_LAST];
			
			public:
				InputController()
				{
					int length = sizeof(mKeys) / sizeof(bool);
					for (int i = 0; i < length; i++)
						mKeys[i] = false;
				}

			
				bool GetKeyState(SDLKey key) const       { return mKeys[key];  }
				void SetKeyState(SDLKey key, bool value) { mKeys[key] = value; }
		};
		
		class GameWindow : public Window::GUI::GUIWindow
		{
			protected:
				Window::GUI::Panel *pMainGame;
				GamePlayBar *pPlayBar;
				GameTopBar *pTopBar;
				Window::GUI::Picture *pic;
				Window::GUI::TextButton *sell;

				InputController input;
				Window::GUI::LoadWindow *pLoading;
				GameInput *pGameInput;

				Window::GUI::ConsoleBuffer *pConsole;
				int consoleID;

				bool particleRunning;
				bool gameRunning;
				
				gc_ptr<Dimension::Unit> buildingUnit;

				int frames;
				Uint32 last_status_time;

				int start_drag_x, start_drag_y, end_drag_x, end_drag_y;
				bool is_pressing_lmb;

				Uint32 last_frame, this_frame;
				float time_passed_since_last_ai_pass;
				float time_since_last_frame;

				float goto_x, goto_y;
				int build_x, build_y;
		
				gc_ptr<Dimension::UnitType> build_type;
				Uint32 goto_time;
				
				Window::GUI::InfoLabel *p_lblPower;
				Window::GUI::InfoLabel *p_lblMoney;
				Window::GUI::InfoLabel *p_lblTime;

				SDL_mutex* renderMutex;
				SDL_mutex* renderMutex2;

				bool pauseRendering;

				static void Sell(Window::GUI::EventType evt, void* arg);
			private:
				static GameWindow* pInstance;

				bool ProcessEvents();
				void DestroyGUI();

				class GUINode : public Scene::Graph::Node
				{
					private:
						Window::GUI::Panel* pMainPanel;
						float w, h;
						GUINode();
					protected:
						virtual void Render();
					public:
						static GUINode instance;
						void SetParams(Window::GUI::Panel* pMainPanel, float w, float h);
				};

			public:
				static GameWindow* Instance();
				static void Destroy();
				static bool IsNull();
				
				int InitGUI();

				Window::GUI::ConsoleBuffer* GetConsoleBuffer(void) const;

				GameWindow();
				bool PaintAll();
				int RunLoop();
				void Stop();
				virtual ~GameWindow();

				void PauseRendering();
				void ResumeRendering();

				GUINode& GetGUINode();
				
			friend class GameInput; //Allows access to protected and private members of this class. Practical reasons.
			friend class UnitBuild;
			friend class UnitSelected;
			friend class UnitActions;
			friend class GamePanel;
		};
	}
}

#ifdef DEBUG_DEP
#warning "gamewindow.h-end"
#endif

#endif

