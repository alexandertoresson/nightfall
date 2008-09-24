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
#ifndef __LOADER_H__
#define __LOADER_H__

#ifdef DEBUG_DEP
#warning "loader.h"
#endif

// Define with the mere reason to point out thread-accessed
// methods.
#define _THREAD_ACCESSED

#include <string>
#include <queue>
#include "sdlheader.h"

namespace Window
{
	struct FunctionRun
	{
		void (*fptr)(void);
		std::string description;
	};
	
	struct ThreadInfo
	{
		std::queue<FunctionRun*>* pFunctions;
		int* pRunThread;
		int* pStatus;
		int* pProgress;
		SDL_mutex* pMutex;
	};

	class DataLoader
	{
		private:
			SDL_Thread*                 mpThread;
			std::queue<FunctionRun*>    mvFunctions;
			SDL_mutex*                  mpMutex;
			
			int mRunThread;
			int mProgress;
			
		public:
			DataLoader(void) : mpThread(NULL), mRunThread(0), mProgress(0), mpMutex(SDL_CreateMutex()) {};
			~DataLoader(void);
			
			void AddMethodToQueue(FunctionRun*);
			void Start(void);
			
			void GetCurrentProgress(int& value);
			void GetMaximumProgress(int& value);
	};
	
	_THREAD_ACCESSED int _ThreadMethod(void*);
}

#ifdef DEBUG_DEP
#warning "loader.h-end"
#endif

#endif
