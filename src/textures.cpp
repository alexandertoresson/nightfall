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
#include "textures.h"

#include "networking.h"
#include "vfs.h"
#include "game.h"
#include "unit.h"

namespace Utilities
{	
	GLuint *texture = NULL;
	int cur_texture = 0, num_textures = 0;

	// Initializes texture storage
	void InitTextures(int num_text)
	{
		if (!texture)
			texture = new GLuint[num_text];
		glGenTextures(num_text, &texture[0]);
		num_textures = num_text;
	}

	// Loads a texture, returns a handle/memory location
	// to it that might be passed as the second argument to glBindTexture()
	int LoadGLTexture(std::string file)
	{
		if (Game::Rules::noGraphics)
			return 0;
		SDL_Surface *TextureImage[1];

		if (cur_texture >= num_textures)
		{
			console << Console::err << "Error: Attempted to load more textures than was allocated space for" << Console::nl;
			return 0;
		}

		std::string filename = VFS::ResolveReadable(file);

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

