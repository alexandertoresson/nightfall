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
#include "sdlheader.h"
#include "guitest.h"
#include "utilities.h"
#include "core.h"
#include "window.h"
#include "compositor.h"

namespace GUI
{
	namespace Testing
	{
		Metrics* met = NULL;
		Component* testComponent = NULL;
	
		void paint(float time_diff)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			/*
			glPushMatrix();
			glColor3f(1.0f, 1.0f, 1.0f);
			
			float w = 10.0f;
			float h = 10.0f;
			
			met->scaleFactorCM(w,h);
			met->alignToPixel(w,h);
			
			glScalef(w, h, 0.0f);
			
			glBegin(GL_QUADS);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(1.0f, 0.0f);
			glVertex2f(1.0f, 1.0f);
			glVertex2f(0.0f, 1.0f);
			glEnd();
			
			glPopMatrix();
			*/
			testComponent->paintComponent();
			
			glFlush();
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
			return false;
		}
		
		bool handlespecificexit(Core::KeyboardEvent e)
		{
			Core::go = false;
			return true;
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
			Core::bindKey(SDLK_q, KMOD_NONE, &handlespecificexit);
			Core::bindKey(SDLK_ESCAPE, KMOD_NONE, &handlespecificexit);
			
			Utilities::SwitchTo2DViewport(1.0f,1.0f);
			met = new GUI::Metrics(1440, 900, 800, 600, true, 15.4f, false);
			met->scale();
			
			float w = 10.0f;
			float h = 2.0f;
			met->scaleFactorCM(w, h);
			
			testComponent = new Component(0.0f, 0.0f, w, h);
			
			Core::mainLoop();
		}
	}
}
