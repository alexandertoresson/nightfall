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
#define SDL_CondWait(cond, mutex) RecordCondWait(cond, mutex, #mutex, __FILE__, __LINE__)

#include <string>

void CheckLock(SDL_mutex* m, std::string name, std::string file, unsigned line);
void CheckUnlock(SDL_mutex* m, std::string name, std::string file, unsigned line);
void WriteLockReport(std::string file);
void RecordDelay(unsigned n);
SDL_Thread* RecordCreateThread(int (*fn)(void *), void *data, std::string file, unsigned line);
void RecordCondWait(SDL_cond* cond, SDL_mutex* mutex, std::string name, std::string file, unsigned line);

#endif
