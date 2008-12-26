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
	class Metrics;

	namespace ThemeEngine
	{
		class InfoBase;

		class SubComponent;

		class Text;

		class ToggleButton;

		class Range;
		
		class Image;
		
		class Button;

		class Borders;
		
		class FrameBorders;

		/* Basic Drawer API:
			// Pass in Info object describing how the widget should be rendered, and position and width and height of it
			virtual void Draw(Foo* text, float x, float y, float w, float h);
			// cw and ch is the size of the components inside this component.
			// w and h are the calculated sizes of this component when it has to contain components of the specified size.
			// Note that cw and ch only apply to things that can contain other stuff, like borders
			virtual void GetSize(Foo* text, float cw, float ch, float& w, float& h);
		*/

		template <typename T>
		class Drawer
		{
			public:
				virtual void Draw(const T& info, float x, float y, float w, float h);
				virtual void GetSize(const T& info, float cw, float ch, float& w, float& h);
				virtual ~Drawer() {}
		};

		class Theme
		{
			public:
				Drawer<Text>* textDrawer;
				Drawer<SubComponent>* subComponentDrawer;
				Drawer<ToggleButton>* toggleButtonDrawer;
				Drawer<Range>* rangeDrawer;
				Drawer<Image>* imageDrawer;
				Drawer<Button>* buttonDrawer;
				Drawer<Borders>* bordersDrawer;
				Drawer<FrameBorders>* frameBordersDrawer;

				Theme() :
					textDrawer(new Drawer<Text>),
					subComponentDrawer(new Drawer<SubComponent>),
					toggleButtonDrawer(new Drawer<ToggleButton>),
					rangeDrawer(new Drawer<Range>),
					imageDrawer(new Drawer<Image>),
					buttonDrawer(new Drawer<Button>),
					bordersDrawer(new Drawer<Borders>),
					frameBordersDrawer(new Drawer<FrameBorders>)
				{
					
				}
		};
		
		class InfoBase
		{
			private:
				Component* comp;
			protected:
				void notifyChanged();
			public:
				InfoBase(Component* component);
				Component* getComponent() const;
				Metrics* getMetrics() const;
		};

		class SubComponent;

		class Text : public InfoBase
		{
			private:
				std::string text;
			public:
				Text(Component* component, std::string text);

				void set(std::string text);
				std::string get() const;
		};

		struct ToggleButtonGroup
		{
			ToggleButton* checked;
		};

		class ToggleButton : public InfoBase
		{
			private:
				bool checked;
				ToggleButtonGroup* group;

				ToggleButton(Component* component, bool checked, ToggleButtonGroup* group);

				void setChecked(bool checked);
				bool getChecked() const;

				ToggleButtonGroup* getGroup();
		};

		class Range : public InfoBase
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

				Range(Component* component, float low, float high, float position, Style style, Direction direction);

				Style getStyle();

				Direction getDirection();

				void setPosition(float position);
				float getPosition() const;

				void setRange(float low, float high);
				float getLow() const;
				float getHigh() const;
		};
		
		class Image : public InfoBase
		{
			private:
				std::string filename;
				float width, height;
			public:
				Image(Component* component, std::string file, float width, float height);

				void setImage(std::string filename);
				void setDimensions(float width, float height);

				std::string getImage() const;
				float getWidth() const;
				float getHeight() const;
		};
		
		class Button : public InfoBase
		{
			public:
				enum Style
				{
					STYLE_NORMAL,
					STYLE_FLAT
				};
			private:
				Style style;

				Button(Component* component, Style style);

				void setStyle(Style style);
				Style getStyle();
		};

		class Borders : public InfoBase
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

			public:
				Borders(Component* component, float size = 0.1f, Style style = STYLE_FLAT);

				void setStyle(Style style);
				void setSize(float size);

				Style getStyle() const;
				float getSize() const;
		};
		
		class FrameBorders : public InfoBase
		{
			public:
				enum Style
				{
					STYLE_NORMAL,
					STYLE_SMALL
				};
			private:
				Style style;

				FrameBorders(Component* component, Style style);

				void setStyle(Style style);
				Style getStyle() const;
		};

	}
}

#endif
