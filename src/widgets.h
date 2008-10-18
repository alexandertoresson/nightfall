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
#ifndef __WIDGETS_H__
#define __WIDGETS_H__ 

#ifdef DEBUG_DEP
	#warning "widgets.h"
#endif

#include "widgets-pre.h"

#include "compositor.h"
#include "themeengine.h"

namespace GUI
{
	namespace Widgets
	{
		class Button : public Component
		{
			public:
				ThemeEngine::SubComponent contents;
				ThemeEngine::Button button;

				Button(std::string text);
				Button(Component *comp);
		};

		class Label : public Component
		{
			public:
				ThemeEngine::Text text;
				Label(std::string text);
		};

		class TextBox : public Component
		{
			private:
				bool multiLine;
			public:
				ThemeEngine::Text text;
				TextBox(bool multiLine = false, std::string text = "");

				void setText(std::string text);
				std::string getText();
		};

		class ToggleButton : public Component
		{
			public:
				ThemeEngine::Text text;
				ThemeEngine::ToggleButton tButton;

				ToggleButton(std::string text, bool checked, ThemeEngine::ToggleButtonGroup* group = NULL);
		};

		class Range : public Component
		{
			public:
				ThemeEngine::Range range;
				Range(ThemeEngine::Range::Direction direction, ThemeEngine::Range::Style style, float low, float high);
		};

/*		class List
		{
			List(float itemWidth, float itemHeight, ThemeEngine::Direction flowDirection, ThemeEngine::Direction scrollDirection);

			void add(std::string Text);
			void add(Component *comp);
			void remove(int index);
			Component* get(int index);
		};*/

		class Image : public Component
		{
			public:
				ThemeEngine::Image image;
				ThemeEngine::Borders borders;

				Image(std::string filename, float width = 0, float height = 0);
		};

		class ToolTip : public Component
		{
			public:
				ThemeEngine::SubComponent contents;
				ThemeEngine::Borders borders;

				ToolTip(std::string text);
				ToolTip(Component *comp);
		};
	}
}

#ifdef DEBUG_DEP
	#warning "widgets.h-end"
#endif

#endif

