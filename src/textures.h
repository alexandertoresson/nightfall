#ifndef __TEXTURES_H__
#define __TEXTURES_H__
#include "sdlheader.h"
#include "console.h"
namespace Utilities
{
	// Initializes texture storage
	void InitTextures(int num_text);

	// Loads a texture, returns a handle/memory location
	// to it that might be passed as the second argument to glBindTexture()
	int LoadGLTexture(std::string file);
}
#endif

