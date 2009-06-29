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
#include "compositor.h"

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
		
		Text::Text(gc_ptr<Component> component, std::string text, Window::GUI::Font font) : InfoBase(component), text(text), font(font)
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

		Borders::Borders(gc_ptr<Component> component, Borders::Style style) : InfoBase(component), style(style)
		{
			
		}

		FrameBorders::FrameBorders(gc_ptr<Component> component, bool hasTitle, std::string text, Window::GUI::Font font) : InfoBase(component), hasTitle(hasTitle), text(component, text, font)
		{
			
		}

		RangeBase::RangeBase(gc_ptr<Component> component, float low, float high, Direction direction) : InfoBase(component), direction(direction), low(low), high(high)
		{
			
		}

		ScrollBar::ScrollBar(gc_ptr<Component> component, float low, float high, float posLow, float posHigh, bool showAlways, Direction direction) :
			RangeBase(component, low, high, direction), posLow(posLow), posHigh(posHigh), showAlways(showAlways)
		{
			
		}
	
		template <>
		void Drawer<FrameBorders>::Draw(FrameBorders& borders, float x, float y, float w, float h)
		{
		}

		template <>
		void Drawer<FrameBorders>::GetInnerSize(FrameBorders& borders, float& w, float& h)
		{
		}

		template <>
		void Drawer<FrameBorders>::GetOuterSize(FrameBorders& borders, float& w, float& h)
		{
		}
		
		template <>
		void Drawer<Text>::Draw(Text& text, float x, float y, float w, float h)
		{
			::Window::GUI::TextRenderer::RenderedText RenderedInfo;
			text.font.RenderText(text.Get(), RenderedInfo, Workspace::metrics.GetDPI());
			
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
		void Drawer<Text>::GetInnerSize(Text& text, float& w, float& h)
		{
			w = h = 0;
		}

		template <>
		void Drawer<Text>::GetOuterSize(Text& text, float& w, float& h)
		{
			::Window::GUI::TextRenderer::TextDimension dims;
			dims = text.font.GetTextSize(text.Get(), Workspace::metrics.GetDPI());
			w = dims.w;
			h = dims.h;
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
		void Drawer<T>::Draw(T& info, float x, float y, float w, float h) {}

		template <typename T>
		void Drawer<T>::GetOuterSize(T& info, float& w, float& h) {}
		
		template <typename T>
		void Drawer<T>::GetInnerSize(T& info, float& w, float& h) {}
	}
}
