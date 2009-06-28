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
#include "compositor.h"
#include <cmath>

namespace GUI
{
	/*******************************************************************/
	/** METRICS ********************************************************/
	/*******************************************************************/
	Metrics::Metrics(int native_w, int native_h, int current_w, int current_h, bool fullscreen, float monitorsize, bool stretched)
	{
		this->native_w = native_w;
		this->native_h = native_h;
		this->current_w = current_w;
		this->current_h = current_h;
		this->monitor = monitorsize;
		this->monitoraspect = (float)native_w / native_h;
		
		if(fullscreen)
		{
			CalculateCoordinateSystem(monitorsize, monitoraspect, stretched);
		}
		else
		{
			CalculateCoordinateSystem();
		}
	}
	
	/**
	 * Coordinate-system calculation.
	 * @param monitorsize	Holds the monitor inch size
	 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
	 */
	void Metrics::CalculateCoordinateSystem(float monitorsize, float monitoraspect, bool stretched)
	{
		//monitorsize.
		double w = (monitoraspect * monitorsize) / sqrt(1 + monitoraspect * monitoraspect);
		double h = sqrt(monitorsize * monitorsize - w * w);
		
		double resolution_aspect = (float)current_w / current_h;
		double window_width = (native_w / monitoraspect) * resolution_aspect;
		
		this->monitor_w = (float)w;
		this->monitor_h = (float)h;
		
		if(!stretched)
			this->dpi = window_width / w;
		else
			this->dpi = this->native_w / w; //this can be based on the accurate description.
		
		this->dotsperwidth = w * this->dpi;
		this->dotsperheight = h * this->dpi;
	}
	
	/**
	 * Coordinate-system calculation.
	 * @param dpi			Specific resolution
	 * @param monitorsize	Holds the monitor inch size
	 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
	 */
	void Metrics::CalculateCoordinateSystem(float dpi, float monitorsize, float monitoraspect)
	{
		//monitorsize.
		double w = (monitoraspect * monitorsize) / sqrt(1 + monitoraspect * monitoraspect);
		double h = sqrt(monitorsize * monitorsize - w * w);
		
		this->monitor_w = (float)w;
		this->monitor_h = (float)h;
		
		this->dpi = dpi;
		this->dotsperwidth = w * this->dpi;
		this->dotsperheight = h * this->dpi;
	}
	
	/**
	 * Coordinate-system calculation when application is in a window.
	 * @param monitorsize		monitor inch size
	 * @param monitor_px_width	monitor native pixel x-axis resolution
	 * @param monitor_px_height monitor native pixel y-axis resolution
	 */
	void Metrics::CalculateCoordinateSystem()
	{
		//reference 15" at 1024x768.
		double monitoraspect = 4.0 / 3.0;
		double monitorsize   = 15;

		double w = (monitoraspect * monitorsize) / sqrt(1 + monitoraspect * monitoraspect);
		double h = sqrt(monitorsize * monitorsize - w * w);
		
		this->monitor_w = (float)w;
		this->monitor_h = (float)h;
		
		this->dpi = 1024 / w;
		this->dotsperwidth = w * this->dpi;
		this->dotsperheight = h * this->dpi;
	}
	
	/**
	 * Coordinate-system calculation when application is in a window with specific resolution.
	 * @param dpi				Specific resolution
	 * @param monitorsize		monitor inch size
	 * @param monitor_px_width	monitor native pixel x-axis resolution
	 * @param monitor_px_height monitor native pixel y-axis resolution
	 */
	void Metrics::CalculateCoordinateSystem(float dpi)
	{
		//reference 15" at 1024x768.
		double monitoraspect = 4.0 / 3.0;
		double monitorsize   = 15;
		
		double w = (monitoraspect * monitorsize) / sqrt(1 + monitoraspect * monitoraspect);
		double h = sqrt(monitorsize * monitorsize - w * w);
		
		this->monitor_w = (float)w;
		this->monitor_h = (float)h;
		
		this->dpi = dpi;
		this->dotsperwidth = w * this->dpi;
		this->dotsperheight = h * this->dpi;
	}
	
