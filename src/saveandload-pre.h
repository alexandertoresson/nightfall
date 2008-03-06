#ifndef __SAVEANDLOAD_H_PRE__
#define __SAVEANDLOAD_H_PRE__

#ifdef DEBUG_DEP
#warning "saveandload.h-pre"
#endif

#include <string>

namespace Game
{
	namespace Dimension
	{
		void SaveGame(std::string filename);
		void LoadGameSaveFile(std::string filename);
		void LoadGame_PostLoad();
	}
}

#endif

