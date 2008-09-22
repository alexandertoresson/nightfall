/*
 *  layoutmanager.h
 *
 *  Created by Marcus Klang on 2008-01-29.
 *
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
			CLOSED   /**< Window has been closed             */
		};
		
		windowEventType type;
		
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
	};
	
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
			std::string* names;
			int namesLen;
			int focused;
			
			DialogButtons()
			{
				names = NULL;
				focused = -1;
				type = B_USERDEFINED;
			}
			
			DialogButtons(int buttoncount)
			{
				names = new std::string[buttoncount];
				namesLen = buttoncount;
				focused = -1;
				type = B_USERDEFINED;
			}
			
			DialogButtons(DialogStandardButton standard)
			{
				switch(standard)
				{
					case B_OK:
						names = new std::string[1];
						names[0] = std::string("OK");
						namesLen = 1;
						focused = 0;
						type = B_OK;
						break;
					case B_OKCANCEL:
						names = new std::string[2];
						names[0] = std::string("OK");
						names[1] = std::string("Cancel");
						namesLen = 2;
						focused = 1;
						type = B_OKCANCEL;
						break;
					case B_YESNO:
						names = new std::string[2];
						names[0] = std::string("Yes");
						names[1] = std::string("No");
						namesLen = 2;
						focused = 1;
						type = B_YESNO;
						break;
					default:
						names = NULL;
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
	
	class Container;
	class Layout;
	
	class Component : public Event
	{
	protected:
		Layout*    layoutmgr;
		Container* container;
		Metrics*   metrics;
		//Workspace* master;
		
		int id;
		float x;
		float y;
		float w;
		float h;
		
	public:
		Component();
		Component(float w, float h);
		Component(float x, float y, float w, float h);
		
		bool isInsideArea(float x, float y);
		
		virtual void setParameter(void* param) {};
		void setLayoutManager(Layout* layout);
		
		virtual void event(Core::MouseEvent evt, bool& handled);
		virtual void event(Core::KeyboardEvent evt);
		virtual void event(WindowEvent evt);
		
		virtual void paint();
		friend class Workspace;
	};
	
	class Frame;
	
	/* Window-Management */	
	class Workspace : Event
	{
		public:
			typedef enum {
				BOTTOM,
				USERSPACE,
				ALWAYSONTOP
			} LayerIndex;
			
			typedef enum {
				DEFAULT,
				CENTERPARENT,
				CENTERSCREEN,
				USERDEFINED
			} StartLocation;
			
		private:
			std::list<Frame*> win;
		protected:
			Metrics* met;
		
			void paintWindows(LayerIndex layer);
			void paintTooltips();
			void paintDialogs();
		public:
			Workspace(int native_w, int native_h, float monitorsize, bool streched);
			
			void paint();
			
			std::list<Component*>::iterator add(Component elem);
			void remove(std::list<Component*>::iterator elem);
			void remove(Component* elem);
			void positionate(Component elem, LayerIndex z);
	};
	
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
	
	class LayoutConstraint
	{
		public:
			void   setAbsolute(Bounds coordinates);
			Bounds getAbsolute(void);
	};
	
	class Layout
	{
		public:
			LayoutConstraint getConstraint(int id);
			void setConstraint(int id, LayoutConstraint constraint);
			void layout();
	};
	
	class Container
	{
		protected:
			std::vector<Component*> components;
		public:
			void add(Component* component, void* param);
			void remove(Component* component);
			void clear();
	};
	
	class Frame : public Component
	{
		protected:
			struct WindowParameter
			{
				Workspace::LayerIndex layer;
				
				Workspace::StartLocation location;
				
				Frame* parent;
			};
		
			WindowParameter parameters;
			
		public:
			Frame(float x, float y, float w, float h, WindowParameter param);
			Frame(float w, float h, WindowParameter param);
			Frame(WindowParameter param);
			
			void setSize(float w, float h);
			void setPosition(float x, float y);
		
		friend class Workspace;
	};

}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#endif
