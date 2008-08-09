/*
 *  guitest.cpp
 *  Nightfall
 *
 *  Created by Marcus Klang on 2008-07-05.
 *  Copyright 2008 Lunds Tekniska Hˆgskola Campus. All rights reserved.
 *
 */

#include "sdlheader.h"
#include "guitest.h"
//#include "compositor.h"
#include "core.h"

namespace GUI
{
	namespace Testing
	{
		void paint(float time_diff)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_QUADS);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(1.0f, 0.0f);
			glVertex2f(1.0f, 1.0f);
			glVertex2f(0.0f, 1.0f);
			glEnd();
			SDL_GL_SwapBuffers();
		}
		
		bool mousehandling(Core::MouseEvent e)
		{
			switch(e.type)
			{
				case Core::MouseEvent::MOUSEUP:
					std::cout << "[MOUSE UP:" << e.button << "] ";
					break;
				case Core::MouseEvent::MOUSEDOWN:
					std::cout << "[MOUSE DOWN:" << e.button << "] ";
					break;
				case Core::MouseEvent::MOUSEMOVE:
					std::cout << "[MOUSE MOVE] ";
					break;
				default:
					break;
					
			}
			
			std::cout << "Mouse X: " << e.x << " Y:" << e.y << std::endl;
			
			return false;
		}
		
		bool handlespecific(Core::KeyboardEvent e)
		{
			switch(e.type)
			{
				case Core::KeyboardEvent::KEYUP:
					std::cout << "[SPECIAL KEYUP: KeyID:" << e.key << ", Unicode:" << e.code << ", Modifiers:" << e.mod << "] " << std::endl;
					break;
				case Core::KeyboardEvent::KEYDOWN:
					std::cout << "[SPECIAL KEYDOWN: KeyID:" << e.key << ", Unicode:" << e.code << ", Modifiers:" << e.mod << "] " << std::endl;
					break;
			}
		}
		
		bool keyhandling(Core::KeyboardEvent e)
		{
			switch(e.type)
			{
				case Core::KeyboardEvent::KEYUP:
					std::cout << "[KEYUP: KeyID:" << e.key << ", Unicode:" << e.code << ", Modifiers:" << e.mod << "] " << std::endl;
					break;
				case Core::KeyboardEvent::KEYDOWN:
					std::cout << "[KEYDOWN: KeyID:" << e.key << ", Unicode:" << e.code << ", Modifiers:" << e.mod << "] " << std::endl;
					break;
			}
			
			return false;
		}
		
		void start()
		{
			Core::addListener(new Core::PaintListener(&paint));
			Core::addListener(new Core::MouseListener(&mousehandling));
			Core::addListener(new Core::KeyListener(&keyhandling));
			Core::bindKey(SDLK_a, (SDLMod)KMOD_CTRL, &handlespecific);
			
			
			Core::mainLoop();
		}
	}
}
