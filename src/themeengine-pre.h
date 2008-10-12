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

#include <string>

namespace GUI
{
	class Component;

	namespace ThemeEngine
	{
		namespace Info
		{
			class SubComponent;

			class Text;

			class ToggleButton;

			class Range;
			
			class Image;
			
			class Button;

			class Borders;
			
			class FrameBorders;
		}

		/* Basic Drawer API:
			// Pass in Info object describing how the widget should be rendered, and position and width and height of it
			virtual void Draw(Info::Foo* text, float x, float y, float w, float h);
			// cw and ch is the size of the components inside this component.
			// w and h are the calculated sizes of this component when it has to contain components of the specified size.
			virtual void GetSize(Info::Foo* text, float cw, float ch, float& w, float& h);
		*/

		class TextDrawer
		{
			public:
				virtual void Draw(Info::Text* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Text* text, float cw, float ch, float& w, float& h);
				virtual ~TextDrawer() {};
		};
		
		class SubComponentDrawer
		{
			public:
				virtual void Draw(Info::SubComponent* text, float x, float y, float w, float h);
				virtual void GetSize(Info::SubComponent* text, float cw, float ch, float& w, float& h);
				virtual ~SubComponentDrawer() {};
		};
		
		class ToggleButtonDrawer
		{
			public:
				virtual void Draw(Info::ToggleButton* text, float x, float y, float w, float h);
				virtual void GetSize(Info::ToggleButton* text, float cw, float ch, float& w, float& h);
				virtual ~ToggleButtonDrawer() {};
		};
		
		class RangeDrawer
		{
			public:
				virtual void Draw(Info::Range* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Range* text, float cw, float ch, float& w, float& h);
				virtual ~RangeDrawer() {};
		};
		
		class ImageDrawer
		{
			public:
				virtual void Draw(Info::Image* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Image* text, float cw, float ch, float& w, float& h);
				virtual ~ImageDrawer() {};
		};
		
		class ButtonDrawer
		{
			public:
				virtual void Draw(Info::Button* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Button* text, float cw, float ch, float& w, float& h);
				virtual ~ButtonDrawer() {};
		};
		
		class BordersDrawer
		{
			public:
				virtual void Draw(Info::Borders* text, float x, float y, float w, float h);
				virtual void GetSize(Info::Borders* text, float cw, float ch, float& w, float& h);
				virtual ~BordersDrawer() {};
		};
		
		class FrameBordersDrawer
		{
			public:
				virtual void Draw(Info::FrameBorders* text, float x, float y, float w, float h);
				virtual void GetSize(Info::FrameBorders* text, float cw, float ch, float& w, float& h);
				virtual ~FrameBordersDrawer() {};
		};
		
		class Theme
		{
			TextDrawer* textDrawer;
			SubComponentDrawer* subComponentDrawer;
			ToggleButtonDrawer* toggleButtonDrawer;
			RangeDrawer* rangeDrawer;
			ImageDrawer* imageDrawer;
			ButtonDrawer* buttonDrawer;
			BordersDrawer* bordersDrawer;
		};
		
		namespace Info
		{
			class InfoBase
			{
				private:
					Component* comp;
				protected:
					void setChanged();
			};

			class SubComponent;

			class Text : InfoBase
			{
				private:
					std::string text;
				public:
					void set(std::string text);

				friend class TextDrawer;
			};

			struct ToggleButtonGroup : InfoBase
			{
				ToggleButton* checked;
			};

			class ToggleButton : InfoBase
			{
				private:
					bool checked;
					ToggleButtonGroup* group;

				friend class ToggleButtonDrawer;
			};

			class Range : InfoBase
			{
				public:
					enum Direction
					{
						DIRECTION_VERTICAL,
						DIRECTION_HORIZONTAL
					};

					enum Style
					{
						STYLE_RANGE,
						STYLE_SCROLLBAR,
						STYLE_NUMBER
					};
				private:
					Style style;
					Direction direction;
					float low, high;
					float position;
				public:

					void setStyle(Style style);

					void setPosition(float position);
					float getPosition();

					void setRange(float low, float high);
					float getLow();
					float getHigh();

				friend class RangeDrawer;
			};
			
			class Image : InfoBase
			{
				private:
					std::string filename;
					float width, height;
				public:
					void setImage(std::string filename);
					void setDimensions(float width, float height);

				friend class ImageDrawer;
			};
			
			class Button : InfoBase
			{
				public:
					enum Style
					{
						STYLE_NORMAL,
						STYLE_FLAT
					};
				private:
					Style style;
					void setStyle(Style style);

				friend class ButtonDrawer;
			};

			class Borders : InfoBase
			{
				public:
					enum Style
					{
						STYLE_UP,
						STYLE_DOWN,
						STYLE_FLAT
					};
				private:
					Style style;
					float size;
					void setStyle(Style style);
					void setSize(float size);

				friend class BordersDrawer;
			};
			
			class FrameBorders : InfoBase
			{
				public:
					enum Style
					{
						STYLE_NORMAL,
						STYLE_SMALL
					};
					Text text;
				private:
					Style style;
					void setStyle(Style style);

				friend class FrameBordersDrawer;
			};
		}

	}
}

#endif
