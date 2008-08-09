/*
 *  core.h
 *  Nightfall
 *
 *  Created by Marcus Klang on 2008-07-05.
 *  Copyright 2008 Lunds Tekniska Hˆgskola Campus. All rights reserved.
 *
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
			MOUSEUP,    /**< Mousebutton up   */
			MOUSEDOWN,  /**< Mousebutton down */
			MOUSEMOVE,  /**< Mousemotion      */
			MOUSESCROLL /**< Mouse scrolling  */
		};
		
		SDL_Event* pEvent;
		
		MouseEventType type; /**< Defines the mouse event type */
		
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
		eventKeyboard fptr;
		KeyAttachment* next;
		
		KeyAttachment(eventKeyboard fptr, SDLMod modifier)
		{
			this->fptr = fptr;
			this->modifier = modifier;
			this->next = NULL;
		}
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
	
	//Listeners
	std::list<Listener*>::iterator addStdListener(Listener* listener);
	
	template<class T>
	std::list<Listener*>::iterator addListener( T* listener)
	{
		return addStdListener(dynamic_cast<Listener*> (listener));
	}
	
	void removeListener(std::list<Listener*>::iterator ptr);
	
	/**
	 * Bind specific keys
	 */
	bool bindKey( SDLKey key, SDLMod mod, eventKeyboard listener);
	void unbindKey(SDLKey key, SDLMod mod);
}
#endif
