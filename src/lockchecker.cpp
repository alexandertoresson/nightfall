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

#define NO_LOCKCHECKER
#include "sdlheader.h"

void Delay(unsigned n)
{
	SDL_Delay(n);
}

void LockMutex(SDL_mutex* m)
{
	SDL_LockMutex(m);
}

void UnlockMutex(SDL_mutex* m)
{
	SDL_UnlockMutex(m);
}

void CondWait(SDL_cond* cond, SDL_mutex* mutex)
{
	SDL_CondWait(cond, mutex);
}

SDL_Thread *CreateThread(int (*fn)(void *), void *data)
{
	return SDL_CreateThread(fn, data);
}

#include "lockchecker.h"

#include <set>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <list>

struct LockItem
{
	std::string file;
	unsigned line;
	std::string name;
	SDL_mutex* m;
	Uint32 lastLockTime;
	Uint32 totalExecTime;
	Uint32 totalLockTime;
	Uint32 totalUnlockTime;
	unsigned lockTimes;
	unsigned unLockTimes;
	unsigned execTimes;
	LockItem *next;
	LockItem(SDL_mutex* m, std::string file, unsigned line, std::string name) : file(file), line(line), name(name), m(m), totalExecTime(0), totalLockTime(0), totalUnlockTime(0), lockTimes(0), unLockTimes(0), execTimes(0), next(NULL)
	{
		
	}
};

class LockHash
{
	private:
		LockItem** items;
		unsigned numItems;
		bool (*cmp)(LockItem*, LockItem*);
		unsigned long (*hash)(LockItem*);

	public:

		LockHash(unsigned numItems, bool (*cmp)(LockItem*, LockItem*), unsigned long (*hash)(LockItem*))
		{
			this->numItems = numItems;
			items = new LockItem*[numItems];
			memset(items, 0, sizeof(LockItem*) * numItems);
			this->cmp = cmp;
			this->hash = hash;
		}

		LockItem* getItem(SDL_mutex* m, std::string file, unsigned line, std::string name)
		{
			LockItem* lastItem = NULL;
			LockItem* newItem = new LockItem(m, file, line, name);
			unsigned long k = hash(newItem) % numItems;
			LockItem* curItem = items[k];

			while (curItem)
			{
				if (cmp(curItem, newItem))
				{
					delete newItem;
					return curItem;
				}
				lastItem = curItem;
				curItem = curItem->next;
			}

			if (lastItem)
			{
				lastItem->next = newItem;
			}
			else
			{
				items[k] = newItem;
			}
			return newItem;
		}

		static bool cmp_lockitems(LockItem* a, LockItem* b)
		{
			return a->totalLockTime + a->totalUnlockTime > b->totalLockTime + b->totalUnlockTime;
		}

		void Iterate(void (*callback)(LockItem*))
		{
			std::list<LockItem*> ls;
			for (unsigned i = 0; i < numItems; i++)
			{
				LockItem* curItem = items[i];
				while (curItem)
				{
					ls.push_back(curItem);
					curItem = curItem->next;
				}
			}
			ls.sort(cmp_lockitems);

			for (std::list<LockItem*>::iterator it = ls.begin(); it != ls.end(); ++it)
			{
				callback(*it);
			}
		}
};

struct LockedItems
{
	SDL_mutex* m;
	LockItem* all;
	LockItem* byM;
	LockItem* byPos;
	LockItem* byUniq;
	Uint32 locker;
	Uint32 lockTime;
	std::string file;
	unsigned line;
};

std::map<SDL_mutex*, LockedItems*> lockedItems;

unsigned long hash_all(LockItem* a)
{
	return (unsigned long) a->m + a->line;
}

bool cmp_all(LockItem* a, LockItem* b)
{
	return a->m == b->m && a->file == b->file && a->line == b->line;
}

unsigned long hash_m(LockItem* a)
{
	return (unsigned long) a->m;
}

bool cmp_m(LockItem* a, LockItem* b)
{
	return a->m == b->m;
}

unsigned long hash_pos(LockItem* a)
{
	return a->line;
}

bool cmp_pos(LockItem* a, LockItem* b)
{
	return a->file == b->file && a->line == b->line;
}

unsigned long hash_uniq(LockItem* a)
{
	return 0;
}

bool cmp_uniq(LockItem* a, LockItem* b)
{
	return (a->m == b->m) || (a->name == b->name) || (a->file == b->file && a->line == b->line);
}

std::vector<LockHash> lockHash;
std::vector<LockHash> lockHashByM;
std::vector<LockHash> lockHashByPos;
std::vector<LockHash> lockHashByUniq;

SDL_mutex *lockCheckMutex = NULL;

