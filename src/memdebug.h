#ifndef __MEMDEBUG_H__
#define __MEMDEBUG_H__

#include <set>
#include <exception> // for std::bad_alloc
#include <new>

/*void* operator new(size_t size, std::string file, unsigned line) throw(std::bad_alloc);

void operator delete(void *p) throw();

void* operator new[](size_t size, std::string, unsigned line) throw(std::bad_alloc);

void operator delete[](void *p) throw();*/

void WriteFragmentationReport(std::string filename);

void WriteLeakReport(std::string filename);

//#define new new(__FILE__, __LINE__)

#endif

