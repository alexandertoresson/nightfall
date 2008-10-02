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
#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__ 

#ifdef DEBUG_DEP
	#warning "compositor.h"
#endif

#include "compositor-pre.h"

#include "core.h"
#include "window.h"

#include <string>
#include <sdlheader.h>
#include <list>
#include <vector>

namespace GUI
{	
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
			void calculateCoordinateSystem(float monitorsize, float monitoraspect, bool streched = false);
			
			/**
			 * Coordinate-system calculation.
			 * @param dpi			Specific resolution
			 * @param monitorsize	Holds the monitor inch size
			 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
			 */
			void calculateCoordinateSystem(float dpi, float monitorsize, float monitoraspect);
			
			/**
			 * Coordinate-system calculation when application is in a window.
			 * @param monitorsize		monitor inch size
			 * @param monitor_px_width	monitor native pixel x-axis resolution
			 * @param monitor_px_height monitor native pixel y-axis resolution
			 */
			void calculateCoordinateSystem();
			
			/**
			 * Coordinate-system calculation when application is in a window with specific resolution.
			 * @param dpi				Specific resolution
			 * @param monitorsize		monitor inch size
			 * @param monitor_px_width	monitor native pixel x-axis resolution
			 * @param monitor_px_height monitor native pixel y-axis resolution
			 */
			void calculateCoordinateSystem(float dpi);
			
		public:
			Metrics(int native_w, int native_h, int current_w, int current_h, bool fullscreen, float monitorsize, bool streched = false);
		
			float getDPI();
			void setDPI(float dpi);
		
			void translatePointToPixel(float& pt_x, float& pt_y);
			void translatePixelToPoint(float& px_x, float& px_y);
			
			void alignToPixel(float& x, float& y);
			
			void scaleFactorInch(float& w, float& h);
			void scaleFactorCM(float& w, float& h);
			
			/**
			 *	from the given
			 */
			void scale();
			void revert();
			
