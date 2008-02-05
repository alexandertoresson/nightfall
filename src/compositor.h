/*
 *  layoutmanager.h
 *
 *  Created by Marcus Klang on 2008-01-29.
 *
 */

#ifndef __COMPOSITOR_H_PRE__
#define __COMPOSITOR_H_PRE__ 

#ifdef DEBUG_DEP
	#warning "compositor.h-pre"
#endif

namespace GUI
{
	namespace Core
	{
		struct MouseEvent;
		struct KeyboardEvent;
		struct WindowEvent;
		
		struct Listener;
		
		typedef void(*mouse)(MouseEvent, void*);
		typedef void(*keyboard)(KeyboardEvent, void*);
		typedef void(*window)(WindowEvent, void*);
		typedef void(*render)(float diff, void*);
	}
	
	typedef void(*universalCallback)(void*);
	
	class Event;
	class Window;
	class Compositor;
	class Metrics;
	
}

#define __COMPOSITOR_H_PRE_END__

#include <iostream.h>
#include <string.h>
#include <sdlheader.h>

#endif



#ifndef __COMPOSITOR_H__
#define __COMPOSITOR_H__ 

#ifdef DEBUG_DEP
	#warning "compositor.h"
#endif

using namespace std;

namespace GUI
{
	
	/**
	  * Handles low-level communication with SDL, event creation, global key-attachments, state-switching.
	  */
	namespace Core
	{
		/**
		 * Symbolises a Mouse event
		 */
		struct MouseEvent
		{
			/**
			 * Mouse event types
			 */
			enum type
			{
				MOUSEUP,    /**< Mousebutton up   */
				MOUSEDOWN,  /**< Mousebutton down */
				MOUSEMOVE,  /**< Mousemotion      */
				MOUSESCROLL /**< Mouse scrolling  */
			};
			
			SDL_Event* pEvent;
			
			int button; /**< Button code for which button that were pressed */
			int state;  /**< Determines if scrolling if up(1) or down(-1)*/
			
			int x;      /**< Absolute x-axis mouse position */
			int y;      /**< Absolute y-axis mouse position */
		};
		
		/**
		 * Symbolises a Keyboard event
		 */
		struct KeyboardEvent
		{
			/**
			 *	Keyboard event types
			 */
			enum type
			{
				KEYUP,   /**< Key released */
				KEYDOWN  /**< Key pressed  */
			};
			
			int code;    /**< Charcode */
			SDLKey key;  /**< Keycode  */
		};
		
		/**
		 *	Symbolises a Window event
		 */
		struct WindowEvent
		{
			/**
			 *	Window event types
			 */
			enum type
			{
				FOCUS,   /**< Window/Controll has recieved focus */
				NOFOCUS, /**< Window/Controll has lost focus     */
				RESIZE,  /**< Resize event, call LayoutManager   */
				CLOSED   /**< Window has been closed             */
			};
			
			float x;
			float y;
			float w;
			float h;
		};
		
		/**
		 *	Handles the standard callback convention
		 */
		template <class T>
		struct Listener
		{
			T fptr;    /**< function to call   */
			void* tag; /**< standard parameter */
			
			Listener(T fptr, tag = NULL)
			{
				this->fptr = fptr;
				this->tag  = tag;
			}
		};
		
		/**
		 * The main loop of the program, handles state execution, termination and SDL event-loop, also paint scheduling.
		 */
		void mainLoop();
	}
	
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
			virtual void event(Core::WindowEvent evt) {}
	};
	
	/**
	 * Handles scaling calculations and proponanlity corrections. [RETHINK]
	 */
	class Metrics
	{
		private:
			float dpi;     /**< dpi factor */
			float monitor; /**< Monitor inch size */
		protected:
			float screenw;   /**< width of component */
			float screenh;   /**< height of component */
			float w;
			float h;
		
			/**
			 * sets the size of the component
			 */
			void setSize(float w, float h);
			
			/**
			 *	from the given
			 */
			void calculateFullscreenSize();
			void scaleCoordinateSystem();
			void revertCoordinateSystem();
		public:
			void setDPI(float dpi);
			float getDPI();
			
			void calculateDPI(float screenInch);
			void calculateLines(int lines);
		
			float getPixelUnit();
		
			float translatePointToPixel(float pt);
			float translatePixelToPoint(float px);
			
			void alignToPixel(float& x, float& y, float& w, float& h);
			
			void setMetrics(Metrics met);
	};
	
	class Compositor : public Metrics, public Event
	{
		protected:
			void event();
			void paint();
			Window* focused;
			
		public:
			
			struct DialogButtons
			{
				enum
				{
					B_OK,
					B_OKCANCEL,
					B_YESNO
				} type;
				
				DialogButtons()
				{
					names = NULL;
					focused = -1;
				}
				
				DialogButtons(int buttoncount)
				{
					names = new string[buttoncount];
					namesLen = buttoncount;
					focused = -1;
				}
				
				DialogButtons(type standard)
				{
					switch(standard)
					{
						case B_OK:
							names = new string[] {"OK"};
							namesLen = 1;
							focused = 0;
							break;
						case B_OKCANCEL:
							names = new string[] {"OK", "Cancel"}
							namesLen = 2;
							focused = 1;
							break;
						case B_YESNO:
							names = new string[] {"Yes", "No"}
							namesLen = 2;
							focused = 1;
							break;
					}
				}
				
				string* names;
				int namesLen;
				int focused;
			};
			
			struct DialogParameters
			{
				enum {
					INFORMATION,
					EXCLAMATION,
					CRITICAL,
					QUESTION
				} type;
				
				Window* parent; //if this is NULL it is considered to be free, and will behave as a normal window.
				bool critical; //if this is true, this window will be always on top and no other windows can be touched. only one at a time.
				
				DialogButtons button;
			};
			
			struct TooltipParameters
			{
				Window* win;
				float x;
				float y;
				float w;
				float h;
				
				string text;
			};
		
			bool add(Window* win, WindowParamaters params);
			void createDialog(DialogParameters paramDiag, universalCallback callback);
			void createTooltip(TooltipParameters paramTooltip);
			void destroy(Window* win);
			void manage();
			void translate(int x, int y, float& xout, float& yout);
	};
	
	class Component;
	
	class Container
	{
		protected:
			vector<Component*> components;
		public:
			void add(Component* component, LayoutParameter* param);
			void remove(Component* component);
			void clear();
	};
	
	class Component : public Event, public Container
	{
		protected:
			float x;
			float y;
			float w;
			float h;
		public:
			Component();
			Component(float w, float h);
			Component(float x, float y, float w, float h);
			
			virtual void setParameter(void* param) {};
			void setLayoutManager(Layout* layout);
			
			virtual void event(Core::MouseEvent evt, bool& handled);
			virtual void event(Core::KeyboardEvent evt);
			virtual void event(Core::WindowEvent evt);
			
			virtual void paint();
		friend class Layer, Window;
	};
	
	class Window : public Metrics, public Panel
	{
		public:
			struct WindowParameters
			{
				enum {
					BOTTOM,
					USERSPACE,
					ALWAYSONTOP
				} z;
				
				enum {
					DEFAULT,
					CENTERPARENT,
					CENTERSCREEN
				} location;
				
				Window* parent;
			};
		protected:
			WindowParameters parameters;
			
		public:
			Window(float x, float y, float w, float h, WindowParameter param);
			Window(float w, float h, WindowParameter param);
			
			void setSize(float w, float h);
			void setPosition(float w, float h);
			
		
		friend class Layer;
	};

}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#define __COMPOSITOR_H_END__  // signal that the main section of foo.h has been fully included
#endif