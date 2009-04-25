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
	struct KeyAttachment
	{
		SDLMod modifier;
		EventKeyboard fptr;
		
		KeyAttachment(EventKeyboard fptr, SDLMod modifier) : modifier(modifier), fptr(fptr)
		{
			this->fptr = fptr;
			this->modifier = modifier;
		}
	};
	
	std::list<KeyAttachment> keymaps[SDLK_LAST];
	
	bool keyState[SDLK_LAST];
	
	bool go = true;
	int mouseX = 0;
	int mouseY = 0;
	
	void ProcessEvents()
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					//Create the KeyEvent structure
					KeyboardEvent keyEvent;
					
					keyEvent.type = static_cast<KeyboardEvent::KeyboardEventType>(event.key.type); //Keyup or Keydown
					keyEvent.code = event.key.keysym.unicode;  //Translated unicode char
					keyEvent.key  = event.key.keysym.sym;      //SDLKey
					keyEvent.mod  = event.key.keysym.mod;      //SDLMod i.e. CTRL + ALT + ...
					
					bool caught = false;  //signal that a listener has caught the signal
					
					//call individual attached keys with possible mods.
					std::list<KeyAttachment>& attachedKeys = keymaps[event.key.keysym.sym];
					for (std::list<KeyAttachment>::iterator it = attachedKeys.begin(); it != attachedKeys.end(); ++it)
					{
						//Must have both tests because if modifier is KMOD_NONE then it will always be false.
						if(keyEvent.mod & it->modifier || keyEvent.mod == it->modifier)
						{
							it->fptr(keyEvent);
							caught = true;
							break;
						}
					}

					if(!caught)
					{
						if(keyListeners::ls.size())
						{
							for(KeyListenerHandle iter = keyListeners::ls.begin(); iter != keyListeners::ls.end(); iter++)
							{
								if((**iter)(keyEvent))
								{
									caught = true;
									break;
								}
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
					
					for(MouseListenerHandle iter = mouseListeners::ls.begin(); iter != mouseListeners::ls.end(); iter++)
					{
						if((**iter)(mouseEvent))
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
	bool BindKey( SDLKey key, SDLMod mod, EventKeyboard listener)
	{
		std::list<KeyAttachment>& attachedKeys = keymaps[key];
		for (std::list<KeyAttachment>::iterator it = attachedKeys.begin(); it != attachedKeys.end(); ++it)
		{
			if(it->modifier == mod)
			{
				return false;
			}
		}

		keymaps[key].push_back(KeyAttachment(listener, mod));
		return true;
	}
	
	//Unbinds a special key.
	void UnbindKey(SDLKey key, SDLMod mod)
	{
		std::list<KeyAttachment>& attachedKeys = keymaps[key];
		for (std::list<KeyAttachment>::iterator it = attachedKeys.begin(); it != attachedKeys.end(); ++it)
		{
			if(it->modifier == mod)
			{
				attachedKeys.erase(it);
				return;
			}
		}
	}
	
	void PreFrameAll(float time_diff)
	{
		for(PreFrameListenerHandle iter = preFrameListeners::ls.begin(); iter != preFrameListeners::ls.end(); iter++)
		{
			(**iter)(time_diff);
		}
	}
	
	void PaintAll(float time_diff)
	{
		for(PaintListenerHandle iter = paintListeners::ls.begin(); iter != paintListeners::ls.end(); iter++)
		{
			(**iter)(time_diff);
		}
	}
	
	void MainLoop()
	{
		int last_time = SDL_GetTicks();
		int frames = 0;
		//float mean[10];  - will be used for better frame timings
		//int mean_ptr = 0;
		float time_diff = 0.0f;

		
		while(go)
		{
			//Process events
			ProcessEvents();
			
			//run preframes
			PreFrameAll(time_diff);
			
			//paint all
			PaintAll(time_diff);
			
			frames++;
			if(SDL_GetTicks() - last_time >= 1000)
			{
				time_diff = (SDL_GetTicks() - last_time) / (1000.0f * frames);
			}
		}
	}
}