			void setMetrics(Metrics* met);
	};
	
	/*
		Fundamental types of GUI objects:
			Window, Dialog and Tooltips
			
		The windowsystem has three z-layers
		Bottom (Gaming parts)
		Normal (InGame chat dialogs)
		Top (Dialog)
	*/
	
	class Component;

	namespace Helper
	{
		typedef enum {
			INFORMATION,
			EXCLAMATION,
			CRITICAL,
			QUESTION
		} DialogType;  /**< Dialog types */
		
		typedef enum
		{
			B_OK,
			B_OKCANCEL,
			B_YESNO,
			B_USERDEFINED  /**< Let's user decide labels */
		} DialogStandardButton; 
	
		struct DialogButtons
		{
			DialogStandardButton type;
			std::vector<std::string> names;
			int focused;
			
			DialogButtons()
			{
				focused = -1;
				type = B_USERDEFINED;
			}
			
			DialogButtons(int buttoncount)
			{
				names = std::vector<std::string>(buttoncount);
				focused = -1;
				type = B_USERDEFINED;
			}
			
			DialogButtons(DialogStandardButton standard)
			{
				switch(standard)
				{
					case B_OK:
						names.push_back("OK");
						focused = 0;
						type = B_OK;
						break;
					case B_OKCANCEL:
						names.push_back("OK");
						names.push_back("Cancel");
						focused = 1;
						type = B_OKCANCEL;
						break;
					case B_YESNO:
						names.push_back("Yes");
						names.push_back("No");
						focused = 1;
						type = B_YESNO;
						break;
					default:
						focused = -1;
						break;
				}
			}
		};
		
		struct DialogParameters
		{
			DialogType diagtype;
			
			Window* parent; /**< if this is NULL it is considered to be free, and will behave as a normal window. */
			bool critical;  /**< if this is true, this window will be always on top and no other windows can be touched. only one at a time. */
			
			std::string title;
			std::string message;
			
			DialogButtons button;
			
			DialogParameters()
			{
				this->diagtype = QUESTION;
				this->parent = NULL;
				this->critical = false;
			}
			
			DialogParameters(DialogType diagType, std::string message, std::string title)
			{
				this->parent = NULL;
				this->critical = false;
				this->diagtype = diagtype;
				this->title = title;
				this->message = message;
			}
			
			DialogParameters(DialogType diagType, std::string message, std::string title, DialogButtons button)
			{
				this->parent = NULL;
				this->critical = false;
				this->diagtype = diagtype;
				this->title = title;
				this->message = message;
				this->button = button;
			}
			
			DialogParameters(Window* parent, DialogType diagType, std::string message, std::string title, DialogButtons button)
			{
				this->parent = parent;
				this->critical = false;
				this->diagtype = diagtype;
				this->message = message;
				this->title = title;
				this->button = button;
			}
			
		};
	
		int createTooltip(int id, std::string text, float x, float y);
		int createTooltipEx(int id, Component comp, float x, float y);
		void removeTooltip(int id);
		void moveTooltip(int id, float x, float y);
		
		void createDialog(DialogParameters paramDiag, universalCallback callback);
	}
	
	/**
	 *	Symbolises a Window event
	 */
	struct WindowEvent
	{
		/**
		 *	Window event types
		 */
		enum windowEventType
		{
			FOCUS,   /**< Window/Controll has recieved focus */
			NOFOCUS, /**< Window/Controll has lost focus     */
			RESIZE,  /**< Resize event, call LayoutManager   */
			CLOSED,  /**< Window has been closed             */
			DROPED   /**< Drag and drop performed            */
		};
		
		windowEventType type;
		void* data;
		
		struct {
			float x;
			float y;
			float w;
			float h;
		} bounds;
	};
	
	/**
	 *	Eventsystem standard
	 */
	class Event
	{
		public:
			
			/**
			 * Mouse event
			 * @param evt MouseEvent structure that contains the actual event information.
			 * @param handled This notifies the calling function if the mouse event is handled or not (if the mouse point is within bounds)
			 * @see Core::KeyboardEvent
			 */
			virtual void event(Core::MouseEvent evt, bool& handled) { handled = false; }
			
			/**
			 * Keyboard event
			 * @param evt KeyboardEvent structure that contains the keyboard event information
			 * @see Core::KeyboardEvent
			 */
			virtual void event(Core::KeyboardEvent evt) {}
			
			/**
			 * Window event
			 * @param evt WindowEvent structure that contains information about the window event.
			 * @see Core::WindowEvent
			 */
			virtual void event(WindowEvent evt) {}
			
			/**
			 * Window event
			* @param evt WindowEvent structure that contains information about the window event.
			 * @see Core::WindowEvent
			 */
			 
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
	
	/*
				Layout-mangement
											*/
/*	class LayoutConstraint
	{
		public:
			(*void   setAbsolute(Bounds coordinates);
			Bounds getAbsolute(void);*)
	};
	
	class Layout
	{
		public:
			struct Bounds
			{
				float x;
				float y;
				float w;
				float h;
				
				Bounds()
				{
					this->x = 0.0f;
					this->y = 0.0f;
					this->w = 0.0f;
					this->h = 0.0f;
				}
				
				Bounds(float x, float y, float w, float h)
				{
					this->x = x;
					this->y = y;
					this->w = w;
					this->h = h;
				}  
			};
			
			LayoutConstraint getConstraint(int id);
			void setConstraint(int id, LayoutConstraint constraint);
			void layout();
	};*/
	
	
	
	/*
					Component
											*/
	typedef std::list<Component*>::iterator componentHandle;
	
	class Component : public Event
	{
		protected:
			
			struct Bounds
			{
				float x;
				float y;
				float w;
				float h;
				
				Bounds()
				{
					x = 0.0f;
					y = 0.0f;
					w = 1.0f;
					h = 1.0f;
				}
			} dimensions;
	
			Metrics*   metrics;
			
			bool visible;
			
			virtual void paint();
		public:
			Component();
			Component(float w, float h);
			Component(float x, float y, float w, float h);
			~Component();
			
			virtual bool isInsideArea(float x, float y);
			
			void setVisible(bool state);
			
			virtual void setSize(float w, float h);
			virtual void setPosition(float x, float y);
			
			virtual void event(Core::MouseEvent evt, bool& handled);
			virtual void event(Core::KeyboardEvent evt);
			virtual void event(WindowEvent evt);
			
			void paintComponent();
			
			friend class Workspace;
	};
	
	class Container : public Component
	{
		protected:
			std::list<Component*> components;
			virtual void paint();
		public:
			componentHandle add(Component* component);
			Component* get(componentHandle handle);
			void remove(componentHandle handle);
			void clear();
			bool isEmpty();
			void paintAll();
	};

	class Frame : public Container
	{
		private:
			virtual void paintBackground() {};
			virtual void paintGlass() {};
		protected:
			typedef enum {
				DEFAULT,
				CENTERPARENT,
				CENTERSCREEN,
				USERDEFINED
			} StartLocation;
			
			typedef enum {
					BOTTOM = 0,
					STANDARD = 1,
					POPUP = 2,
					END = 3
			} LayerIndex;
		
			struct WindowParameter
			{
				LayerIndex layer;
				StartLocation location;
				Frame* parent;
				Metrics* met;
			};
			
			StartLocation	start;
			WindowParameter parameters;
			Bounds			windowDimensions;
			
			void paint();
		public:
			Frame(float x, float y, float w, float h, WindowParameter param);
			Frame(float w, float h, WindowParameter param);
			Frame(WindowParameter param);
			~Frame();
			
			friend class Workspace;
	};
	
	typedef std::list<Frame*>::iterator windowHandle;
	
	/* Window-Management */	
	class Workspace : Event
	{
		private:
			std::list<Frame*> win;
			std::list<Frame*>::iterator bottom; /* layer-pointers */
			std::list<Frame*>::iterator standard;
			std::list<Frame*>::iterator popup;
			
		protected:
			Metrics* met;
		
			void paintWindows(Frame::LayerIndex layer);
			void paintTooltips();
			void paintDialogs();
			
		public:
			Workspace(int native_w, int native_h, float monitorsize, bool streched);
			
			void paint();
			
			windowHandle add(Frame* elem);
			void remove(windowHandle elem);
			void remove(Frame* elem); /* SLOW! use remove(componentHandle) */
			void positionate(windowHandle elem, Frame::LayerIndex z);
	};
}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#endif
