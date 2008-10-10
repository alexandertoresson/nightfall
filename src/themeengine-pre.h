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
#ifndef __THEMEENGINE_H_PRE__
#define __THEMEENGINE_H_PRE__ 

#ifdef DEBUG_DEP
	#warning "themeengine.h-pre"
#endif

namespace GUI
{
	namespace ThemeEngine
	{
		namespace Info
		{
			enum Direction
			{
				DIRECTION_VERTICAL,
				DIRECTION_HORIZONTAL
			};

			class SubComponent;

			class Text;

			struct ToggleButtonGroup;

			class ToggleButton;

			class Range;
			
			class Image;
		}

		class TextDrawer
		{
			public:
				virtual void Draw(Info::Text* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Text* text, float& w, float& h);
		};
	}
}

#endif
