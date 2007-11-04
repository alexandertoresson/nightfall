#ifndef __SAVEANDLOAD_H_PRE__
#define __SAVEANDLOAD_H_PRE__

#ifdef DEBUG_DEP
#warning "saveandload.h-pre"
#endif

namespace Game
{
	namespace Dimension
	{
	}
}

#define __SAVEANDLOAD_H_PRE_END__

#endif

#ifndef __SAVEANDLOAD_H__
#define __SAVEANDLOAD_H__

#include <string>

#ifdef DEBUG_DEP
#warning "saveandload.h"
#endif

namespace Game
{
	namespace Dimension
	{
		void SaveGame(std::string filename);
		void LoadGame(std::string filename);
	}
}

#ifdef DEBUG_DEP
#warning "saveandload.h-end"
#endif

#define __SAVEANDLOAD_H_END__

#endif
