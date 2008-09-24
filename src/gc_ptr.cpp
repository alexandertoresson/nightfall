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

#include "gc_ptr.h"

void gc_marker_base::insert()
{
	// This is so gc_ptrs can be constructed before initgc() has been run.
	// Before that, we do not have any threads running either, so no danger
	// in not locking the mutex.
	if (mutex)
		SDL_LockMutex(mutex);
	
	if (!marked[0])
	{
		marked[0] = new MarkerSet;
		marked[1] = new MarkerSet;
	}

	marked[mark]->insert(this);

	if (mutex)
		SDL_UnlockMutex(mutex);
}

void gc_marker_base::shade()
{
	if (tempMark == MARK_WHITE)
	{
		marked_temp[MARK_GRAY].insert(this);
		marked_temp[MARK_WHITE].erase(this);
		tempMark = MARK_GRAY;
	}
}

gc_marker_base::gc_marker_base(Mark mark, int refs) : mark(mark), tempMark(MARK_WHITE), refs(refs)
{
	insert();
}
		
void gc_marker_base::increfs()
{
	SDL_LockMutex(mutex);
	assert(refs >= 0);
	if (refs == 0)
	{
		marked[MARK_WHITE]->erase(this);
		marked[MARK_GRAY]->insert(this);

		mark = MARK_GRAY;
	}
	refs++;
	SDL_UnlockMutex(mutex);
}

void gc_marker_base::decrefs()
{
	SDL_LockMutex(mutex);
	assert(refs > 0);
	refs--;
	if (refs == 0 && mark == MARK_GRAY)
	{
		marked[MARK_GRAY]->erase(this);
		marked[MARK_WHITE]->insert(this);

		mark = MARK_WHITE;
	}
	SDL_UnlockMutex(mutex);
}

void gc_marker_base::sweep()
{
	if (!startedCollecting)
	{
		SDL_LockMutex(mutex);

		marked_temp[MARK_WHITE] = *marked[MARK_WHITE];
		marked_temp[MARK_GRAY] = *marked[MARK_GRAY];

		SDL_UnlockMutex(mutex);

		startedCollecting = true;

	}

	MarkerSet& white = marked_temp[MARK_WHITE];
	for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
	{
		(*it)->tempMark = MARK_WHITE;
	}

	MarkerSet& gray = marked_temp[MARK_GRAY];
	while (gray.size())
	{
		gc_marker_base* m2 = *gray.begin();
		gray.erase(gray.begin());
		m2->tempMark = MARK_GRAY;
		m2->blacken();
	}

	if (!gray.size())
	{
		MarkerSet& white = marked_temp[MARK_WHITE];

		int amount = 0;

		std::map<std::string, int> numPerSize;

		for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
		{
			std::string name = (*it)->name();
			amount += (*it)->size();
			if (numPerSize.find(name) == numPerSize.end())
			{
				numPerSize[name] = 1;
			}
			else
			{
				numPerSize[name]++;
			}
			(*it)->dispose();
			delete *it;
		}

		SDL_LockMutex(mutex);
		MarkerSet& rwhite = *marked[MARK_WHITE];

/*		std::cout << "Swept " << amount << " bytes in " << white.size() << " objects out of " << rwhite.size() << ". Gray: " << marked[MARK_GRAY].size() << std::endl;
		for (std::map<std::string, int>::iterator it = numPerSize.begin(); it != numPerSize.end(); it++)
		{
			std::cout << it->first << ": " << it->second << std::endl;
		}*/

		for (MarkerSet::iterator it = white.begin(); it != white.end(); it++)
		{
			rwhite.erase(rwhite.find(*it));
		}
		SDL_UnlockMutex(mutex);

		startedCollecting = false;
	}
}

void gc_marker_base::initgc()
{
	mutex = SDL_CreateMutex();
}

gc_marker_base::MarkerSet *(gc_marker_base::marked[2]) = {NULL, NULL};
gc_marker_base::MarkerSet gc_marker_base::marked_temp[2];

SDL_mutex* gc_marker_base::mutex = NULL;

gc_marker_base::CollectStep gc_marker_base::collectStep = gc_marker_base::COLLECTSTEP_NOTSTARTED;
bool gc_marker_base::startedCollecting = false;

