#ifndef __GUI_H_PRE__
#define __GUI_H_PRE__

#ifdef DEBUG_DEP
#warning "gui.h-pre"
#endif

namespace Window
{
	namespace GUI
	{
		class Panel;
		class FixedPanel;
		class GUIWindow;
		class Widget;
		struct Event;
		struct PanelInfo;
		struct TranslatedMouse;
		
		class TextButton;
		class PicButton;
		class Label;
		class Picture;
		class Selector;
		
		enum EventType
		{
			MOUSE_PRESS = 0,
			MOUSE_UP = 1,
			MOUSE_DOWN = 2,
			MOUSE_SCROLL = 3,
			MOUSE_MOVE = 4,
			KB_DOWN = 5,
			KB_UP = 6,
			MOUSE_OUT = 7,
			FOCUS = 8,
			FOCUS_OUT = 9,
			EVENT_COUNT = 10
		};

	}
}

#include "errors.h"

#define __GUI_H_PRE_END__

#include "terrain.h"
#include "font.h"
#include "console.h"

#endif

#ifdef __FONT_H_PRE_END__

#ifdef __TERRAIN_H_PRE_END__

#ifndef __GUI_H__
#define __GUI_H__

#ifdef DEBUG_DEP
#warning "gui.h"
#endif

#include "sdlheader.h"
#include <vector>
#include <deque>
#include <string>
#include <map>

using namespace std;

/* Standards:
 *	xUnit & yUnit: Pixel Unit at this location, gets bigger as smaller the element gets. If over 1 then the widget is below 1 pixel big.
 *	Widget Width, Height is panel specific. FixedPanel totally ignores it.
 */

namespace Window
{
	namespace GUI 
	{

		float PixelAlign(float value, float unit);
		GLuint CreateMap(SDL_Surface*,Game::Dimension::HeightMap*,int,int,int,int);

		struct TranslatedMouse
		{
			float x;
			float y;
		};

		struct Event
		{
			EventType eType;
			SDL_Event *pEvent;
			TranslatedMouse *pMouse;
		};

		class GUIWindow //Handles window and message loop and initates paints.
		{
			protected:
				float x;
				float y;
				float w;
				float h;
				float xUnit; //The pixel unit, or resolution
				float yUnit;
				Panel* pMainPanel;
				bool go;
				int returnValue;

				bool sleep;
				int sleepms;
				
				virtual void PerformPreFrame() {}
				virtual bool PaintAll(); //Recursive painting...
				virtual bool ProcessEvents(); //Sends all events recursive...
				TranslatedMouse* TranslateMouseCoords(Uint16,Uint16);
			public:
				GUIWindow(); //Master Width, Height
				virtual void SetPanel(Panel*);
				virtual int RunLoop();

				virtual ~GUIWindow();
		};

		class Widget //The visible end product: GUI Element
		{
			protected:
				float w;
				float h;
				
				float gx;
				float gy;
				float gw;
				float gh;

				float xUnit;
				float yUnit;
			public:
				Panel* parent;
				int id;

				Widget();
				virtual int Paint();
				virtual void PaintTooltip() { }
				virtual int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				void SetCoordinateSystem(float w, float h, float xUnit, float yUnit);
				void SetGlobalCoordinateSystem(float gx, float gy, float gw, float gh);
				void ConvertToGlobalCoordinateSystem(float& x, float& y, float& w, float& h);
				//This is for panels that will take the width / heigth ratio into account when calculating layout.
				virtual float Width() { return 1.0f; }
				virtual float Height() { return 1.0f; }
				virtual ~Widget();
		};

		enum GUIType
		{
			typePanel,
			typeWidget,
			typeEnd
		};

		typedef union 
		{ 
			Widget* pWidget;
			Panel* pPanel;
		} PanelWidget;

		struct ObjectInfo // Placeholder object
		{
			float x;
			float y;
			float w;
			float h;
			float sx;
			float sy;
			float target_aspect;
			bool visible;
			GUIType type;
			PanelWidget object;
			void* tag; //For future additions to FixedPanel
		};
		
		class Panel // Handles relative positioning
		{
			protected:
				float x;
				float y;
				float w;
				float h;
				float gx;
				float gy;
				float gw;
				float gh;
				float xUnit;
				float yUnit;

