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
#ifndef GAMEGUI_H
#define GAMEGUI_H

#ifdef DEBUG_DEP
#warning "gamegui.h"
#endif

#include "gamegui-pre.h"

#include "game-pre.h"
#include "gui.h"
#include "unit-pre.h"
#include <string>
#include <vector>
#include <map>

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
			gc_root_ptr<Dimension::Unit>::type pUnit;
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
			void SetSelected(const gc_ptr<Dimension::Unit>&);
			void Update();
			~UnitActions();

		};

		class UnitBuild : public Window::GUI::Panel
		{
		protected:
			std::vector<BuildButton*> objects;

			gc_root_ptr<Dimension::UnitType>::type pUnitType;
			gc_root_ptr<Dimension::Unit>::type pUnit;

			void SetLayout();

			struct InternalHandler
			{
				UnitBuild *parent;
				int buildID;
			};

			static void BuildSelected(Window::GUI::EventType, void*);

			GLuint nopic;

			GameWindow* pGame;
			void GetBuildPercentage(int id, float& value, std::string& lbl);
		public:
			UnitBuild(GLuint, GameWindow*);
			void SetUnitType(const gc_ptr<Dimension::UnitType>&);
			void SetUnit(const gc_ptr<Dimension::Unit>&);
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

			gc_root_ptr<Dimension::Unit>::type pUnit;

			std::map<gc_root_ptr<Dimension::UnitType>::type, UnitBuild*> GUI_Build;
			GameWindow* pGame;
			GLuint tmap;

			GLuint nopic;

			float location1;
			float location2;

			GLuint texturesTop[3];
			GLuint textureBack;

			void PaintBackground();
			void PaintTop();

		public:
			GamePlayBar(GameWindow* ref);
			void init();
			void SetUnit();
			void SwitchSelected(const gc_ptr<Dimension::Unit>&);
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

			void UnitActionEventHandler(SDL_Event*);
			void MouseDownLeft(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseDownRight(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseUpLeft(SDL_Event*,Window::GUI::TranslatedMouse*);
			void MouseUpRight(SDL_Event*,Window::GUI::TranslatedMouse*);
			void SetTypeToBuild(unsigned int num);
			void AddSelectedUnit(const gc_ptr<Dimension::Unit>& unit); 
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
			Window::GUI::TextButton *pLoad;
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

		class FileBrowserDialog : public Window::GUI::GUIWindow
		{
		public:
			enum BrowserMode
			{
				MODE_LOAD = 1,
				MODE_SAVE
			};

			static const char ALL_FILES = '*';

		private:

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

			struct FileEntry
			{
//				Utilities::FSData* file;
				int buttonId;
			};

			std::string path;
			std::string extension;
			Window::GUI::Panel*   pMainPanel;
			Window::GUI::Label*   pHeader;
			FileEntry**           fileList;

			SwitchState returnState;
			BrowserMode mode;
			int filesCount;

			void PopulateList();
			void DeallocButtons();

		public:

			FileBrowserDialog(BrowserMode);
			~FileBrowserDialog();

			void SetInitialDirectory(std::string newPath) { path = newPath;    }
			std::string GetFilename() const               { return path;       }

			void SetExtension(const std::string newExt)  { extension = newExt; }
			std::string GetExtension() const             { return extension;   }
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

		std::string GetPower();
		std::string GetMoney();
		std::string GetTime();
	}
}

#ifdef DEBUG_DEP
#warning "gamegui.h-end"
#endif

#endif