Uint32 startTime = 0;

unsigned numThreads = 1;

Uint32 spentLTime = 0, spentDTime = 0, spentCTime = 0;
Uint32 numLocks = 0;

struct ThreadInfo
{
	Uint32 lockTime;
	Uint32 delay;
	Uint32 condWait;
	std::string file;
	unsigned line;
	ThreadInfo()
	{
		lockTime = 0;
		delay = 0;
		condWait = 0;
	}
};

std::map<Uint32, unsigned> idToIndex;
std::vector<ThreadInfo> threads;

static void AddThread(const ThreadInfo& tI, Uint32 id)
{
	threads.push_back(tI);
	numThreads = threads.size();
	idToIndex[id] = numThreads-1;

	lockHash.push_back(LockHash(256, cmp_all, hash_all));
	lockHashByM.push_back(LockHash(256, cmp_m, hash_m));
	lockHashByPos.push_back(LockHash(256, cmp_pos, hash_pos));
	lockHashByUniq.push_back(LockHash(1, cmp_uniq, hash_uniq));
}

void CheckLock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	if (!m)
	{
		std::cout << "The mutex pointer received was NULL. SDL would let this continue, but rather do nothing as it has no mutex to lock. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	if (startTime == 0 || startTime > 100000)
	{
		startTime = SDL_GetTicks();
	}
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		AddThread(tI, locker);
	}

	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		UnlockMutex(lockCheckMutex);
		return;
	}
	unsigned index = it2->second;

	std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.find(m);

	if (it != lockedItems.end() && it->second->locker == locker)
	{
		LockedItems* lI = it->second;
		std::cout << "Error: Attempted to lock already locked mutex " << name << " from same thread at " << file << ":" << line << std::endl;
		std::cout << "Originally locked at " << lI->file << ":" << lI->line << std::endl;
		UnlockMutex(lockCheckMutex);
		return;
	}

	LockedItems* lI = new LockedItems;
	lI->all = lockHash[index].getItem(m, file, line, name);
	lI->byM = lockHashByM[index].getItem(m, file, line, name);
	lI->byPos = lockHashByPos[index].getItem(m, file, line, name);
	lI->byUniq = lockHashByUniq[index].getItem(m, file, line, name);
	lI->m = m;
	lI->locker = locker;
	lI->file = file;
	lI->line = line;
	UnlockMutex(lockCheckMutex);

	Uint32 sTime = SDL_GetTicks();

	LockMutex(m);

	Uint32 totTime = SDL_GetTicks() - sTime;

	LockMutex(lockCheckMutex);

	lI->lockTime = SDL_GetTicks();

	lI->all->totalLockTime += totTime;
	lI->all->lockTimes++;

	lI->byM->totalLockTime += totTime;
	lI->byM->lockTimes++;

	lI->byPos->totalLockTime += totTime;
	lI->byPos->lockTimes++;

	lI->byUniq->totalLockTime += totTime;
	lI->byUniq->lockTimes++;

	spentLTime += totTime;
	threads[index].lockTime += totTime;
	numLocks++;

	lockedItems[m] = lI;
	UnlockMutex(lockCheckMutex);
	
}

void RecordLock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	if (!m)
	{
		std::cout << "The mutex pointer received was NULL. SDL would let this continue, but rather do nothing as it has no mutex to lock. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	if (startTime == 0 || startTime > 100000)
	{
		startTime = SDL_GetTicks();
	}
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		AddThread(tI, locker);
	}

	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		return;
	}
	unsigned index = it2->second;

	std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.find(m);

	if (it != lockedItems.end() && it->second->locker == locker)
	{
		LockedItems* lI = it->second;
		std::cout << "Error: Attempted to lock already locked mutex " << name << " from same thread at " << file << ":" << line << std::endl;
		std::cout << "Originally locked at " << lI->file << ":" << lI->line << std::endl;
		return;
	}

	LockedItems* lI = new LockedItems;
	lI->all = lockHash[index].getItem(m, file, line, name);
	lI->byM = lockHashByM[index].getItem(m, file, line, name);
	lI->byPos = lockHashByPos[index].getItem(m, file, line, name);
	lI->byUniq = lockHashByUniq[index].getItem(m, file, line, name);
	lI->m = m;
	lI->locker = locker;
	lI->file = file;
	lI->line = line;

	Uint32 sTime = SDL_GetTicks();

	Uint32 totTime = SDL_GetTicks() - sTime;

	lI->lockTime = SDL_GetTicks();

	lI->all->totalLockTime += totTime;
	lI->all->lockTimes++;

	lI->byM->totalLockTime += totTime;
	lI->byM->lockTimes++;

	lI->byPos->totalLockTime += totTime;
	lI->byPos->lockTimes++;

	lI->byUniq->totalLockTime += totTime;
	lI->byUniq->lockTimes++;

	spentLTime += totTime;
	threads[index].lockTime += totTime;
	numLocks++;

	lockedItems[m] = lI;
}

