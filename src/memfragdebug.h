#include <set>
#include <exception> // for std::bad_alloc
#include <new>

#ifdef FRAGREPORT

void* operator new(size_t size) throw(std::bad_alloc);

void operator delete(void *p) throw();

void WriteFragmentationReport();

#endif

