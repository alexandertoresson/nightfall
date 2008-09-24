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
#include "loader.h"

namespace Window
{
	DataLoader::~DataLoader(void)
	{
		while (mvFunctions.size() > 0)
		{
			FunctionRun* pFunc = mvFunctions.front();
		
			delete pFunc;
			mvFunctions.pop();
		}
		SDL_DestroyMutex(mpMutex);
	}
	
	void DataLoader::AddMethodToQueue(FunctionRun* pFunc)
	{
		if (pFunc == NULL)
			return;
		
		SDL_mutexP(mpMutex);
		mvFunctions.push(pFunc);
		SDL_mutexV(mpMutex);
	}
	
	void DataLoader::Start(void)
	{
		SDL_mutexP(mpMutex);
		if (mRunThread == 1)
		{
			mRunThread = 0;
			SDL_WaitThread(mpThread, NULL);
			
			mpThread = NULL;
		}
		SDL_mutexV(mpMutex);
	
		if (mvFunctions.size() == 0)
			return;
		
		ThreadInfo* pInfo = new ThreadInfo;
		pInfo->pFunctions = &mvFunctions;
		pInfo->pRunThread = &mRunThread;
		pInfo->pProgress  = &mProgress;
		pInfo->pMutex     = mpMutex;
		
		pInfo->pStatus = 0;
		
		mpThread = SDL_CreateThread(Window::_ThreadMethod, pInfo);
	}
	
	void DataLoader::GetCurrentProgress(int& value)
	{
		SDL_mutexP(mpMutex);
		value = mProgress;
		SDL_mutexV(mpMutex);
	}
	
	void DataLoader::GetMaximumProgress(int& value)
	{
		SDL_mutexP(mpMutex);
		value = mvFunctions.size();
		SDL_mutexV(mpMutex);
	}
	
	_THREAD_ACCESSED int _ThreadMethod(void* arg)
	{
		ThreadInfo* pInfo = static_cast<Window::ThreadInfo*>(arg);
		
		while (pInfo->pFunctions->size() > 0 &&
		       *pInfo->pRunThread == 1)
		{
			SDL_mutexP(pInfo->pMutex);
			
			FunctionRun* pFunc = pInfo->pFunctions->front();
			pFunc->fptr();
			
			// RITA
			// DrawStringSortOfMethod(pFunc->description);
			
			delete pFunc;
			pInfo->pFunctions->pop();
			
			pInfo->pProgress++;
			
			SDL_mutexV(pInfo->pMutex);
		}
		
		delete pInfo;
		
		return 0;
	}
}
