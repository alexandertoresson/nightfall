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
#ifndef __CORE_H_
#define __CORE_H_

#include <vector>
#include <list>
#include "sdlheader.h"

/**
 * Handles low-level communication with SDL, event creation, global key-attachments, state-switching (replaced by scene graph).
 */
namespace Core
{
	#define SCROLLUP 1
	#define SCROLLDOWN -1
	
	/**
	 * Symbolises a Mouse event
	 */
	struct MouseEvent
	{
		/**
		 * Mouse event types
		 */
		enum MouseEventType
		{
			MOUSEUP,     /**< Mousebutton up   */
			MOUSEDOWN,   /**< Mousebutton down */
			MOUSEMOVE,   /**< Mousemotion      */
			MOUSESCROLL, /**< Mouse scrolling  */
			MOUSENONE
		};
		
		SDL_Event* pEvent;
		
		MouseEventType type; /**< Defines the mouse event type */
		
		int button; /**< Button code for which button that were pressed */
		int state;  /**< Determines if scrolling if up(1) or down(-1)*/
		
		int x;      /**< Absolute x-axis mouse position */
		int y;      /**< Absolute y-axis mouse position */

		MouseEvent() : pEvent(NULL), type(MOUSENONE), button(0), state(0), x(0), y(0) {}
	};
	
	/**
	 * Symbolises a Keyboard event
	 */
	struct KeyboardEvent
	{
		enum KeyboardEventType
		{
			KEYUP = SDL_KEYUP,
			KEYDOWN = SDL_KEYDOWN
		};
		/**
		 *	Keyboard event types
		 */
		Uint8 type; /**< Keydown or keyup, corresponding SDL_KEYUP, SDL_KEYDOWN */
		int code;    /**< Charcode      */
		SDLKey key;  /**< Keycode       */
		SDLMod mod;  /**< Modifier hash */
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
			virtual EventType getType() { return UNKNOWN; }
			void shade() {}
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
			
			MouseListener(eventMouse fptr) : fptr(fptr)
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
	
	class PreFrameListener : public Listener
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
		eventKeyboard fptr;
		gc_ptr<KeyAttachment> next;
		
		KeyAttachment(eventKeyboard fptr, SDLMod modifier)
		{
			this->fptr = fptr;
			this->modifier = modifier;
			this->next = NULL;
		}

		void shade()
		{
			next.shade();
		}
	};
	
	/**
	 * The main loop of the program, handles state execution, termination and SDL event-loop, also paint scheduling.
	 */
	void MainLoop();
	
	//Listeners
	template <typename T>
	struct listeners
	{
		static std::list<T> ls;
	};

	template <typename T>
	std::list<T> listeners<T>::ls;

	template <typename T>
	typename std::list<gc_ptr<T> >::iterator AddListener(gc_ptr<T> listener)
	{
		return listeners<gc_ptr<T> >::ls.insert(listeners<gc_ptr<T> >::ls.end(), listener);
	}
	
	template <typename T>
	typename std::list<gc_ptr<T> >::iterator AddListener(T* listener)
	{
		return AddListener(gc_ptr<T>(listener));
	}
	
	template <typename T>
	void RemoveListener(T it)
	{
		listeners<typename T::value_type>::ls.erase(it);
	}

	typedef listeners<gc_ptr<KeyListener> > keyListeners;
	typedef listeners<gc_ptr<MouseListener> > mouseListeners;
	typedef listeners<gc_ptr<PreFrameListener> > preFrameListeners;
	typedef listeners<gc_ptr<PaintListener> > paintListeners;

	typedef std::list<gc_ptr<KeyListener> >::iterator KeyListenerHandle;
	typedef std::list<gc_ptr<MouseListener> >::iterator MouseListenerHandle;
	typedef std::list<gc_ptr<PreFrameListener> >::iterator PreFrameListenerHandle;
	typedef std::list<gc_ptr<PaintListener> >::iterator PaintListenerHandle;
	
	/**
	 * Bind specific keys
	 */
	bool BindKey( SDLKey key, SDLMod mod, eventKeyboard listener);
	void UnbindKey(SDLKey key, SDLMod mod);
}
#endif
