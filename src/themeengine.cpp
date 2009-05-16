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
#include "themeengine.h"
#include "font.h"

namespace GUI
{
	namespace ThemeEngine
	{
		InfoBase::InfoBase(gc_ptr<Component> component) : comp(component)
		{
			
		}

		void InfoBase::NotifyChanged() const
		{
			comp->ScheduleRelayout();
		}

		gc_ptr<Component> InfoBase::GetComponent() const
		{
			return comp;
		}
		
		Text::Text(gc_ptr<Component> component, std::string text) : InfoBase(component), text(text)
		{
			
		}

		void Text::Set(std::string text)
		{
			this->text = text;
		}

		std::string Text::Get() const
		{
			return text;
		}

		Borders::Borders(gc_ptr<Component> component, float size, Borders::Style style) : InfoBase(component), style(style), size(size)
		{
			
		}

		FrameBorders::FrameBorders(gc_ptr<Component> component, FrameBorders::Style style) : InfoBase(component), style(style)
		{
			
		}

		void FrameBorders::SetStyle(Style style)
		{
			this->style = style;
		}

		FrameBorders::Style FrameBorders::GetStyle() const
		{
			return style;
		}

		RangeBase::RangeBase(gc_ptr<Component> component, float low, float high, Direction direction) : InfoBase(component), direction(direction), low(low), high(high)
		{
			
		}

		ScrollBar::ScrollBar(gc_ptr<Component> component, float low, float high, float posLow, float posHigh, bool showAlways, Direction direction) :
			RangeBase(component, low, high, direction), posLow(posLow), posHigh(posHigh), showAlways(showAlways)
		{
			
		}

		template <>
		void Drawer<Text>::Draw(const Text& text, float x, float y, float w, float h) const
		{
			::Window::GUI::TextRenderer::RenderedText RenderedInfo;
			::Window::GUI::defaultFonts[2].RenderText(text.Get(), RenderedInfo, Workspace::metrics.GetDPI());
			
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, RenderedInfo.texture);
			glPushMatrix();
			glTranslatef(x, y, 0.0f);
			glBegin(GL_QUADS);
				glColor4f(1.0f,1.0f,1.0f,1.0f);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);

				glTexCoord2f(RenderedInfo.maxX, 0.0f);
				glVertex2f(RenderedInfo.w, 0.0f);

				glTexCoord2f(RenderedInfo.maxX, RenderedInfo.maxY);
				glVertex2f(RenderedInfo.w, RenderedInfo.h);

				glTexCoord2f(0.0f, RenderedInfo.maxY);
				glVertex2f(0.0f, RenderedInfo.h);
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
		}

		template <>
		void Drawer<Text>::GetInnerSize(const Text& text, float w, float h, float& iw, float& ih) const
		{
			iw = ih = 0;
		}

		template <>
		void Drawer<Text>::GetOuterSize(const Text& text, float cw, float ch, float& w, float& h) const
		{
			::Window::GUI::TextRenderer::TextDimension dims;
			dims = ::Window::GUI::defaultFonts[2].GetTextSize(text.Get(), Workspace::metrics.GetDPI());
			w = dims.w;
			h = dims.h;
		}
		
		template <>
		void Drawer<Component>::Draw(const Component& comp, float x, float y, float w, float h) const
		{
		}
		
		template <>
		void Drawer<Component>::GetInnerSize(const Component& comp, float w, float h, float& iw, float& ih) const
		{
			iw = w - comp.margin.right - comp.margin.left;
			ih = h - comp.margin.top - comp.margin.bottom;
		}

		template <>
		void Drawer<Component>::GetOuterSize(const Component& comp, float cw, float ch, float& w, float& h) const
		{
			w = cw + comp.margin.right + comp.margin.left;
			h = ch + comp.margin.top + comp.margin.bottom;
		}
		
		template class Drawer<ToggleButton>;
		template class Drawer<SubComponent>;
		template class Drawer<Image>;
		template class Drawer<Button>;
		template class Drawer<Borders>;
		template class Drawer<FrameBorders>;
		template class Drawer<Range>;
		template class Drawer<ScrollBar>;
		template class Drawer<UpDown>;
		
		// Default null implementations used for unimplemented default drawers
		template <typename T>
		void Drawer<T>::Draw(const T& info, float x, float y, float w, float h) const {}

		template <typename T>
		void Drawer<T>::GetOuterSize(const T& info, float cw, float ch, float& w, float& h) const {w = cw; h = ch;}
		
		template <typename T>
		void Drawer<T>::GetInnerSize(const T& info, float w, float h, float& iw, float& ih) const {iw = w; ih = h;}
	}
}
