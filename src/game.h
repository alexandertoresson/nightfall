#ifndef __GAME_H_PRE__
#define __GAME_H_PRE__

#ifdef DEBUG_DEP
#warning "game.h-pre"
#endif

namespace Game
{
	namespace Rules
	{

		extern char* CurrentLevel;

		extern float time_since_last_frame;
		extern float time_passed_since_last_water_pass;

		//All game related run loop shall return a switchstate.
		enum SwitchState
		{
			QUIT = 0,
			SETTINGS,
			INGAMEMENU,
			GAME,
			NEWGAME,
			ENDGAME,
			CREDITS,
			MENU,
			MULTIPLAYER,
			NETWORKCREATE,
			NETWORKJOIN,
			NETWORKCREATE_DEDICATED
		};

		class GameWindow;
	}
}

#define __GAME_H_PRE_END__

#include "dimension.h"
#include "unit.h"
#include "gui.h"
#include "gamegui.h"
#include "effect.h"
#include "camera.h"
#include "gamegui.h"

#endif

#ifdef __GAMEGUI_H_PRE_END__
#ifdef __EFFECT_H_PRE_END__
#ifdef __DIMENSION_H_PRE_END__
#ifdef __UNIT_H_PRE_END__
#ifdef __GUI_H_END__
#ifdef __GAMEGUI_H_END__
#ifdef __CAMERA_H_PRE_END__

#ifndef __GAME_H__
#define __GAME_H__

#ifdef DEBUG_DEP
#warning "game.h"
#endif

#include <vector>
#include <queue>
#include <sstream>
#include <iomanip>

using namespace std;

namespace Game
{
	namespace Rules
	{
		void WaitForNetwork();
		int GameMain(void);
		void PrintGLError();

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
				
				//map<Dimension::UnitType*, UnitBuild*> GUI_Build;
				Dimension::Unit* buildingUnit;
				//UnitBuild* buildingGUI;

				//Animation variables
				int frames;
				Uint32 time;

				int start_drag_x, start_drag_y, end_drag_x, end_drag_y;
				bool is_pressing_lmb;

				Uint32 last_frame, this_frame;
				float time_passed_since_last_ai_pass;

				float goto_x, goto_y;
				int build_x, build_y;
		
				Dimension::UnitType* build_type;
				Uint32 goto_time;
				int pause;
				
				Dimension::Camera *worldCamera;

				Window::GUI::InfoLabel *p_lblPower;
				Window::GUI::InfoLabel *p_lblMoney;
				Window::GUI::InfoLabel *p_lblTime;

				static void Sell(Window::GUI::EventType evt, void* arg);
			private:
				static GameWindow* pInstance;

				bool ProcessEvents();
				void DestroyGUI();

				int InitGame();
			public:
				static GameWindow* Instance();
				static void Destroy();
				static bool IsNull();
				void NewGame();
				void EndGame();
				
				int InitGUI(SDL_Surface* img);

				Dimension::Camera* GetCamera(void) const;
				Window::GUI::ConsoleBuffer* GetConsoleBuffer(void) const;

				GameWindow();
				bool PaintAll();
				void PaintGame();
				void PerformPreFrame();
				void PerformPostFrame();
				void ResetGame(); //Should perform last initalization, and game start.
				int RunLoop();
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

#define __GAME_H_END__

#include "lua.h" // << needed for loading missions.

#endif

#endif

#endif

#endif

#endif

#endif

#endif

#endif