void CheckUnlock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	LockedItems* lI = NULL;
	if (!m)
	{
		std::cout << "The mutex pointer received was NULL. SDL would let this continue, but rather do nothing as it has no mutex to lock. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);
	
	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		UnlockMutex(lockCheckMutex);
		return;
	}
	unsigned index = it2->second;

	std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.find(m);

	if (it != lockedItems.end())
	{
		lI = it->second;
	}
	if (!lI)
	{
		std::cout << "Error: Attempted to unlock not locked mutex " << name << " at " << file << ":" << line << std::endl;
		UnlockMutex(lockCheckMutex);
		return;
	}

	Uint32 eTime = SDL_GetTicks() - lI->lockTime;

	lI->all->totalExecTime += eTime;
	lI->all->execTimes++;

	lI->byM->totalExecTime += eTime;
	lI->byM->execTimes++;

	lI->byPos->totalExecTime += eTime;
	lI->byPos->execTimes++;

	lI->byUniq->totalExecTime += eTime;
	lI->byUniq->execTimes++;

	lockedItems.erase(m);
	Uint32 cTime = SDL_GetTicks();
	UnlockMutex(m);

	eTime = SDL_GetTicks() - cTime;
	
	lI->all->totalUnlockTime += eTime;
	lI->all->unLockTimes++;
	
	lI->byM->totalUnlockTime += eTime;
	lI->byM->unLockTimes++;
	
	lI->byPos->totalUnlockTime += eTime;
	lI->byPos->unLockTimes++;

	lI->byUniq->totalUnlockTime += eTime;
	lI->byUniq->unLockTimes++;

	spentLTime += eTime;
	threads[index].lockTime += eTime;

	delete lI;
	
	UnlockMutex(lockCheckMutex);
}

void RecordUnlock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	LockedItems* lI = NULL;
	if (!m)
	{
		std::cout << "The mutex pointer received was NULL. SDL would let this continue, but rather do nothing as it has no mutex to lock. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	
	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		return;
	}
	unsigned index = it2->second;

	std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.find(m);

	if (it != lockedItems.end())
	{
		lI = it->second;
	}
	if (!lI)
	{
		std::cout << "Error: Attempted to unlock not locked mutex " << name << " at " << file << ":" << line << std::endl;
		return;
	}

	Uint32 eTime = SDL_GetTicks() - lI->lockTime;

	lI->all->totalExecTime += eTime;
	lI->all->execTimes++;

	lI->byM->totalExecTime += eTime;
	lI->byM->execTimes++;

	lI->byPos->totalExecTime += eTime;
	lI->byPos->execTimes++;

	lI->byUniq->totalExecTime += eTime;
	lI->byUniq->execTimes++;

	lockedItems.erase(m);
	Uint32 cTime = SDL_GetTicks();

	eTime = SDL_GetTicks() - cTime;
	
	lI->all->totalUnlockTime += eTime;
	lI->all->unLockTimes++;
	
	lI->byM->totalUnlockTime += eTime;
	lI->byM->unLockTimes++;
	
	lI->byPos->totalUnlockTime += eTime;
	lI->byPos->unLockTimes++;

	lI->byUniq->totalUnlockTime += eTime;
	lI->byUniq->unLockTimes++;

	spentLTime += eTime;
	threads[index].lockTime += eTime;

	delete lI;
}

void RecordDelay(unsigned n)
{
	Uint32 locker = SDL_ThreadID();
	Uint32 sTime = SDL_GetTicks();
	Delay(n);
	Uint32 eTime = SDL_GetTicks() - sTime;
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		AddThread(tI, SDL_ThreadID());
	}
	
	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		UnlockMutex(lockCheckMutex);
		return;
	}
	unsigned index = it2->second;

	spentDTime += eTime;
	threads[index].delay += eTime;
	UnlockMutex(lockCheckMutex);
}

