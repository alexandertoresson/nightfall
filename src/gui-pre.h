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

#endif

