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
#ifndef GUI_H_PRE
#define GUI_H_PRE

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

