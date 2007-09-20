#include "textures.h"

#include "networking.h"
#include "paths.h"

namespace Utilities
{	
	GLuint *texture;
	int cur_texture = 0, num_textures = 0;

	// Initializes texture storage
	void InitTextures(int num_text)
	{
		texture = new GLuint[num_text];
		glGenTextures(num_text, &texture[0]);
		num_textures = num_text;
	}

	// Loads a texture, returns a handle/memory location
	// to it that might be passed as the second argument to glBindTexture()
	int LoadGLTexture(std::string file)
	{
		if (Game::Networking::isDedicatedServer)
			return 0;
		SDL_Surface *TextureImage[1];

		if (cur_texture >= num_textures)
		{
			console << Console::err << "Error: Attempted to load more textures than was allocated space for" << Console::nl;
			return 0;
		}

		std::string filename = GetDataFile(file);

		if (!filename.length())
		{
			return 0;
		}

		// Load the texture using SDL_Image
		TextureImage[0] = IMG_Load(filename.c_str());
		if (TextureImage[0])
		{
			glBindTexture(GL_TEXTURE_2D, texture[cur_texture]);

			// use bilinear scaling for scaling down and trilinear scaling for scaling up
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate mipmaps using GLU
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->w, TextureImage[0]->h, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->pixels);

			// free the texture loaded with SDL_Image
			SDL_FreeSurface(TextureImage[0]);
		}
		return texture[cur_texture++];
	}
}

