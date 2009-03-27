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
#ifndef WINDOW_H
#define WINDOW_H

#ifdef DEBUG_DEP
#warning "window.h"
#endif

#include "window-pre.h"

#include "sdlheader.h"

namespace Window
{
	// Jag är inte säker på att vi behöver extern
	// i namespace. Åja, ren gissning.
	// Om vi stöter på problem räcker det ju med
	// att helt enkelt ta bort kommentarerna.
	//
	// Notera dock: initialized måste initieras.
	//              blir "unknown symbol exception"
	//              annars när man kallar Init.
	extern SDL_Surface* pWindow;
	
	// Initierar SDL
	int   Init();
	
	// Event called when closing the window
	void  OnClose(void (*fptr)(void));
	
	// Stäng ned programmet - rensa!
	void  Exit(void);	
	
	// Öppna ett fönster genom att läsa innehållet ur en konfigurationsfil. 
	// Kallar InitStatic efter värdena har lästs.
	int   OpenDynamic();
	
	// Öppnar ett fönster via statiska värden.
	int   OpenStatic(int width, int height, int bpp, bool fullscreen);
	
	// Sätter titeln på fönstret vilket namnger därmed programmet
	void  SetTitle(const char* title);

	// Sparar screenshot i ./screenshots
	void  MakeScreenshot(void);
}

#ifdef DEBUG_DEP
#warning "window.h-end"
#endif

#endif