				vector<ObjectInfo*> ObjectInformation;
				map<int, ObjectInfo*> ObjectMap;
				int counter;
				
				PanelWidget Focused;
				GUIType FocusedType;

				ObjectInfo* lastMouseOver;
				TranslatedMouse lastMouseCoords;

				ObjectInfo* currentMouseDown;

				ObjectInfo* currentMovable;
				TranslatedMouse currentMovableAnchor;

				void MouseOutCheck(ObjectInfo*, TranslatedMouse*);
				void MouseOutCall();
				
				virtual bool GetWidgetPanelInPoint(TranslatedMouse* Coord, ObjectInfo*& obj);
				virtual void PaintBackground();
				virtual void PaintTop() {}

				bool focusOnClick;
				GLfloat bgColor[4];
				GLuint texture;
				bool hasTexture;
			public:
				Panel* parent;
				int id;

				Panel();
				virtual int Add(Widget*);
				virtual int Add(Panel*);
				virtual int Delete(int panel_widget);
				virtual int Clear();

				virtual void Paint();
				virtual void PaintTooltip();
				virtual void PerformPreFrame() {}
				virtual int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				
				void FixCoordinateSystem(float x, float y, float sx, float  sy); //Fixes OpenGL coordinate system.
				void SetCoordinateSystem(float x, float y, float w, float h, float xUnit, float yUnit); //Sets by Window/Pane/Panel
				TranslatedMouse GetInverseCoordinates(int id, float relX, float relY);
				void SetMovable(int id, float anchorX, float anchorY);

				void SetGlobalCoordinateSystem(float gx, float gy, float gw, float gh); //Sets by Window/Panel
				void SetConstraint(int panel_widget, float x, float y, float w, float h);
				void SetConstraintPercent(int panel_widget, float x, float y, float w, float h);
				void CopyCoordianteSystem(int panel_widget, PanelWidget, GUIType);
				void RevertCoordinateSystem();
				void SetTexture(GLuint texture);

				virtual void SetVisible(int id, bool value);
				virtual void SetElement(int panel_widget, PanelWidget, GUIType);
				
				virtual float GetWidth();
				virtual float GetHeight();
				
				virtual void UpdateLayout() {}

				void ReformMouseCoords(TranslatedMouse* coords);
				void ReformWidgetMouseCoords(TranslatedMouse* coords, ObjectInfo*);
				void SetFocus(int id);
				void SetFocusEnabled(bool value);
				virtual ~Panel();
		};
		// End of GUI Core

		typedef string(*InfoLabelfptr)(void);

		class Tooltip : virtual public Widget
		{
			protected:	
				string tooltip;
				bool painttooltip;
				float mx;
				float my;
				int tooltiptype;
			public:
				Tooltip() {tooltiptype = 0; painttooltip = false;}
				virtual void PaintTooltip();
				void SetTooltip(string tooltip);
		};

		class Picture : virtual public Widget, virtual public Tooltip
		{
			protected:
				GLuint picture;
				GLfloat *texCoords; //4 * 2
				GLfloat *quadCoords; //4 * 2
				GLfloat bgColor[4];
				GLfloat picColor[4];
				GLuint bgPicture;
				void PaintBackground();
				void PaintPicture();
			public:
				Picture();
				virtual int Paint();
				void SetPicture(GLuint pic, GLfloat *texCoords, GLfloat *quadCoords);
				void SetPicture(GLuint pic);
				void SetPictureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
				void SetBackground(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
				void SetBackground(GLuint pic);
				virtual ~Picture() {}
		};

		//Standard centered.
		class Label : virtual public Widget , virtual public Tooltip
		{
			protected:
				string text;
				int type;
				void PaintLabel(Window::GUI::FontCache::RenderedText obj, float tx, float ty);
				void PaintLabel();
			public:
				Label();
				virtual int Paint();
				virtual int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				virtual void SetType(int sizeType) { type = sizeType; }
				void SetText(string text);
				virtual ~Label() {}
		};

