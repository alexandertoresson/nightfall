
#ifndef __LOCKCHECKER_H__
#define __LOCKCHECKER_H__

#ifdef SDL_LockMutex
#undef SDL_LockMutex
#endif

#ifdef SDL_UnlockMutex
#undef SDL_UnlockMutex
#endif

#define SDL_LockMutex(x) CheckLock(x, #x, __FILE__, __LINE__)
#define SDL_UnlockMutex(x) CheckUnlock(x, #x, __FILE__, __LINE__)
#define SDL_Delay(n) RecordDelay(n)
#define SDL_CreateThread(fn, data) RecordCreateThread(fn, data, __FILE__, __LINE__)
#define SDL_CondWait(cond, mutex) RecordCondWait(cond, mutex)

#include <string>

void CheckLock(SDL_mutex* m, std::string name, std::string file, unsigned line);
void CheckUnlock(SDL_mutex* m, std::string name, std::string file, unsigned line);
void WriteLockReport(std::string file);
void RecordDelay(unsigned n);
SDL_Thread* RecordCreateThread(int (*fn)(void *), void *data, std::string file, unsigned line);
void RecordCondWait(SDL_cond* cond, SDL_mutex* mutex);

#endif