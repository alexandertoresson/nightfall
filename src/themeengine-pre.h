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
#ifndef THEMEENGINE_H_PRE
#define THEMEENGINE_H_PRE 

#ifdef DEBUG_DEP
	#warning "themeengine.h-pre"
#endif

#include "gc_ptr.h"

#include <string>

namespace GUI
{
	class Component;

	namespace ThemeEngine
	{
		class InfoBase;
		class SubComponent;
		class Text;
		class ToggleButton;
		class Image;
		class Button;
		class Borders;
		class FrameBorders;
		class Range;
		class ScrollBar;
		class UpDown;
		
		/* Basic Drawer API:
			// Pass in Info object describing how the widget should be rendered, and position and width and height of it
			virtual void Draw(Foo* into, float x, float y, float w, float h);
			// cw and ch is the size of the components inside this component.
			// w and h are the calculated sizes of this component when it has to contain components of the specified size.
			// Note that cw and ch only apply to things that can contain other stuff, like borders
			virtual void GetOuterSize(Foo* info, float cw, float ch, float& w, float& h);
			// w and h is the size of the component.
			// iw and ih are the calculated width and height when the component has the specified size.
			virtual void GetInnerSize(Foo* info, float w, float h, float& iw, float& ih);
		*/

		template <typename T>
		class Drawer
		{
			public:
				virtual void Draw(const T& info, float x, float y, float w, float h);
				virtual void GetOuterSize(const T& info, float cw, float ch, float& w, float& h);
				virtual void GetInnerSize(const T& info, float w, float h, float& iw, float& ih);
				virtual ~Drawer() {}
				virtual void shade() {}
		};

		class Theme
		{
			public:
				gc_ptr<Drawer<Text> > textDrawer;
				gc_ptr<Drawer<SubComponent> > subComponentDrawer;
				gc_ptr<Drawer<ToggleButton> > toggleButtonDrawer;
				gc_ptr<Drawer<Image> > imageDrawer;
				gc_ptr<Drawer<Button> > buttonDrawer;
				gc_ptr<Drawer<Borders> > bordersDrawer;
				gc_ptr<Drawer<FrameBorders> > frameBordersDrawer;
				gc_ptr<Drawer<Range> > rangeDrawer;
				gc_ptr<Drawer<ScrollBar> > scrollBarDrawer;
				gc_ptr<Drawer<UpDown> > upDownDrawer;

				Theme() :
					textDrawer(new Drawer<Text>),
					subComponentDrawer(new Drawer<SubComponent>),
					toggleButtonDrawer(new Drawer<ToggleButton>),
					imageDrawer(new Drawer<Image>),
					buttonDrawer(new Drawer<Button>),
					bordersDrawer(new Drawer<Borders>),
					frameBordersDrawer(new Drawer<FrameBorders>),
					rangeDrawer(new Drawer<Range>),
					scrollBarDrawer(new Drawer<ScrollBar>),
					upDownDrawer(new Drawer<UpDown>)
				{
					
				}
		};
		
		class InfoBase
		{
			private:
				gc_ptr<Component> comp;
			protected:
				void NotifyChanged();
			public:
				InfoBase(gc_ptr<Component> component);
				gc_ptr<Component> GetComponent() const;

				virtual void shade()
				{
					comp.shade();
				}
		};

		class SubComponent;

		class Text : public InfoBase
		{
			private:
				std::string text;
			public:
				Text(gc_ptr<Component> component, std::string text);

				void Set(std::string text);
				std::string Get() const;
		};

		struct ToggleButtonGroup
		{
			gc_ptr<ToggleButton> checked;

			void shade()
			{
				checked.shade();
			}
		};

		class ToggleButton : public InfoBase
		{
			private:
				bool checked;
				gc_ptr<ToggleButtonGroup> group;

				ToggleButton(gc_ptr<Component> component, bool checked, gc_ptr<ToggleButtonGroup> group);

				void SetChecked(bool checked);
				bool GetChecked() const;

				gc_ptr<ToggleButtonGroup> GetGroup();
				
				void shade()
				{
					InfoBase::shade();
					group.shade();
				}
		};

		class RangeBase : public InfoBase
		{
			public:
				enum Direction
				{
					DIRECTION_VERTICAL,
					DIRECTION_HORIZONTAL
				};

			private:
				Direction direction;
				float low, high;
			public:

				RangeBase(gc_ptr<Component> component, float low, float high, Direction direction);

				Direction GetDirection();

				void SetRange(float low, float high);
				float GetLow() const;
				float GetHigh() const;
		};

		class Range : public RangeBase
		{
			private:
				float position;
			public:
				Range(gc_ptr<Component> component, float low, float high, float position, Direction direction);
		};
		
		class UpDown : public Range
		{
			public:
				UpDown(gc_ptr<Component> component, float low, float high, float position, Direction direction);
		};

		class ScrollBar : public RangeBase
		{
			private:
				float posLow, posHigh;
				bool showAlways; // Even if unnecessary?
			public:

				ScrollBar(gc_ptr<Component> component, float low, float high, float posLow, float posHigh, bool showAlways, Direction direction);

				void SetPosition(float posLow, float posHigh);
				void GetPosition(float& posLow, float& posHigh) const;

				void SetShowAlways(bool showAlways);
				bool GetShowAlways();
		};
		
		class Image : public InfoBase
		{
			private:
				std::string filename;
				float width, height;
			public:
				Image(gc_ptr<Component> component, std::string file, float width, float height);

				void SetImage(std::string filename);
				void SetDimensions(float width, float height);

				std::string GetImage() const;
				float GetWidth() const;
				float GetHeight() const;
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

				Button(gc_ptr<Component> component, Style style);

				void SetStyle(Style style);
				Style GetStyle();
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
				Borders(gc_ptr<Component> component, float size = 0.1f, Style style = STYLE_FLAT);

				void SetStyle(Style style);
				void SetSize(float size);

				Style GetStyle() const;
				float GetSize() const;
		};
		
		class FrameBorders : public InfoBase
		{
			public:
				enum Style
				{
					STYLE_NONE,
					STYLE_NORMAL,
					STYLE_SMALL
				};
			private:
				Style style;

			public:
				FrameBorders(gc_ptr<Component> component, Style style);

				void SetStyle(Style style);
				Style GetStyle() const;
		};

	}
}

#endif