	/**
	 *	from the given coordinatesyste, assumes surface to be 1 wide and 1 high
	 */
	void Metrics::Scale()
	{
		glScalef(1.0f / this->dotsperwidth, 1.0f / this->dotsperheight, 0.0f);
		glPushMatrix();
	}
	
	void Metrics::Revert()
	{
		glPopMatrix();
	}

	float Metrics::GetDPI()
	{
		return this->dpi;
	}
	
	void Metrics::SetDPI(float dpi)
	{
		this->dpi = dpi;
	}
	
	/* Conversion method for getting the pixel at the point coordinate */
	void Metrics::TranslatePointToPixel(float& pt_x, float& pt_y)
	{
		pt_x = floor(((pt_x / this->dotsperwidth) / this->monitor_w * this->current_w) + 0.5);
		pt_y = floor(((pt_y / this->dotsperheight) / this->monitor_h * this->current_h) + 0.5);
	}

	/* Conversion method for getting the pixel at the pixel coordinate */
	void Metrics::TranslatePixelToPoint(float& px_x, float& px_y)
	{
		px_x = (px_x / this->current_w) * this->monitor_w * this->dotsperwidth;
		px_y = (px_y / this->current_h) * this->monitor_h * this->dotsperheight;
	}
	
	/* Conversion method for translating point coordinate into the real world inch coordinate */
	void Metrics::ScaleFactorInch(float& w, float& h)
	{
		w = w * this->dpi;
		h = h * this->dpi;
	}

	/* Conversion method for translating point coordinate into the real world centimeter coordinate */
	void Metrics::ScaleFactorCM(float& w, float& h)
	{
		w = (w / 2.54f) * this->dpi;
		h = (h / 2.54f) * this->dpi;
	}
	
	/* alignment to the closest pixel */
	void Metrics::AlignToPixel(float& x, float& y)
	{
		TranslatePointToPixel(x,y);
		TranslatePixelToPoint(x,y);
	}
	
	/* a way to copy the settings of an existing Metrics */
	void Metrics::SetMetrics(gc_ptr<Metrics> met)
	{
		
	}
	
	/*******************************************************************/
	/** COMPONENT ******************************************************/
	/*******************************************************************/
	Component::Component(float x, float y, float w, float h) : dimensions(x, y, w, h), min(0.0f, 0.0f), max(1.0f, 1.0f), aspectRatio(0.0), vAdjustment(V_ADJUSTMENT_NONE), hAdjustment(H_ADJUSTMENT_NONE), visible(true), needsRelayout(true)
	{
		for (unsigned i = 0; i < ANCHOR_NUM; i++)
		{
			anchors[i] = false;
		}
	}
	
	Component::~Component()
	{
		
	}

	bool Component::IsInsideArea(float x, float y)
	{
		if(dimensions.x >= this->dimensions.x && x <= (this->dimensions.x + this->dimensions.w) &&
		   dimensions.y >= this->dimensions.y && dimensions.y <= (this->dimensions.y + this->dimensions.h) )
		{
			return true;
		}
		
		return false;
	}
	
	void Component::SetVisible(bool state)
	{
		if (visible != state)
		{
			visible = state;
			ScheduleRelayout();
		}
	}

	void Component::SetMinSize(float w, float h)
	{
		this->min.w = w;
		this->min.h = h;
		ScheduleRelayout();
	}

	void Component::SetMaxSize(float w, float h)
	{
		this->max.w = w;
		this->max.h = h;
		ScheduleRelayout();
	}

	void Component::SetSize(float w, float h)
	{
		this->dimensions.w = w;
		this->dimensions.h = h;
		ScheduleRelayout();
	}

	void Component::SetAspectRatio(float r)
	{
		aspectRatio = r;
		ScheduleRelayout();
	}

