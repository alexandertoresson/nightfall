#ifndef __GAMEGUI_H_PRE__
#define __GAMEGUI_H_PRE__

#ifdef DEBUG_DEP
#warning "gamegui.h-pre"
#endif

namespace Game
{
	namespace Rules
	{
		class GameTopBar;
		class GamePlayBar;
		class UnitSelected;
		class UnitBuild;
		class UnitActions;

		class BuildButton;
		class GameMap;
		class GameInput;
		class GameMenu;
	}
}

#define __GAMEGUI_H_PRE_END__

#include "game.h"
#include "gui.h"
#include "dimension.h"
#include "unit.h"

#endif

#ifdef __UNIT_H_PRE_END__
#ifdef __DIMENSION_H_PRE_END__
#ifdef __GUI_H_END__
#ifdef __GAME_H_PRE_END__

#ifndef __GAMEGUI_H__
#define __GAMEGUI_H__

#ifdef DEBUG_DEP
#warning "gamegui.h"
#endif

#include <string>

using namespace std;

namespace Game
{
	namespace Rules
	{
		class UnitSelected : public Window::GUI::Widget
		{
		protected:
			GameWindow *pGame;
			GamePlayBar *pPlay;
			float wBox;
			float hBox;
			float txBox;
			float tyBox;

			int colCount;
			int mouseOver;
			int count;
			GLuint nopic;
		public:
			UnitSelected(GameWindow *pWinRef, GamePlayBar *pPlayBar, GLuint nopic);
			int HandleEvent(Window::GUI::EventType,SDL_Event*,Window::GUI::TranslatedMouse*);
			int Paint();
			void PaintTooltip() { }
			void Update();
		};

		class BuildButton : public virtual Window::GUI::Widget, public Window::GUI::Button, public Window::GUI::Picture, public Window::GUI::Label, public Window::GUI::Progressbar
		{
		protected:
			UnitBuild *pBuildPanel;
			int id;
		public:
			BuildButton(UnitBuild *pParent, int id) { pBuildPanel = pParent; this->id = id; this->type = 0; this->vertical = true;}
			int HandleEvent(Window::GUI::EventType,SDL_Event*,Window::GUI::TranslatedMouse*);
			int Paint();
		};

		class UnitActions : public Window::GUI::Panel
		{
		protected:
			Dimension::Unit *pUnit;
			Window::GUI::PicButton* MoveAttack;
			Window::GUI::PicButton* Stop;
			Window::GUI::PicButton* Build;
			Window::GUI::Picture* UnitPic;
			Window::GUI::Progressbar* Health;

			int idMoveAttack;
			int idStop;
			int idBuild;
			int idHealth;

			GameWindow *pGame;

			struct InternalHandler
			{
				UnitActions *parent;
				int ActionID;
			};

			static void ActionSelected(Window::GUI::EventType, void*);

			GLuint nopic;
		public:
			UnitActions(GLuint nopic, GameWindow *ref);
			void InitLayout();
			void SetSelected(Dimension::Unit*);
			void Update();
			~UnitActions();

		};

		class UnitBuild : public Window::GUI::Panel
		{
		protected:
			vector<BuildButton*> objects;

			Dimension::UnitType *pUnitType;
			Dimension::Unit *pUnit;

			void SetLayout();

			struct InternalHandler
			{
				UnitBuild *parent;
				int buildID;
			};

			static void BuildSelected(Window::GUI::EventType, void*);

			GLuint nopic;

			GameWindow* pGame;
			void GetBuildPercentage(int id, float& value, string& lbl);
		public:
			UnitBuild(GLuint, GameWindow*);
			void SetUnitType(Dimension::UnitType*);
			void SetUnit(Dimension::Unit*);
			~UnitBuild();
			friend class BuildButton;
		};

		class GameMap : public Window::GUI::Widget
		{
		private:
			GLuint map;
			GLuint mapoverlay;
			float aspect;
			bool mouseDown;
			int frames;
		public:
			GameMap(GLuint themap);
			int HandleEvent(Window::GUI::EventType,SDL_Event*,Window::GUI::TranslatedMouse*);
			int Paint();
			void Update();
			void SetPicture(GLuint map) { this->map = map; }
			GLuint GetPicture() { return map; }
		};

		class GameTopBar : public Window::GUI::Panel //Top Location
		{
		protected:
			//Menu Button
			
			GLuint textures[3];
			void PaintBackground();
		public:
			GameTopBar();
			void init() {}
			~GameTopBar() {}

		};

		void test(Window::GUI::EventType evType,void* tag);

		class GamePlayBar : public Window::GUI::Panel //Bottom Location
		{
		protected:
			//Status Panel
			//Selection Panel
			//Map Panel
			int build_panel;
			int info_panel;
			int map_panel;
			bool buildSelected;

			UnitActions* pActions;
			UnitSelected* pSelected;
			Window::GUI::Panel* pEmptyBuild;
			Window::GUI::Panel* pEmptySelected;
			Window::GUI::Picture* pPicture;
			GameMap* pMap;

