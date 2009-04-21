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
#ifndef COMPOSITOR_H
#define COMPOSITOR_H 

#ifdef DEBUG_DEP
	#warning "compositor.h"
#endif

#include "compositor-pre.h"
#include "themeengine-pre.h"

#include "core.h"
#include "window.h"

#include <string>
#include <sdlheader.h>
#include <list>
#include <vector>

namespace GUI
{	
	/*
		Fundamental types of GUI objects:
			Window, Dialog and Tooltips
			
		The windowsystem has three z-layers
		Bottom (Gaming parts)
		Normal (InGame chat dialogs)
		Top (Dialog)
	*/
	
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
						names.push_back(_("OK"));
						focused = 0;
						type = B_OK;
						break;
					case B_OKCANCEL:
						names.push_back(_("OK"));
						names.push_back(_("Cancel"));
						focused = 1;
						type = B_OKCANCEL;
						break;
					case B_YESNO:
						names.push_back(_("Yes"));
						names.push_back(_("No"));
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
			
			gc_ptr<Frame> parent; /**< if this is NULL it is considered to be free, and will behave as a normal window. */
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
			
			DialogParameters(gc_ptr<Frame> parent, DialogType diagType, std::string message, std::string title, DialogButtons button)
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
		int createTooltipEx(int id, gc_ptr<Component> comp, float x, float y);
		void removeTooltip(int id);
		void moveTooltip(int id, float x, float y);
		
		void createDialog(DialogParameters paramDiag, universalCallback callback);
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
			virtual void event(WindowEvent evt) {}
			
			/**
			 * Window event
			* @param evt WindowEvent structure that contains information about the window event.
			 * @see Core::WindowEvent
			 */
			virtual ~Event() {};

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
	typedef std::list<gc_ptr<Component> >::iterator componentHandle;
	
	class Component : public Event
	{
		public:
			enum VerticalAdjustment
			{
				V_ADJUSTMENT_LEFT,
				V_ADJUSTMENT_MIDDLE,
				V_ADJUSTMENT_RIGHT,
				V_ADJUSTMENT_NONE
			};

			enum HorizontalAdjustment
			{
				H_ADJUSTMENT_TOP,
				H_ADJUSTMENT_MIDDLE,
				H_ADJUSTMENT_BOTTOM,
				H_ADJUSTMENT_NONE
			};

			enum Anchor
			{
				ANCHOR_TOP=0,
				ANCHOR_RIGHT,
				ANCHOR_BOTTOM,
				ANCHOR_LEFT,
				ANCHOR_NUM
			};

		protected:
			
			gc_ptr<Container> parent;
			componentHandle containerHandle;

			struct Bounds
			{
				float x;
				float y;
				float w;
				float h;
				
				Bounds(float x, float y, float w, float h) : x(x), y(y), w(w), h(h)
				{
				}
			} dimensions;

			struct Size
			{
				float w;
				float h;
				
				Size(float w, float h) : w(w), h(h) {}
			} min, max;

			struct BorderSize
			{
				float top, right, bottom, left;
				BorderSize() : top(0.0f), right(0.0f), bottom(0.0f), left(0.0f)
				{
					
				}
			} margin;

			float aspectRatio;

			VerticalAdjustment vAdjustment;
			HorizontalAdjustment hAdjustment;

			bool anchors[ANCHOR_NUM];
	
			gc_ptr<Metrics> metrics;
			
			bool visible;

			bool needsRelayout;
			
			virtual void paint();
			
			virtual void layout();

		public:

			Component(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f);
			virtual ~Component();
			
			gc_ptr<Metrics> getMetrics();

			virtual bool isInsideArea(float x, float y);
			
			void setVisible(bool state);
			
			void setMinSize(float w, float h);
			void setMaxSize(float w, float h);
			void setSize(float w, float h);
			void setAspectRatio(float r);

			void setPosition(float x, float y);

			void setVerticalAdjustment(VerticalAdjustment vAdjustment);
			void setHorizontalAdjustment(HorizontalAdjustment vAdjustment);
			
			void setAnchor(Anchor anchor, bool enabled);
			
			void setMargin(float top, float right, float bottom, float left);

			virtual void event(Core::MouseEvent evt, bool& handled);
			virtual void event(Core::KeyboardEvent evt);
			virtual void event(WindowEvent evt);
			
			virtual void paintComponent();
			
			void scheduleRelayout();
			
			friend class Workspace;
			friend class Container;

			friend class ThemeEngine::Drawer<Component>;
			
			virtual void shade()
			{
				parent.shade();
				metrics.shade();
			}

	};
	
	class Container : public Component
	{
		protected:
			std::list<gc_ptr<Component> > components;

			BorderSize padding;

			bool subAnchors[ANCHOR_NUM];
	
			void applyAnchors();
			void applyAdjustment();
			
			virtual void layout();

			virtual void layoutAll();
			virtual void postLayout();
			
			void paintAll();

			virtual void paintComponent();

		public:
			ThemeEngine::Borders innerBorders;
			ThemeEngine::Borders outerBorders;

			Container();

			componentHandle insert(gc_ptr<Component> component, int position);
			componentHandle add(gc_ptr<Component> component);
			gc_ptr<Component> get(componentHandle handle);
			virtual void remove(componentHandle handle);
			virtual void clear();
			bool isEmpty();

			void setPadding(float top, float right, float bottom, float left);
			void setSubAnchor(Anchor anchor, bool set);

			virtual void paint();
			
			virtual void shade()
			{
				Component::shade();
				gc_shade_container(components);
			}
	};

	class Frame : public Container
	{
		private:
			virtual void paintBackground() {};
			virtual void paintGlass() {};
		protected:
			ThemeEngine::FrameBorders frameBorders;
			ThemeEngine::Text frameTitle;

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
				gc_ptr<Frame> parent;
				gc_ptr<Metrics> met;
			};
			
			StartLocation start;
			WindowParameter parameters;
			Bounds windowDimensions;
			
			void paint();
		public:
			Frame(float x, float y, float w, float h, WindowParameter param);
			Frame(float w, float h, WindowParameter param);
			Frame(WindowParameter param);
			~Frame();
			
			friend class Workspace;
	};
	
	typedef std::list<gc_ptr<Frame> >::iterator windowHandle;
	
	/* Window-Management */	
	class Workspace : Event
	{
		private:
			std::list<gc_ptr<Frame> > win;
			std::list<gc_ptr<Frame> >::iterator bottom; /* layer-pointers */
			std::list<gc_ptr<Frame> >::iterator standard;
			std::list<gc_ptr<Frame> >::iterator popup;
			
		protected:
			gc_ptr<Metrics> met;
		
			void paintWindows(Frame::LayerIndex layer);
			void paintTooltips();
			void paintDialogs();
			
		public:
			Workspace(int native_w, int native_h, float monitorsize, bool streched);
			
			void paint();
			
			windowHandle add(gc_ptr<Frame> elem);
			void remove(windowHandle elem);
			void remove(gc_ptr<Frame> elem); /* SLOW! use remove(windowHandle) */
			void positionate(windowHandle elem, Frame::LayerIndex z);
	};
}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#endif
