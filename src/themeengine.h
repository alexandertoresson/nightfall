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
#ifndef __THEMEENGINE_H__
#define __THEMEENGINE_H__ 

#ifdef DEBUG_DEP
	#warning "themeengine.h"
#endif

#include "themeengine-pre.h"

#include "compositor.h"

namespace GUI
{
	namespace ThemeEngine
	{
		namespace Info
		{
			class SubComponent
			{
				protected:
					Component *subComp;
			};

			class Text
			{
				protected:
					std::string text;
			};

			struct ToggleButtonGroup
			{
				ToggleButton* checked;
			};

			class ToggleButton
			{
				protected:
					bool checked;
					ToggleButtonGroup* group;
			};

			class Range
			{
				public:
					enum Style
					{
						STYLE_RANGE,
						STYLE_SCROLLBAR,
						STYLE_NUMBER
					} style;
				protected:
					Direction direction;
					float low, high;
					float position;
			};
			
			class Image
			{
				protected:
					std::string filename;
					float width, height;
			};
		}
	}
}

#ifdef DEBUG_DEP
	#warning "themeengine.h-end"
#endif

#endif