			Dimension::Unit* pUnit;

			map<Dimension::UnitType*, UnitBuild*> GUI_Build;
			GameWindow* pGame;
			GLuint tmap;

			float location1;
			float location2;

			GLuint texturesTop[3];
			GLuint textureBack;

			void PaintBackground();
			void PaintTop();

		public:
			GamePlayBar(GameWindow* ref,SDL_Surface *map);
			void init();
			void SetUnit();
			void SwitchSelected(Dimension::Unit*);
			void SwitchBuild();
			GameMap* GetMap();
			~GamePlayBar();
			friend class GameWindow;
		};

		class GameInput : public Window::GUI::Widget
		{
		protected:
			GameWindow *pGame;
			float start_fx;
			float start_fy;
			float end_fx;
			float end_fy;

			void MouseDownLeft(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseDownRight(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseUpLeft(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseUpRight(SDL_Event*,Window::GUI::TranslatedMouse*);
			void SetTypeToBuild(unsigned int num);
			void AddSelectedUnit(Dimension::Unit* unit); 
			void SetGroup(unsigned int num);
			void RecallGroup(unsigned int num);
			void AddGroup(unsigned int num);

		public:
			GameInput(GameWindow *ref);
			int Paint();
			int HandleEvent(Window::GUI::EventType,SDL_Event*,Window::GUI::TranslatedMouse*);
		};

		class GameTest : public Window::GUI::GUIWindow
		{
			protected:
				Window::GUI::Panel *pMainPanel;
				Window::GUI::TextBox *pTextBox;
				Window::GUI::TextButton *pExit;

				static void ActionSelect(Window::GUI::EventType, void*);

			public:
				GameTest();
		};

		class NetworkJoinOrCreate : public Window::GUI::GUIWindow
		{
		private:
			Window::GUI::TextButton *pJoin;
			Window::GUI::TextButton *pCreate;
			Window::GUI::TextButton *pQuit;
			Window::GUI::TextBox *pPort;
			Window::GUI::TextBox *pNick;
			Window::GUI::Panel *pMainPanel;
			Window::GUI::Label *pPortText;
			Window::GUI::Label *pNickText;
		public:
			NetworkJoinOrCreate();
			static void Join(Window::GUI::EventType, void*);
			static void Create(Window::GUI::EventType, void*);
			static void Quit(Window::GUI::EventType, void*);
		};

		class NetworkCreate : public Window::GUI::GUIWindow
		{
		private:
			Window::GUI::Label *pStatus;
			Window::GUI::TextButton *pStart;
			Window::GUI::TextButton *pQuit;
			Window::GUI::Panel *pMainPanel;

			int startGame;
			bool awaitingGo;

			static void Start(Window::GUI::EventType, void*);
			static void Quit(Window::GUI::EventType, void*);
		protected:
			void PerformPreFrame();
		public:
			NetworkCreate();
			void Reset();
		};

		class NetworkJoin : public Window::GUI::GUIWindow
		{
		private:
			Window::GUI::Panel *pMainPanel;
			Window::GUI::Label *pIPAdress;
			Window::GUI::TextBox *pIPText;
			Window::GUI::TextButton *pConnect;
			Window::GUI::Label *pStatus;
			Window::GUI::TextButton *pQuit;

			static void Connect(Window::GUI::EventType, void*);
			static void Quit(Window::GUI::EventType, void*);
		protected:
			bool requestSent;
			void PerformPreFrame();
		public:
			NetworkJoin();
			void Reset();
		};

		class GameMenu : public Window::GUI::GUIWindow
		{
		private:
			Window::GUI::TextButton *pStart;
			Window::GUI::TextButton *pQuit;
			Window::GUI::TextButton *pCredits;
			Window::GUI::TextButton *pMultiplayer;
			Window::GUI::Panel *pMainPanel;

			SwitchState returnState;

			struct Internal
			{
				GameMenu *parent;
				SwitchState action;

				Internal(GameMenu *mParent, SwitchState mAction)
				{
					parent = mParent;
					action = mAction;
				}
			};

			static void ActionSelect(Window::GUI::EventType, void*);

		public:
			GameMenu();
			~GameMenu();
		};

		class GameInGameMenu : public Window::GUI::GUIWindow
		{
		private:
			Window::GUI::TextButton *pContinue;
			Window::GUI::TextButton *pEndGame;
			Window::GUI::TextButton *pQuit;
			Window::GUI::Panel *pMainPanel;

			SwitchState returnState;

			struct Internal
			{
				GameInGameMenu *parent;
				SwitchState action;

				Internal(GameInGameMenu *mParent, SwitchState mAction)
				{
					parent = mParent;
					action = mAction;
				}
			};

			static void ActionSelect(Window::GUI::EventType, void*);

		public:
			GameInGameMenu();
			~GameInGameMenu();
		};

		string GetPower();
		string GetMoney();
		string GetTime();
	}
}

#define __GAMEGUI_H_END__

#ifdef DEBUG_DEP
#warning "gamegui.h-end"
#endif

#endif

#endif

#endif

#endif

#endif

