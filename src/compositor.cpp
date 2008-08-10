/*
 *  compositer.cpp
 *  Nightfall
 *
 *  Created by Marcus Klang on 2008-04-26.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
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
	Component::Component()
	{
		this->x = 0;
		this->y = 0;
		this->w = w;
		this->h = h;
	}
	
	Component::Component(float w, float h)
	{
		this->x = 0;
		this->y = 0;
		this->w = w;
		this->h = h;
	}
	
	Component::Component(float x, float y, float w, float h)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	
	bool Component::isInsideArea(float x, float y)
	{
		if(x >= this->x && x <= (this->x + this->w) &&
		   y >= this->y && y <= (this->y + this->h) )
		{
			return true;
		}
		
		return false;
	}
	
	void Component::setLayoutManager(Layout* layout)
	{
		this->layoutmgr = layout;
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
	
	void Component::paint()
	{
		glPushMatrix();
		glScalef(this->w, this->h,0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		
		glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();
		
		glPopMatrix();
	}
}
