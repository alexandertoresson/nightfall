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

#ifndef COMPOSITOR_H_PRE
#define COMPOSITOR_H_PRE 

#ifdef DEBUG_DEP
	#warning "compositor.h-pre"
#endif

#include "gc_ptr.h"

namespace GUI
{	
	typedef void(*universalCallback)(void*);
	
	class Event;
	class Frame;
	class Component;
	class Container;

	/**
	 * Handles scaling calculations and proponanlity corrections. Handles all types of aspects and monitor resolution.
	 * Equation: h_ref / sqrt( t * t / (a * a + 1) ) ) * 96, h_ref is the inch reference in height, a = spect
	 */
	class Metrics
	{
		private:
			float dpi;     /**< dpi factor */
			float monitor; /**< Monitor inch size */
			float monitoraspect;
			float monitor_w; /**< Calculated from the monitors diagonal size + aspect */
			float monitor_h; /**< Calculated from the monitors diagonal size + aspect */
			
		protected:
			float dotsperwidth;  /**< dots per x-axis */
			float dotsperheight; /**< dots per y-axis */
			int   native_w;
			int   native_h;
			int   current_w;
			int   current_h;
			
			/**
			 * Coordinate-system calculation.
			 * @param monitorsize	Holds the monitor inch size
			 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
			 */
			void CalculateCoordinateSystem(float monitorsize, float monitoraspect, bool stretched = false);
			
			/**
			 * Coordinate-system calculation.
			 * @param dpi			Specific resolution
			 * @param monitorsize	Holds the monitor inch size
			 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
			 */
			void CalculateCoordinateSystem(float dpi, float monitorsize, float monitoraspect);
			
			/**
			 * Coordinate-system calculation when application is in a window.
			 * @param monitorsize		monitor inch size
			 * @param monitor_px_width	monitor native pixel x-axis resolution
			 * @param monitor_px_height monitor native pixel y-axis resolution
			 */
			void CalculateCoordinateSystem();
			
			/**
			 * Coordinate-system calculation when application is in a window with specific resolution.
			 * @param dpi				Specific resolution
			 * @param monitorsize		monitor inch size
			 * @param monitor_px_width	monitor native pixel x-axis resolution
			 * @param monitor_px_height monitor native pixel y-axis resolution
			 */
			void CalculateCoordinateSystem(float dpi);
			
		public:
			Metrics(int native_w, int native_h, int current_w, int current_h, bool fullscreen, float monitorsize, bool stretched = false);
		
			float GetDPI();
			void SetDPI(float dpi);
		
			void TranslatePointToPixel(float& pt_x, float& pt_y);
			void TranslatePixelToPoint(float& px_x, float& px_y);
			
			void AlignToPixel(float& x, float& y);
			
			void ScaleFactorInch(float& w, float& h);
			void ScaleFactorCM(float& w, float& h);
			
			/**
			 *	from the given
			 */
			void Scale();
			void Revert();
			
			void SetMetrics(gc_ptr<Metrics> met);

			void shade() {}
	};
	
	/**
	 *	Symbolises a Window event
	 */
	struct WindowEvent
	{
		/**
		 *	Window event types
		 */
		enum WindowEventType
		{
			FOCUS,   /**< Window/Controll has recieved focus */
			NOFOCUS, /**< Window/Controll has lost focus     */
			RESIZE,  /**< Resize event, call LayoutManager   */
			CLOSED,  /**< Window has been closed             */
			DROPPED  /**< Drag and drop performed            */
		};
		
		WindowEventType type;
		void* data;
		
		struct {
			float x;
			float y;
			float w;
			float h;
		} bounds;
	};
	
	class TrackingArea
	{
		public:
			enum
			{
				MOVE,
				DRAG_DROP,
				TEXT
			} AreaType;
	};
	
}

#endif