	void Component::SetPosition(float x, float y)
	{
		this->dimensions.x = x;
		this->dimensions.y = y;
		ScheduleRelayout();
	}

	void Component::SetVerticalAdjustment(VerticalAdjustment vAdjustment)
	{
		this->vAdjustment = vAdjustment;
		ScheduleRelayout();
	}
	
	void Component::SetHorizontalAdjustment(HorizontalAdjustment hAdjustment)
	{
		this->hAdjustment = hAdjustment;
		ScheduleRelayout();
	}

	void Component::SetAnchor(Anchor anchor, bool enabled)
	{
		anchors[anchor] = enabled;
		ScheduleRelayout();
	}

	void Component::SetMargin(float top, float right, float bottom, float left)
	{
		margin.top = top;
		margin.right = right;
		margin.bottom = bottom;
		margin.left = left;
		ScheduleRelayout();
	}

	void Component::PaintComponent()
	{
		Paint();
	}
	
	void Component::Paint()
	{
		glPushMatrix();
		glScalef(this->dimensions.w, this->dimensions.h,0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		
		glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();
		
		glPopMatrix();
	}
	
	void Component::Layout()
	{
	}

	/** CONTAINER ******************************************************/
	
	Container::Container(float x, float y, float w, float h) :
		Component(x, y, w, h),
		innerBorders(GetRef()),
		outerBorders(GetRef()),
		vertScrollBar(GetRef(), 0.0f, 1.0f, 0.0f, 1.0f, false, ThemeEngine::ScrollBar::DIRECTION_VERTICAL),
		horizScrollBar(GetRef(), 0.0f, 1.0f, 0.0f, 1.0f, false, ThemeEngine::ScrollBar::DIRECTION_HORIZONTAL)
	{
		
	}

	void Container::LayoutAll()
	{
		for (ComponentHandle it = components.begin(); it != components.end(); it++)
		{
			(*it)->Layout();
		}
	}
	
	void Container::PostLayout()
	{
		ApplyAdjustment();
		ApplyAnchors();
	}

	void Container::Layout()
	{
		LayoutAll();
		PostLayout();
	}
	
	void Component::ScheduleRelayout()
	{
		if (!needsRelayout)
		{
			needsRelayout = true;
/*			if (parent)
			{
				parent->ScheduleRelayout();
			}*/
		}
	}

	void Container::ApplyAnchors()
	{
		
	}

	void Container::ApplyAdjustment()
	{
		float minX, minY, maxX, maxY;

		if (vAdjustment == V_ADJUSTMENT_NONE && hAdjustment == H_ADJUSTMENT_NONE)
		{
			return;
		}

		minX = 10000.0f;
		minY = 10000.0f;
		maxX = -10000.0f;
		maxY = -10000.0f;
		for (ComponentHandle it = components.begin(); it != components.end(); it++)
		{
			if ((*it)->dimensions.x < minX)
				minX = (*it)->dimensions.x;
			if ((*it)->dimensions.y < minY)
				minY = (*it)->dimensions.y;
			if ((*it)->dimensions.x + (*it)->dimensions.w  > maxX)
				maxX = (*it)->dimensions.x + (*it)->dimensions.w;
			if ((*it)->dimensions.y + (*it)->dimensions.h  > maxX)
				maxX = (*it)->dimensions.y + (*it)->dimensions.h;
		}

		float dX = 0.0f, dY = 0.0f;

		switch (vAdjustment)
		{
			case V_ADJUSTMENT_LEFT:
				dX = minX - padding.left;
				break;
			case V_ADJUSTMENT_MIDDLE:
				dX = ((minX + maxX) - (padding.left + dimensions.w - padding.right)) / 2;
				break;
			case V_ADJUSTMENT_RIGHT:
				dX = maxX - (dimensions.w - padding.right);
				break;
			case V_ADJUSTMENT_NONE:
				dX = 0.0f;
				break;
		}
		
		switch (hAdjustment)
		{
			case H_ADJUSTMENT_TOP:
				dY = minY - padding.top;
				break;
			case H_ADJUSTMENT_MIDDLE:
				dY = ((minY + maxY) - (padding.top + dimensions.h - padding.bottom)) / 2;
				break;
			case H_ADJUSTMENT_BOTTOM:
				dY = maxY - (dimensions.h - padding.bottom);
				break;
			case H_ADJUSTMENT_NONE:
				dY = 0.0f;
				break;
		}
		
		for (ComponentHandle it = components.begin(); it != components.end(); it++)
		{
			(*it)->dimensions.x += dX;
			(*it)->dimensions.y += dY;
		}

	}
	
	void Container::SetPadding(float top, float right, float bottom, float left)
	{
		padding.top = top;
		padding.right = right;
		padding.bottom = bottom;
		padding.left = left;
		ScheduleRelayout();
	}

	void Container::Insert(gc_ptr<Component> component, int position)
	{
		if (component->parent)
		{
			component->parent->Remove(component);
		}
		if (position >= 0 && position < (signed) components.size())
		{
			ComponentHandle it = components.begin();
			for (int i = 0; i < position; i++, it++) ;
			component->handle = components.insert(it, component);
		}
		else
		{
			components.push_back(component);
			component->handle = --components.end();
		}
		component->parent = GetRef();
	}
	
	void Container::Add(gc_ptr<Component> component)
	{
		Insert(component, -1);
	}
	
	void Container::Remove(gc_ptr<Component> component)
	{
		components.erase(component->handle);
		component->parent = NULL;
	}

	void Container::Clear()
	{
		while (components.size())
		{
			Remove(components.front());
		}
	}

	void Container::Paint()
	{
		PaintComponent();
		PaintAll();
	}
	
	void Container::PaintComponent()
	{
	}
	
	void Container::PaintAll()
	{
		for (ComponentHandle it = components.begin(); it != components.end(); it++)
		{
			(*it)->Paint();
		}
	}

	Frame::Frame(float x, float y, float w, float h, StartLocation location, LayerIndex layer) :
		Container(x, y, w, h),
		layer(layer),
		location(location),
		borders(GetRef(), true, "", Window::GUI::defaultFonts[2])
	{
		
	}

	void Frame::Paint()
	{
		float iw, ih;
		Workspace::theme->frameBordersDrawer->GetInnerSize(borders, dimensions.w, dimensions.h, iw, ih);
	}

	Workspace::Workspace() : frames(Frame::LAYER_END)
	{
	}
	
	void Workspace::Add(gc_ptr<Frame> elem)
	{
		if (elem->parent)
		{
			elem->parent->Remove(elem);
		}
		frames[elem->layer].push_back(elem);
		elem->handle = --frames[elem->layer].end();
		elem->parent = GetRef();
	}
	
	void Workspace::Remove(gc_ptr<Frame> elem)
	{
		frames[elem->layer].erase(elem->handle);
		elem->parent = NULL;
	}

	void Workspace::Paint()
	{
		for (std::vector<std::list<gc_ptr<Frame> > >::iterator it = frames.begin(); it != frames.end(); ++it)
		{
			for (std::list<gc_ptr<Frame> >::iterator it2 = it->begin(); it2 != it->end(); ++it2)
			{
				(*it2)->Paint();
			}
		}
	}

	void Workspace::shade()
	{
		for (int i = 0; i < Frame::LAYER_END; ++i)
		{
			gc_shade_container(frames[i]);
		}
	}

	void Workspace::InitializeWorkspaces(int native_w, int native_h, float monitorsize, bool stretched)
	{
		Workspace::metrics = GUI::Metrics(native_w, native_h, 1024, 768, true, monitorsize, stretched);
		Workspace::theme = new ThemeEngine::Theme();
	}
	
	Metrics Workspace::metrics(1024, 768, 1024, 768, true, 17.0f, false);
	gc_ptr<ThemeEngine::Theme> Workspace::theme = new ThemeEngine::Theme();
		
}
