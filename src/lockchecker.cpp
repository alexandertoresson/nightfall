
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
		unsigned (*hash)(LockItem*);

	public:

		LockHash(unsigned numItems, bool (*cmp)(LockItem*, LockItem*), unsigned (*hash)(LockItem*))
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
			unsigned k = hash(newItem) % numItems;
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

		void Iterate(void (*callback)(LockItem*))
		{
			for (unsigned i = 0; i < numItems; i++)
			{
				LockItem* curItem = items[i];
				while (curItem)
				{
					callback(curItem);
					curItem = curItem->next;
				}
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

unsigned hash_all(LockItem* a)
{
	return (unsigned) a->m + a->line;
}

bool cmp_all(LockItem* a, LockItem* b)
{
	return a->m == b->m && a->file == b->file && a->line == b->line;
}

unsigned hash_m(LockItem* a)
{
	return (unsigned) a->m;
}

bool cmp_m(LockItem* a, LockItem* b)
{
	return a->m == b->m;
}

unsigned hash_pos(LockItem* a)
{
	return a->line;
}

bool cmp_pos(LockItem* a, LockItem* b)
{
	return a->file == b->file && a->line == b->line;
}

unsigned hash_uniq(LockItem* a)
{
	return 0;
}

bool cmp_uniq(LockItem* a, LockItem* b)
{
	return (a->m == b->m) || (a->name == b->name) || (a->file == b->file && a->line == b->line);
}

LockHash lockHash(256, cmp_all, hash_all);
LockHash lockHashByM(256, cmp_m, hash_m);
LockHash lockHashByPos(256, cmp_pos, hash_pos);
LockHash lockHashByUniq(1, cmp_uniq, hash_uniq);

SDL_mutex *lockCheckMutex = SDL_CreateMutex();

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

void CheckLock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	Uint32 locker = SDL_ThreadID();
	if (startTime == 0)
	{
		startTime = SDL_GetTicks();
	}
	LockMutex(lockCheckMutex);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		threads.push_back(tI);
	}
	std::map<SDL_mutex*, LockedItems*>::iterator it = lockedItems.find(m);

	if (it != lockedItems.end() && it->second->locker == locker)
	{
		LockedItems* lI = it->second;
		std::cout << "Error: Attempted to lock already locked mutex " << name << " from same thread at " << file << ":" << line << std::endl;
		std::cout << "Originally locked at " << lI->file << ":" << lI->line << std::endl;
		return;
	}

	LockedItems* lI = new LockedItems;
	lI->all = lockHash.getItem(m, file, line, name);
	lI->byM = lockHashByM.getItem(m, file, line, name);
	lI->byPos = lockHashByPos.getItem(m, file, line, name);
	lI->byUniq = lockHashByUniq.getItem(m, file, line, name);
	lI->m = m;
	lI->locker = locker;
	lI->file = file;
	lI->line = line;
	UnlockMutex(lockCheckMutex);

	Uint32 sTime = SDL_GetTicks();

	LockMutex(m);

	LockMutex(lockCheckMutex);
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
	threads[idToIndex[SDL_ThreadID()]].lockTime += totTime;
	numLocks++;

	lockedItems[m] = lI;
	UnlockMutex(lockCheckMutex);
	
}

void CheckUnlock(SDL_mutex* m, std::string name, std::string file, unsigned line)
{
	LockedItems* lI = NULL;
	LockMutex(lockCheckMutex);
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

	Uint32 cTime = SDL_GetTicks();
	lockedItems.erase(m);
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
	threads[idToIndex[SDL_ThreadID()]].lockTime += eTime;

	delete lI;
	
	UnlockMutex(lockCheckMutex);
}

void RecordDelay(unsigned n)
{
	Uint32 sTime = SDL_GetTicks();
	Delay(n);
	LockMutex(lockCheckMutex);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		threads.push_back(tI);
	}
	Uint32 eTime = SDL_GetTicks() - sTime;
	spentDTime += eTime;
	threads[idToIndex[SDL_ThreadID()]].delay += eTime;
	UnlockMutex(lockCheckMutex);
}

void RecordCondWait(SDL_cond* cond, SDL_mutex* mutex)
{
	Uint32 sTime = SDL_GetTicks();
	CondWait(cond, mutex);
	if (startTime == 0)
	{
		startTime = SDL_GetTicks();
	}
	LockMutex(lockCheckMutex);
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		threads.push_back(tI);
	}
	Uint32 eTime = SDL_GetTicks() - sTime;
	spentCTime += eTime;
	threads[idToIndex[SDL_ThreadID()]].condWait += eTime;
	UnlockMutex(lockCheckMutex);
}

SDL_Thread* RecordCreateThread(int (*fn)(void *), void *data, std::string file, unsigned line)
{
	SDL_Thread *thread;
	thread = CreateThread(fn, data);
	LockMutex(lockCheckMutex);
	
	if (threads.size() == 0)
	{
		ThreadInfo tI;
		tI.file = "main";
		tI.line = 0;
		threads.push_back(tI);
	}

	ThreadInfo tI;
	tI.file = file;
	tI.line = line;
	tI.delay = 0;
	threads.push_back(tI);

	numThreads++;
	idToIndex[SDL_GetThreadID(thread)] = numThreads-1;
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
	LockMutex(lockCheckMutex);
	
	Uint32 totTime = SDL_GetTicks() - startTime;

	ofile.open("lockreport.txt");
	
	ofile << "Format: <name> <file> <line>: <avglocktime> over <locktimes>, <avgexectime> over <exectimes>, <avgunlocktime> over <unlocktimes>, <totlocktime>" << std::endl;
	
	ofile << "All:" << std::endl;
	lockHash.Iterate(OutputLockItem);
	
	ofile << std::endl;
	ofile << "By Mutex:" << std::endl;
	lockHashByM.Iterate(OutputLockItem);
	
	ofile << std::endl;
	ofile << "By Position:" << std::endl;
	lockHashByPos.Iterate(OutputLockItem);
	
	ofile << std::endl;
	ofile << "By Unique:" << std::endl;
	lockHashByUniq.Iterate(OutputLockItem);
	
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

	ofile.close();

	UnlockMutex(lockCheckMutex);
}
