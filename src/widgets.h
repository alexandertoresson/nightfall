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

namespace GUI
{
	namespace Widgets
	{
		enum Direction
		{
			DIRECTION_VERTICAL,
			DIRECTION_HORIZONTAL
		};

		class Button : public Component
		{
			Button(std::string text);
			Button(Component *comp);

			void setText(std::string text);
			void setContents(Component *comp);
		};

		class Label : public Component
		{
			Label(std::string text);

			void setText(std::string text);
		};

		class TextBox : public Component
		{
			TextBox(bool multiLine = false, std::string text = "");

			void setText(std::string text);
			std::string getText();
		};

		class ToggleButtonGroup
		{
			ToggleButton* checked;
		};

		class ToggleButton : public Component
		{
			ToggleButton(std::string text, bool checked, ToggleButtonGroup* group = NULL);

			void setText(std::string text);
			
			void setChecked(bool checked = true);
			bool getChecked();
		};

		class Range
		{
			Range(Direction direction, float low, float high);

			void setPosition(float position);
			float getPosition();
		};

		class List
		{
			List(float itemWidth, float itemHeight, Direction flowDirection, Direction scrollDirection);

			void add(std::string Text);
			void add(Component *comp);
			void remove(int index);
			Component* get(int index);
		};

		class Image
		{
			Image(std::string filename, float width, float height);

			void setImage(std::string filename);
		};

		class ToolTip
		{
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

