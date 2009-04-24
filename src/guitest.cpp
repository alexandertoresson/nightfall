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
		gc_ptr<Metrics> met;
		gc_ptr<Component> testComponent;
	
		void Paint(float time_diff)
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
			testComponent->PaintComponent();
			
			glFlush();
			SDL_GL_SwapBuffers();
		}
		
		bool MouseHandling(Core::MouseEvent e)
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
		
		bool HandleSpecific(Core::KeyboardEvent e)
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
		
		bool HandleSpecificExit(Core::KeyboardEvent e)
		{
//			Core::go = false;
			return true;
		}
		
		bool KeyHandling(Core::KeyboardEvent e)
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
		
		void Start()
		{
			Core::AddListener(new Core::PaintListener(&Paint));
			Core::AddListener(new Core::MouseListener(&MouseHandling));
			Core::AddListener(new Core::KeyListener(&KeyHandling));
			Core::BindKey(SDLK_a, (SDLMod)KMOD_CTRL, &HandleSpecific);
			Core::BindKey(SDLK_q, KMOD_NONE, &HandleSpecificExit);
			Core::BindKey(SDLK_ESCAPE, KMOD_NONE, &HandleSpecificExit);
			
			Utilities::SwitchTo2DViewport(1.0f,1.0f);
			met = new GUI::Metrics(1440, 900, 800, 600, true, 15.4f, false);
			met->Scale();
			
			float w = 10.0f;
			float h = 2.0f;
			met->ScaleFactorCM(w, h);
			
			testComponent = new Component(0.0f, 0.0f, w, h);
			
			Core::MainLoop();
		}
	}
}
