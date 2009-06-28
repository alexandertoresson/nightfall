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

#include "core.h"
#include "window.h"
#include "themeengine.h"

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
		enum DialogType
		{
			INFORMATION,
			EXCLAMATION,
			CRITICAL,
			QUESTION
		};  /**< Dialog types */
		
		enum DialogStandardButton
		{
			B_OK,
			B_OKCANCEL,
			B_YESNO,
			B_USERDEFINED  /**< Let's user decide labels */
		}; 
	
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
	
		int CreateTooltip(int id, std::string text, float x, float y);
		int CreateTooltipEx(int id, gc_ptr<Component> comp, float x, float y);
		void RemoveTooltip(int id);
		void MoveTooltip(int id, float x, float y);
		
		void CreateDialog(DialogParameters paramDiag, universalCallback callback);
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
			virtual bool Handle(Core::MouseEvent evt) { return false; }
			
			/**
			 * Keyboard event
			 * @param evt KeyboardEvent structure that contains the keyboard event information
			 * @see Core::KeyboardEvent
			 */
			virtual bool Handle(Core::KeyboardEvent evt) { return false; }
			
			/**
			 * Window event
			 * @param evt WindowEvent structure that contains information about the window event.
			 * @see Core::WindowEvent
			 */
			virtual bool Handle(WindowEvent evt) { return false; }
			
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
			
			typedef std::list<gc_ptr<Component> >::iterator ComponentHandle;

			ComponentHandle handle;

			gc_ptr<Container> parent;

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
	
			bool visible;

			bool needsRelayout;
			
			virtual void Paint();
			
			virtual void Layout();

		public:

			Component(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f);
			virtual ~Component();
			
			virtual bool IsInsideArea(float x, float y);
			
			void SetVisible(bool state);
			
			void SetMinSize(float w, float h);
			void SetMaxSize(float w, float h);
			void SetSize(float w, float h);
			void SetAspectRatio(float r);

			void SetPosition(float x, float y);

			void SetVerticalAdjustment(VerticalAdjustment vAdjustment);
			void SetHorizontalAdjustment(HorizontalAdjustment vAdjustment);
			
			void SetAnchor(Anchor anchor, bool enabled);
			
			void SetMargin(float top, float right, float bottom, float left);

			virtual void PaintComponent();
			
			void ScheduleRelayout();
			
			friend class Workspace;
			friend class Container;

			friend class ThemeEngine::Drawer<Component>;
			
			virtual void shade() {}

	};
	
	class Container : public Component, public gc_ptr_from_this<Container>
	{
		protected:
			std::list<gc_ptr<Component> > components;

			BorderSize padding;

			bool subAnchors[ANCHOR_NUM];
	
			void ApplyAnchors();
			void ApplyAdjustment();
			
			virtual void Layout();

			virtual void LayoutAll();
			virtual void PostLayout();
			
			void PaintAll();

			virtual void PaintComponent();

		public:
			ThemeEngine::Borders innerBorders;
			ThemeEngine::Borders outerBorders;
			ThemeEngine::ScrollBar vertScrollBar;
			ThemeEngine::ScrollBar horizScrollBar;

			Container(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f);

			void Insert(gc_ptr<Component> component, int position);
			void Add(gc_ptr<Component> component);
			virtual void Remove(gc_ptr<Component> component);
			virtual void Clear();
			bool IsEmpty();

			void SetPadding(float top, float right, float bottom, float left);
			void SetSubAnchor(Anchor anchor, bool set);

			virtual void Paint();
			
			virtual void Shade()
			{
				Component::shade();
				gc_shade_container(components);
			}
	};

	class Workspace;

	class Frame : public Container
	{
		private:
			
			typedef std::list<gc_ptr<Frame> >::iterator FrameHandle;

			FrameHandle handle;
			
			gc_ptr<Workspace> parent;

		protected:
			enum StartLocation
			{
				LOCATION_DEFAULT,
				LOCATION_CENTERSCREEN,
				LOCATION_USERDEFINED
			};
			
			enum LayerIndex
			{
				LAYER_BOTTOM = 0,
				LAYER_STANDARD = 1,
				LAYER_POPUP = 2,
				LAYER_END = 3
			};
		
			LayerIndex layer;
			StartLocation location;

			void Paint();
		public:
			ThemeEngine::FrameBorders borders;

			Frame(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f, StartLocation location = LOCATION_DEFAULT, LayerIndex layer = LAYER_STANDARD);
			
			friend class Workspace;
	};
	
	/* Window-Management */	
	class Workspace : public Event, public gc_ptr_from_this<Workspace>
	{
		private:
			std::vector<std::list<gc_ptr<Frame> > > frames;
			
		public:
			Workspace();
			
			void Paint();
			
			void Add(gc_ptr<Frame> elem);
			void Remove(gc_ptr<Frame> elem);
			void Position(gc_ptr<Frame> elem, Frame::LayerIndex z);

			void shade();
			
		public:
			static Metrics metrics;
			static gc_ptr<ThemeEngine::Theme> theme;
		
			static void InitializeWorkspaces(int native_w, int native_h, float monitorsize, bool stretched);
	};

}

#ifdef DEBUG_DEP
	#warning "compositor.h-end"
#endif

#endif
