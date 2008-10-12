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
	Metrics::Metrics(int native_w, int native_h, int current_w, int current_h, bool fullscreen, float monitorsize, bool streched)
	{
		this->native_w = native_w;
		this->native_h = native_h;
		this->current_w = current_w;
		this->current_h = current_h;
		this->monitor = monitorsize;
		this->monitoraspect = (float)native_w / native_h;
		
		if(fullscreen)
		{
			calculateCoordinateSystem(monitorsize, monitoraspect, streched);
		}
		else
		{
			calculateCoordinateSystem();
		}
	}
	
	/**
	 * Coordinate-system calculation.
	 * @param monitorsize	Holds the monitor inch size
	 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
	 */
	void Metrics::calculateCoordinateSystem(float monitorsize, float monitoraspect, bool streched)
	{
		//monitorsize.
		double w = (monitoraspect * monitorsize) / sqrt(1 + monitoraspect * monitoraspect);
		double h = sqrt(monitorsize * monitorsize - w * w);
		
		double resolution_aspect = (float)current_w / current_h;
		double window_width = (native_w / monitoraspect) * resolution_aspect;
		
		this->monitor_w = (float)w;
		this->monitor_h = (float)h;
		
		if(!streched)
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
	void Metrics::calculateCoordinateSystem(float dpi, float monitorsize, float monitoraspect)
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
	void Metrics::calculateCoordinateSystem()
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
	void Metrics::calculateCoordinateSystem(float dpi)
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
	void Metrics::scale()
	{
		glScalef(1.0f / this->dotsperwidth, 1.0f / this->dotsperheight, 0.0f);
		glPushMatrix();
	}
	
	void Metrics::revert()
	{
		glPopMatrix();
	}

	float Metrics::getDPI()
	{
		return this->dpi;
	}
	
	void Metrics::setDPI(float dpi)
	{
		this->dpi = dpi;
	}
	
	/* Conversion method for getting the pixel at the point coordinate */
	void Metrics::translatePointToPixel(float& pt_x, float& pt_y)
	{
		pt_x = floor(((pt_x / this->dotsperwidth) / this->monitor_w * this->current_w) + 0.5);
		pt_y = floor(((pt_y / this->dotsperheight) / this->monitor_h * this->current_h) + 0.5);
	}

	/* Conversion method for getting the pixel at the pixel coordinate */
	void Metrics::translatePixelToPoint(float& px_x, float& px_y)
	{
		px_x = (px_x / this->current_w) * this->monitor_w * this->dotsperwidth;
		px_y = (px_y / this->current_h) * this->monitor_h * this->dotsperheight;
	}
	
	/* Conversion method for translating point coordinate into the real world inch coordinate */
	void Metrics::scaleFactorInch(float& w, float& h)
	{
		w = w * this->dpi;
		h = h * this->dpi;
	}

	/* Conversion method for translating point coordinate into the real world centimeter coordinate */
	void Metrics::scaleFactorCM(float& w, float& h)
	{
		w = (w / 2.54f) * this->dpi;
		h = (h / 2.54f) * this->dpi;
	}
	
	/* alignment to the closest pixel */
	void Metrics::alignToPixel(float& x, float& y)
	{
		translatePointToPixel(x,y);
		translatePixelToPoint(x,y);
	}
	
	/* a way to copy the settings of an existing Metrics */
	void Metrics::setMetrics(Metrics* met)
	{
		
	}
	
	/*******************************************************************/
	/** COMPONENT ******************************************************/
	/*******************************************************************/
	Component::Component() : aspectRatio(0.0), vAdjustment(V_ADJUSTMENT_NONE), hAdjustment(H_ADJUSTMENT_NONE), visible(true), needsRelayout(true)
	{
		for (unsigned i = 0; i < ANCHOR_NUM; i++)
		{
			anchors[i] = false;
		}
	}
	
	Component::Component(float w, float h)
	{
		dimensions.w = w;
		dimensions.h = h; 
	}
	
	Component::Component(float x, float y, float w, float h)
	{
		dimensions.x = x;
		dimensions.y = y; 
		dimensions.w = w;
		dimensions.h = h;
	}
	
	bool Component::isInsideArea(float x, float y)
	{
		if(dimensions.x >= this->dimensions.x && x <= (this->dimensions.x + this->dimensions.w) &&
		   dimensions.y >= this->dimensions.y && dimensions.y <= (this->dimensions.y + this->dimensions.h) )
		{
			return true;
		}
		
		return false;
	}
	
	void Component::setSize(float w, float h)
	{
		this->dimensions.w = w;
		this->dimensions.h = h;
	}

	void Component::setPosition(float x, float y)
	{
		this->dimensions.x = x;
		this->dimensions.y = y;
	}
	
	void Component::event(Core::MouseEvent evt, bool& handled)
	{
		handled = false;
	}
	void Component::event(Core::KeyboardEvent evt)
	{
		
	}
	
	void Component::event(WindowEvent evt)
	{
		
	}
	
	/** COMPONENT::CONTAINER ******************************************************/
	componentHandle Container::add(Component* component)
	{
		components.push_back(component);
		componentHandle last = components.end();
		return --last;
	}
	
	void Component::paintComponent()
	{
		//1. paint itself
		paint();
		//2. paint all controls in it.
	}
	
	void Component::paint()
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
	
	void Component::layout()
	{
		
	}
}