void RecordCondWait(SDL_cond* cond, SDL_mutex* mutex, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	if (!cond)
	{
		std::cout << "The condition pointer received was NULL. SDL would let this continue, but rather do nothing as it has no condition to wait for. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	if (!mutex)
	{
		std::cout << "The mutex pointer received was NULL. SDL would let this continue, but rather do nothing as it has no mutex to lock. I'm now gonna crash the program, to prevent you from shooting yourself in the foot." << std::endl;
		*(int*) 0 = 0;
	}
	if (startTime == 0 || startTime > 100000)
	{
		startTime = SDL_GetTicks();
	}
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);
	
	std::map<Uint32, unsigned>::iterator it2 = idToIndex.find(locker);
	if (it2 == idToIndex.end())
	{
		UnlockMutex(lockCheckMutex);
		return;
	}
	unsigned index = it2->second;

	RecordUnlock(mutex, name, file, line);
	UnlockMutex(lockCheckMutex);

	Uint32 sTime = SDL_GetTicks();
	CondWait(cond, mutex);
	Uint32 eTime = SDL_GetTicks() - sTime;

	LockMutex(lockCheckMutex);
	RecordLock(mutex, name, file, line);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		AddThread(tI, SDL_ThreadID());
	}
	spentCTime += eTime;
	threads[index].condWait += eTime;
	UnlockMutex(lockCheckMutex);
}

SDL_Thread* RecordCreateThread(int (*fn)(void *), void *data, std::string file, unsigned line)
{
	SDL_Thread *thread;
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);

	thread = CreateThread(fn, data);
	
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		AddThread(tI, SDL_ThreadID());
	}

	ThreadInfo tI;
	tI.file = file;
	tI.line = line;

	AddThread(tI, SDL_GetThreadID(thread));

	UnlockMutex(lockCheckMutex);
	return thread;
}

std::ofstream ofile;
Uint32 stopTime;

void OutputLockItem(LockItem* item)
{
	ofile << item->name << " "
	      << item->file << " "
	      << item->line << ": "
	      << (float) item->totalLockTime / item->lockTimes << " over " << item->lockTimes << ", "
	      << (float) item->totalExecTime / item->execTimes << " over " << item->execTimes << ", "
	      << (float) item->totalUnlockTime / item->unLockTimes << " over " << item->unLockTimes << ", "
	      << item->totalLockTime + item->totalUnlockTime << std::endl;
}

void WriteLockReport(std::string file)
{
	if (!lockCheckMutex)
	{
		lockCheckMutex = SDL_CreateMutex();
	}
	LockMutex(lockCheckMutex);
	
	Uint32 totTime = SDL_GetTicks() - startTime;

	ofile.open("lockreport.txt");
	
	ofile << "Format: <name> <file> <line>: <avglocktime> over <locktimes>, <avgexectime> over <exectimes>, <avgunlocktime> over <unlocktimes>, <totlocktime>" << std::endl;
	
	int index = 0;
	for (std::vector<ThreadInfo>::iterator it = threads.begin(); it != threads.end(); it++, index++)
	{

		ofile << std::endl << it->file << " " << it->line << ": " << std::endl << std::endl;

		ofile << "All:" << std::endl;
		lockHash[index].Iterate(OutputLockItem);
		
		ofile << std::endl;
		ofile << "By Mutex:" << std::endl;
		lockHashByM[index].Iterate(OutputLockItem);
		
		ofile << std::endl;
		ofile << "By Position:" << std::endl;
		lockHashByPos[index].Iterate(OutputLockItem);
		
		ofile << std::endl;
		ofile << "By Unique:" << std::endl;
		lockHashByUniq[index].Iterate(OutputLockItem);
		
	}
		
	ofile << std::endl;
	ofile << "Locked mutexes:" << std::endl;

	for (std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.begin(); it != lockedItems.end(); it++)
	{
		OutputLockItem(it->second->byUniq);
	}

	ofile << std::endl;
	ofile << "Delay, locktime, condwait, time running by thread:" << std::endl;
	for (std::vector<ThreadInfo>::iterator it = threads.begin(); it != threads.end(); it++)
	{
		ofile << it->file << " " << it->line << ": " << it->delay << ", " << it->lockTime << ", " << it->condWait << ", " << (float) (totTime - it->delay - it->lockTime - it->condWait) / totTime << std::endl;
	}

	ofile << std::endl;
	ofile << "Total locking time spent: " << spentLTime << std::endl;
	ofile << "Total locks: " << numLocks << std::endl;
	ofile << "Total delay time spent: " << spentDTime << std::endl;
	ofile << "Total condwait spent: " << spentCTime << std::endl;
	ofile << "Total time: " << SDL_GetTicks() - startTime << std::endl;
	ofile << "Total number of threads: " << numThreads << std::endl;
	ofile << "Time running: " << (float) ((totTime * numThreads) - spentLTime - spentDTime - spentCTime) / totTime << std::endl;
	ofile << "Start time: " << startTime << std::endl;
	ofile << "Current time: " << SDL_GetTicks() << std::endl;

	ofile.close();

	UnlockMutex(lockCheckMutex);
}

