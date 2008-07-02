#ifndef __GAME_H__
#define __GAME_H__

#ifdef DEBUG_DEP
#warning "game.h"
#endif

#include "game-pre.h"

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

		class GameWindow : public Window::GUI::GUIWindow
		{
			protected:
				Window::GUI::Panel *pMainGame;
				GamePlayBar *pPlayBar;
				GameTopBar *pTopBar;
				Window::GUI::Picture *pic;
				Window::GUI::TextButton *sell;

				Game::Dimension::InputController *input;
				Window::GUI::LoadWindow *pLoading;
				GameInput *pGameInput;

				Window::GUI::ConsoleBuffer *pConsole;
				int consoleID;

				Game::FX::Explosion *particleGen;
				float part_x;
				float part_y;

				bool particleRunning;
				bool gameRunning;
				
				Dimension::Unit* buildingUnit;

				//Animation variables
				int frames;
				Uint32 last_status_time;

				int start_drag_x, start_drag_y, end_drag_x, end_drag_y;
				bool is_pressing_lmb;

				Uint32 last_frame, this_frame;
				float time_passed_since_last_ai_pass;

				float goto_x, goto_y;
				int build_x, build_y;
		
				ref_ptr<Dimension::UnitType> build_type;
				Uint32 goto_time;
				int pause;
				
				Window::GUI::InfoLabel *p_lblPower;
				Window::GUI::InfoLabel *p_lblMoney;
				Window::GUI::InfoLabel *p_lblTime;

				static void Sell(Window::GUI::EventType evt, void* arg);
			private:
				static GameWindow* pInstance;

				bool ProcessEvents();
				void DestroyGUI();

				int InitGame(bool is_new_game = true, bool isNetworked = false, Networking::NETWORKTYPE ntype = Networking::CLIENT);
		
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
				int NewGame(bool isNetworked = false, Networking::NETWORKTYPE ntype = Networking::CLIENT);
				int LoadGame(bool isNetworked = false, Networking::NETWORKTYPE ntype = Networking::CLIENT);
				void EndGame();
				
				int InitGUI(SDL_Surface* img);

				Window::GUI::ConsoleBuffer* GetConsoleBuffer(void) const;

				GameWindow();
				bool PaintAll();
				void PerformPreFrame();
				void PerformPostFrame();
				void ResetGame(); //Should perform last initalization, and game start.
				int RunLoop();
				void Stop();
				virtual ~GameWindow();
				
			friend class GameInput; //Allows access to protected and private members of this class. Practical reasons.
			friend class UnitBuild;
			friend class UnitSelected;
			friend class UnitActions;
			friend class GamePanel;
		};
	}
}

#ifdef DEBUG_DEP
#warning "game.h-end"
#endif

#endif

