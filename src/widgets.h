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
		class Button : public Component, public ThemeEngine::Info::SubComponent
		{
			public:
				Button(std::string text);
				Button(Component *comp);

				void setText(std::string text);
				void setContents(Component *comp);
		};

		class Label : public Component, public ThemeEngine::Info::Text
		{
			public:
				Label(std::string text);

				void setText(std::string text);
		};

		class TextBox : public Component, public ThemeEngine::Info::Text
		{
			private:
				bool multiLine;
			public:
				TextBox(bool multiLine = false, std::string text = "");

				void setText(std::string text);
				std::string getText();
		};

		class ToggleButton : public Component, public ThemeEngine::Info::ToggleButton, public ThemeEngine::Info::Text
		{
			public:
				ToggleButton(std::string text, bool checked, ThemeEngine::Info::ToggleButtonGroup* group = NULL);

				void setText(std::string text);
				
				void setChecked(bool checked = true);
				bool getChecked();
		};

		class Range : public Component, public ThemeEngine::Info::Range
		{
			public:
				Range(ThemeEngine::Info::Direction direction, Style style, float low, float high);

				void setPosition(float position);
				float getPosition();

				void setRange(float low, float high);
				float getLow();
				float getHigh();
		};

		class List
		{
			List(float itemWidth, float itemHeight, ThemeEngine::Info::Direction flowDirection, ThemeEngine::Info::Direction scrollDirection);

			void add(std::string Text);
			void add(Component *comp);
			void remove(int index);
			Component* get(int index);
		};

		class Image : public Component, public ThemeEngine::Info::Image
		{
			public:
				Image(std::string filename, float width = 0, float height = 0);

				void setImage(std::string filename);
		};

		class ToolTip : public Component, public ThemeEngine::Info::SubComponent
		{
			public:
				ToolTip(std::string text);
				ToolTip(Component *comp);

				void setText(std::string text);
				void setContents(Component *comp);
		};
	}
}

#ifdef DEBUG_DEP
	#warning "widgets.h-end"
#endif

#endif

