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
#ifndef THEMEENGINE_H
#define THEMEENGINE_H 

#ifdef DEBUG_DEP
	#warning "themeengine.h"
#endif

#include "themeengine-pre.h"
#include "compositor-pre.h"
#include "materialxml-pre.h"

#include "font.h"

namespace GUI
{
	namespace ThemeEngine
	{
		class SubComponent : public InfoBase
		{
			protected:
				gc_ptr<Component> subComp;
			public:
				void Set(std::string text);
				void Set(gc_ptr<Component> comp);
		};
		
		class Text : public InfoBase
		{
			private:
				std::string text;
			public:
				Window::GUI::Font font;

				Text(gc_ptr<Component> component, std::string text, Window::GUI::Font font);

				void Set(std::string text);
				std::string Get() const;
		};

		class FrameBorders : public InfoBase
		{
			private:
				bool hasTitle;

			public:
				FrameBorders(gc_ptr<Component> component, bool hasTitle, std::string text, Window::GUI::Font font);

				Text text;

				void SetHasTitle(bool hasTitle);
				bool GetHasTitle() const;
		};

		class Image : public InfoBase
		{
			public:
				enum Style
				{
					// Bit pattern legend:
					// bit 0: Whether to stretch x axis
					// bit 1: Whether to stretch y axis
					// bit 2: Whether to preserve aspect ratio
					STYLE_UNSCALED = 0x0, // 0b000
					STYLE_STRETCH_X, // 0b001
					STYLE_STRETCH_Y, // 0b010
					STYLE_STRETCH, // 0b011
					// 0b100 is skipped as it does not make sense; if you don't stretch any axis, aspect ratio will be kept by default
					STYLE_SCALE_X = 0x5, // 0b101
					STYLE_SCALE_Y, // 0b110
					STYLE_SCALE // 0b111
				};

			private:
				std::string filename;
				gc_ptr<Utilities::TextureImageData> texture;

				float width, height;
				Style style;
				
			public:
				Image(gc_ptr<Component> component, std::string file, float width, float height, Style style);

				void SetImage(std::string filename);
				void SetDimensions(float width, float height);
				void SetStyle(Style style);

				std::string GetImage() const;
				float GetWidth() const;
				float GetHeight() const;
				Style GetStyle() const;
		};
		
		struct Color
		{
			Color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}
			unsigned char r, g, b, a;

			void shade() {}
		};

		template <typename T>
		struct States
		{
			T normal, active, prelight, insensitive, selected;
			void shade()
			{
				normal.shade();
				active.shade();
				prelight.shade();
				insensitive.shade();
				selected.shade();
			}
		};

		typedef States<Color> ColorStates;

		struct Style
		{
			ColorStates text, base, fg, bg;
			Window::GUI::Font font;

			float xThickness, yThickness;

			void shade()
			{
				text.shade();
				base.shade();
				fg.shade();
				bg.shade();
				font.shade();
			}
		};

	}
}

#ifdef DEBUG_DEP
	#warning "themeengine.h-end"
#endif

#endif
