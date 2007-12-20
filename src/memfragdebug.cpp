#include "memfragdebug.h"
#include <cstdlib> // for malloc() and free()
#include <map>
#include <iostream>
#include <fstream>

#define HASH_SIZE 3331337
#define HASH_FUNC(x) ((unsigned long)x % HASH_SIZE)

struct hash_entry
{
	void* pnt;
	size_t size;
	hash_entry *next;
};

hash_entry **hash_table;

void* operator new (size_t size) throw(std::bad_alloc)
{
	void *p=malloc(size); 
	if (p==0) // did malloc succeed?
    		throw std::bad_alloc(); // ANSI/ISO compliant behavior

	if (!hash_table)
	{
		hash_table = (hash_entry**) calloc(HASH_SIZE, sizeof(hash_entry*));
	}

	hash_entry *hentry = (hash_entry*) malloc(sizeof(hash_entry));
	hentry->pnt = p;
	hentry->next = NULL;
	hentry->size = size;
	
	int h = HASH_FUNC(p);

	if (!hash_table[h])
	{
		hash_table[h] = hentry;
	}
	else
	{
		hash_entry *curentry =  hash_table[h];
		while (curentry->next)
		{
			curentry = curentry->next;
		}
		curentry->next = hentry;
	}

 	return p;
}

void operator delete (void *p) throw ()
{
	int h = HASH_FUNC(p);	
	hash_entry *curentry = hash_table[h];
	hash_entry *lastentry = NULL;
	bool found = false;

	while (curentry)
	{
		if (curentry->pnt == p)
		{
			found = true;
			if (lastentry)
			{
				lastentry->next = curentry->next;
			}
			else
			{
				hash_table[h] = NULL;
			}
			free(curentry);
			break;
		}
		lastentry = curentry;
		curentry = curentry->next;
	}

	if (found)
	{
 		free(p); 
 	}
}

void WriteFragmentationReport()
{
	std::ofstream ofile("fragreport.txt");
	for (int i = 0; i < HASH_SIZE; i++)
	{
		hash_entry *curentry = hash_table[i];
		while (curentry)
		{
			ofile << curentry->pnt << " " << curentry->size << std::endl;
			curentry = curentry->next;
		}
	}
	ofile.close();
}

