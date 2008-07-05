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

#include <string>
#include <sdlheader.h>
#include <list>
#include <vector>

namespace GUI
{
	
	/**
	  * Handles low-level communication with SDL, event creation, global key-attachments, state-switching (replaced by scene graph).
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
			enum mouseEventType
			{
				MOUSEUP,    /**< Mousebutton up   */
				MOUSEDOWN,  /**< Mousebutton down */
				MOUSEMOVE,  /**< Mousemotion      */
				MOUSESCROLL /**< Mouse scrolling  */
			};
			
			SDL_Event* pEvent;
			
			mouseEventType type; /**< Defines the mouse event type */
			
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
			Uint8 type; /**< Keydown or keyup, corresponding SDL_KEYUP, SDL_KEYDOWN */
			int code;    /**< Charcode      */
			SDLKey key;  /**< Keycode       */
			SDLMod mod;  /**< Modifier hash */
		};
		
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
		
		/* Function pointer typedefs */
		typedef bool(*eventKeyboard)(KeyboardEvent e);
		typedef bool(*eventMouse)(MouseEvent e);
		typedef void(*eventPaint)(float time_diff);
		typedef void(*eventPreFrame)(float time_diff);
		
		enum EventType
		{
			MOUSE,
			KEY,
			PAINT,
			PREFRAME,
			UNKNOWN
		};
		
		/**
		 *	Handles the standard callback convention
		 */
		class Listener
		{
			public:
				void* tag;  /**< standard parameter */
			
				Listener()
				{
					this->tag = NULL;
				}
				
			virtual EventType getType() { return UNKNOWN; }
		};
		
		class KeyListener : public Listener
		{
			public:
				eventKeyboard fptr;
			
				KeyListener(eventKeyboard fptr)
				{
					this->fptr = fptr;
				}
				
				bool call(KeyboardEvent e)
				{
					return fptr(e);
				}
				
				virtual EventType getType() { return KEY; }
		};
		
		class MouseListener : public Listener
		{
			public:
				eventMouse fptr;
				
				MouseListener(eventMouse fptr)
				{
					this->fptr = fptr;
				}
				
				bool call(MouseEvent e)
				{
					return fptr(e);
				}
				
				virtual EventType getType() { return MOUSE; }
		};
		
		class PaintListener : public Listener
		{
			public:
				eventPaint fptr;
				
				PaintListener(eventPaint fptr)
				{
					this->fptr = fptr;
				}
				
				void call(float time_diff)
				{
					fptr(time_diff);
				}
				
				virtual EventType getType() { return PAINT; }
		};
		
		class PreFrameListener : protected Listener
		{
			public:
				eventPreFrame fptr;
				
				PreFrameListener(eventPreFrame fptr)
				{
					this->fptr = fptr;
				}
				
				void call(float time_diff)
				{
					fptr(time_diff);
				}
						
				virtual EventType getType() { return PREFRAME; }
		};
		
		
		struct KeyAttachment
		{
			SDLMod modifier;
			KeyListener listener;
			KeyAttachment* next;
		};
		
		extern KeyAttachment*		keymaps[SDLK_LAST];
		extern std::list<Listener*>	mouseListener;
		extern std::list<Listener*>	keyListener;
		extern std::list<Listener*>	paintListener;
		extern std::list<Listener*>	preFrameListener;
		
		extern bool keyState[SDLK_LAST];
		extern int mouseX;
		extern int mouseY;
		
		/**
		 * The main loop of the program, handles state execution, termination and SDL event-loop, also paint scheduling.
		 */
		void mainLoop();
		
		std::list<Listener*>::iterator addStdListener(Listener* listener);
		
		template<class T>
		std::list<Listener*>::iterator addListener( T* listener)
		{
			return addStdListener(dynamic_cast<Listener*> (listener));
		}
		
		void removeListener(std::list<Listener*>::iterator ptr);
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
	 * Handles scaling calculations and proponanlity corrections. Handles all types of aspects and monitor resolution.
	 * Equation: h_ref / sqrt( t * t / (a * a + 1) ) ) * 96, h_ref is the inch referens in height
	 */
	class Metrics
	{
		private:
			float dpi;     /**< dpi factor */
			float monitor; /**< Monitor inch size */
			float monitoraspect;
		protected:
			float dotsperwidth;  /**< dots per x-axis */
			float dotsperheight; /**< dots per y-axis */
			float percentscaling; /**< temporary scaling */
			
			/**
			 * Coordinate-system calculation.
			 * @param monitorsize	Holds the monitor inch size
			 * @param monitoraspect Holds the monitor aspect, this to correct errors imposed by non-native incorrect aspect scaling, viewing 800x600 in 1280x768 screen.
			 */
			void calculateCoordinateSystem(float monitorsize, float monitoraspect);
			
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
			void calculateCoordinateSystem(float monitorsize, int monitor_px_width, int monitor_px_height);
			
			/**
			 * Coordinate-system calculation when application is in a window with specific resolution.
			 * @param dpi				Specific resolution
			 * @param monitorsize		monitor inch size
			 * @param monitor_px_width	monitor native pixel x-axis resolution
			 * @param monitor_px_height monitor native pixel y-axis resolution
			 */
			void calculateCoordinateSystem(float dpi, float monitorsize, int monitor_px_width, int monitor_px_height);
			
			/**
			 *	from the given
			 */
			void scaleCoordinateSystem();
			void revertCoordinateSystem();
		public:
			float getDPI();
			float setDPI(float dpi);
		
			//Problems! Coordinate can be asymmetrical!
			float translatePointToPixel(float pt);
			float translatePixelToPoint(float px);
			
			void alignToPixel(float& x, float& y, float& w, float& h);
			
			void scale(float percent);
			
			void setMetrics(Metrics met);
	};
	
	/*
		Fundamental types of GUI objects:
			Window, Dialog and Tooltips
			
		The windowsystem has three z-layers
		Bottom (Gaming parts)
		Normal (InGame chat dialogs)
	*/
	
	class Component;

	namespace Utilities
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
	
	class Component : public Event, public Metrics
	{
	protected:
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
		virtual void event(Core::WindowEvent evt);
		
		virtual void paint();
		friend class Workspace;
	};
	
	/* Window-Management */	
	class Workspace : Metrics, Event
	{
		private:
			struct Windows {
				Windows* prev;
				Windows* next;
				Component object;
			};
			
			Windows** win;
		protected:
			void paintWindows(int layer);
			void paintTooltips();
			void paintDialogs();
		public:
			void paint();
			
			void add(Component elem);
			void remove(Component elem);
			void positionate(Component elem, int z);
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
	
	class Window : public Component
	{
		public:
			typedef enum {
				BOTTOM,
				USERSPACE,
				ALWAYSONTOP
			} LayerIndex;
		
			struct WindowParameter
			{
				LayerIndex layer;
				
				enum {
					DEFAULT,
					CENTERPARENT,
					CENTERSCREEN
				} location;
				
				Window* parent;
				
			};
			
			struct WindowChild
			{
				Window* current;
				WindowChild* prev;
				WindowChild* next;
			};
		protected:
			WindowParameter parameters;
			WindowChild* children;
			
		public:
			Window(float x, float y, float w, float h, WindowParameter param);
			Window(float w, float h, WindowParameter param);
			Window(WindowParameter param);
			
			void setSize(float w, float h);
			void setPosition(float x, float y);
		
		friend class Workspace;
	};

}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#endif
