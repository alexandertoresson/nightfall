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
#include "core.h"

namespace Core
{
	KeyAttachment*			keymaps[SDLK_LAST];
	std::list<Listener*>	mouseListener;
	std::list<Listener*>	keyListener;
	std::list<Listener*>	paintListener;
	std::list<Listener*>	preFrameListener;
	
	bool keyState[SDLK_LAST];
	
	bool go = true;
	int mouseX = 0;
	int mouseY = 0;
	
	void processEvents()
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					bool down = false;
					if(event.key.type == SDL_KEYDOWN)
						down = true;
					
					//Create the KeyEvent structure
					KeyboardEvent keyEvent;
					
					keyEvent.type = event.key.type;            //Keyup or Keydown
					keyEvent.code = event.key.keysym.unicode;  //Translated unicode char
					keyEvent.key  = event.key.keysym.sym;      //SDLKey
					keyEvent.mod  = event.key.keysym.mod;      //SDLMod i.e. CTRL + ALT + ...
					
					bool caught = false;  //signal that globallistener has caught the event and no further listner shall have it.
					
					if(keyListener.size())
					{
						for(std::list<Listener*>::iterator iter = keyListener.begin(); iter != keyListener.end(); iter++)
						{
							KeyListener* keyListen = (KeyListener*)(*iter);
							
							if(keyListen->call(keyEvent))
							{
								caught = true;
								break;
							}
						}
					}
					
					if(!caught)
					{
						//call induvidual attached key with possible mods.
						KeyAttachment* attachedKey = keymaps[event.key.keysym.sym];
						if(attachedKey)
						{
							while(attachedKey)
							{
								if(keyEvent.mod & attachedKey->modifier || attachedKey->modifier == keyEvent.mod) //Must be both because if modifer is KMOD_NONE then it will always be false.
								{
									attachedKey->fptr(keyEvent);
									break;
								}
								
								attachedKey = attachedKey->next;
							}
						}
					}
					break;
				}
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
				{
					//Synthesize the mouse event.
					MouseEvent mouseEvent;
					switch(event.type)
					{
						case SDL_MOUSEMOTION:
							mouseX = event.motion.x;
							mouseY = event.motion.y;
							mouseEvent.x = mouseX;
							mouseEvent.y = mouseY;
							mouseEvent.type = MouseEvent::MOUSEMOVE;
							break;
						case SDL_MOUSEBUTTONUP:
							mouseX = event.button.x;
							mouseY = event.button.y;
							mouseEvent.type = MouseEvent::MOUSEUP;
							mouseEvent.button = event.button.button;
							mouseEvent.x = mouseX;
							mouseEvent.y = mouseY;
							break;
						case SDL_MOUSEBUTTONDOWN:
							mouseX = event.button.x;
							mouseY = event.button.y;
							mouseEvent.type = MouseEvent::MOUSEDOWN;
							mouseEvent.button = event.button.button;
							mouseEvent.x = mouseX;
							mouseEvent.y = mouseY;
							break;
					}
					
					for(std::list<Listener*>::iterator iter = mouseListener.begin(); iter != mouseListener.end(); iter++)
					{
						MouseListener* mouseListen = (MouseListener*)(*iter);
						if(mouseListen->call(mouseEvent))
							break;
					}
					break;
				}
				case SDL_QUIT:
					go = false;
					return;
					
				default:
				{
					break;
				}
			}
		}
	}
	
	//Binds special keys i.e. CTRL+A to a listener.
	bool bindKey( SDLKey key, SDLMod mod, eventKeyboard listener)
	{
		if(keymaps[key])
		{
			KeyAttachment* current = keymaps[key];
			KeyAttachment* next    = current->next;
			
			//If the current mod and key exists, it's a duplicate and will be considerd an error, duplicated binds is not supported.
			if(current->modifier == mod)
				return false;
			
			//The goal is to append the binding to the last item in the linked-list.
			if(next)
			{
				while(next)
				{
					if(next->modifier == mod)
						return false;
					
					current = current->next;
					next = next->next;
				}
			}
			
			current->next = new KeyAttachment(listener, mod);
			return true;
		}
		else
		{
			keymaps[key] = new KeyAttachment(listener, mod);
			return true;
		}
	}
	
	//Unbinds a special key.
	void unbindKey(SDLKey key, SDLMod mod)
	{
		if(keymaps[key])
		{
			KeyAttachment* prev = keymaps[key];
			KeyAttachment* current = keymaps[key];
			
			while(current)
			{
				if(current->modifier == mod)
				{
					if(current->next == NULL) //Final or middle attachment or the only one
					{
						if(current == prev) //The first attachment
						{
							keymaps[key] = NULL;
							delete current;
						}
						else
						{
							prev->next = current->next;
							delete current;
						}
					}
					break;
				}
				
				prev = current;
				current = current->next;
			}
		}
	}
	
	std::list<Listener*>::iterator addStdListener( Listener* listener)
	{
		switch(listener->getType())
		{
			case MOUSE:
				mouseListener.push_back(listener);
				return mouseListener.end();
			case KEY:
				keyListener.push_back(listener);
				return keyListener.end();
			case PAINT:
				paintListener.push_back(listener);
				return paintListener.end();
			case PREFRAME:
				preFrameListener.push_back(listener);
				return preFrameListener.end();
			default:
				return std::list<Listener*>::iterator();
		}
	}
	
	void removeListener(const std::list<Listener*>::iterator ptr)
	{
		Listener* listener = (*ptr);
		switch(listener->getType())
		{
			case MOUSE:
				mouseListener.erase(ptr);
				break;
			case KEY:
				keyListener.erase(ptr);
				break;
			case PAINT:
				paintListener.erase(ptr);
				break;
			case PREFRAME:
				preFrameListener.erase(ptr);
				break;
			default:
				break;
		}
	}
	
	void preFrameAll(float time_diff)
	{
		for(std::list<Listener*>::iterator iter = preFrameListener.begin(); iter != preFrameListener.end(); iter++)
		{
			PreFrameListener* listener = (PreFrameListener*)(*iter);
			listener->call(time_diff);
		}
	}
	
	void paintAll(float time_diff)
	{
		for(std::list<Listener*>::iterator iter = paintListener.begin(); iter != paintListener.end(); iter++)
		{
			PaintListener* listener = (PaintListener*)(*iter);
			listener->call(time_diff);
		}
	}
	
	void mainLoop()
	{
		int last_time = SDL_GetTicks();
		int frames = 0;
		//float mean[10];  - will be used for better frame timings
		//int mean_ptr = 0;
		float time_diff = 0.0f;

		
		while(go)
		{
			//Process events
			processEvents();
			
			//run preframes
			preFrameAll(time_diff);
			
			//paint all
			paintAll(time_diff);
			
			frames++;
			if(SDL_GetTicks() - last_time >= 1000)
			{
				time_diff = (SDL_GetTicks() - last_time) / (1000.0f * frames);
			}
		}
	}
}