		class InfoLabel : public Label
		{
			protected:
				InfoLabelfptr datasource;
			public:
				InfoLabel();
				void AttachHandler(InfoLabelfptr);
				virtual int Paint();
				virtual ~InfoLabel() {}
				
		};

		typedef void(*funcptr)(EventType,void*);	
		/*
		-- Selector class.
		class Selector : virtual public Widget, virtual public Tooltip
		{
			protected:
				vector<string> selections;
				int selected;
			public:
				Selector();
				virtual int Paint();
				virtual int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				virtual ~Selector();

		};
		*/

		class TextBox : virtual public Widget, virtual public Tooltip
		{
			protected:
				unsigned marker;
				int markerbyte;
				float translateX;
				string text;
				vector<Uint16> textBlocks;
				unsigned maxSize;

				float markerX;
				FontCache::TextDimension textSize;

				bool drawMarker;

				void Backspace();
				void Delete();
				void Left();
				void Right();
				void UpdateGraphicalMarker();
			public:
				TextBox();
				virtual int Paint();
				virtual int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				void SetMaxLen(int len);
				void SetText(string text);
				string GetText();
				virtual ~TextBox() {}

		};

		class Button : virtual public Widget, virtual public Tooltip
		{
			protected:
				funcptr onEvent;
				float fadeValue;
				bool mouseDown;
				bool mouseOver;
				void* tag;
				Uint32 lastPaint;
				void PaintOverlay();
				void PaintBorderOverlay();
			public:
				Button();
				virtual int HandleEvent(EventType, SDL_Event*, TranslatedMouse*);
				void SetTag(void *tag);
				void AttachHandler(funcptr);
				void ResetFade();
				virtual ~Button() {};
		};

		class TextButton : public Button, public Label
		{
			public:
				TextButton() {}
				int HandleEvent(EventType, SDL_Event*, TranslatedMouse*);
				int Paint();
				virtual ~TextButton() {}
		};

		class PicButton : public Button, public Picture
		{
			public:
				PicButton() {}
				int Paint();
		};

		class PicTextButton : public Label, public Picture, public Button
		{
			public:
				int HandleEvent(EventType, SDL_Event*, TranslatedMouse*);
				int Paint();
		};

		class Progressbar : virtual public Widget
		{
			protected:
				float maxValue;
				float value;
				bool vertical;
				bool shadeColors;
				GLfloat color[4];
				void PaintProgressbar();
				float shade;
			public:
				Progressbar();
				Progressbar(bool vertical);
				void SetShade(bool active, float strength);
				void SetMax(float value);
				void SetValue(float value);
				void Increment(float value);
				void SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
				virtual int Paint();
				virtual ~Progressbar() {}
		};

		class LoadWindow : public GUIWindow
		{
			private:
				Progressbar *progressbar;
				Label *label;
				Panel *panel;
				void SetLayout();				
			public:
				LoadWindow(float maxProgress);
				virtual int RunLoop() { return SUCCESS; }
				void SetProgress(float val);
				void SetMessage(string message);
				void Increment(float val);
				int Update();
				virtual ~LoadWindow();				
		};

		class ConsoleBuffer : public Label
		{
			protected:
				float scrollSize;
				float scrollWidth;
				bool mouseDown;
				bool mouseCorrect;
				float lineHeight;
				int translate;
				int visibleLines;
				bool scrollOnInput;
				void SetLineHeight(float value, int vlines);
			public:
				ConsoleBuffer();
				void PrepareBuffer();
				int Paint();
				int HandleEvent(EventType, SDL_Event*, TranslatedMouse*);
				void EventRemLine();
		};
/*
		class Selector : public Label
		{
			private:
				vector<string> selections;
				int current;
				void PaintArrows();
			public:
				Selector() { current = 0 };
				int Paint();
				int HandleEvent(EventType,SDL_Event*,TranslatedMouse*);
				void AddSelection(string name);
				void SetSelectedID(int id);
				int GetSelectedID();

		};
*/
		class Map : public Widget
		{
			protected:
				GLuint mapTexture;
			public:
				Map(GLuint map);
				int Paint();
		};
	}
}
#ifdef DEBUG_DEP
#warning "gui.h-end"
#endif

#define __GUI_H_END__

#endif

#endif

#endif
