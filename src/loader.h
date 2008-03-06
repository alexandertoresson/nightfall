/*
 *  loader.h
 *  Projektarbete
 *
 *  Created by Leonard Wickmark on 2006-10-02.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
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
