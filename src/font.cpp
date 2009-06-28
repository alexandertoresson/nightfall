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
#include "font.h"

#include "utilities.h"
#include "errors.h"
#include "game.h"
#include "vfs.h"
#include "window.h"
#include <sstream>
#include <cmath>
#include <iostream>

namespace Window
{
	namespace GUI
	{
		std::deque<gc_ptr<FontHandle> > FontHandle::LRUQueue;
		int FontHandle::fontCacheAmount = 32;
		int FontHandle::nextID = 0;
		std::set<Font> Font::fontSet;

		FontHandle::FontHandle(const std::string& filename, int ptSize, int faceIndex) :
		           filename(filename), ptSize(ptSize), faceIndex(faceIndex), failedLoading(false), font(NULL), id(nextID++)
		{
		}

		FontHandle::~FontHandle()
		{
			Unload();
		}

		void FontHandle::Unload()
		{
			if (font)
			{
				TTF_CloseFont(font);
				font = NULL;
			}
		}

		int FontHandle::GetPointSize(float size)
		{
			float factor = (float)Window::windowHeight / 480.0f;
			return (int)floor((factor * size) + 0.5f) + 1;
		}
		
		TTF_Font* FontHandle::GetFont()
		{
			if (font)
			{
				return font;
			}
			else
			{
				if (failedLoading)
					return NULL;

				if (LRUQueue.size() == (unsigned) fontCacheAmount)
				{
					LRUQueue.front()->Unload();
					LRUQueue.pop_front();
				}

				std::string path = Utilities::VFS::ResolveReadable("/data/fonts/" + filename);

				if (!path.length())
				{
					std::cout << "Could not find font \"" << filename << "\"!" << std::endl;
					failedLoading = true;
					return NULL;
				}

				std::cout << "Loading font file \"" << path << "\"" << std::endl;
			
				if(!(font = TTF_OpenFontIndex(path.c_str(), GetPointSize(ptSize), faceIndex)))
				{
					std::cout << "Couldn't open font file \"" << path << "\". SDL_ttf Reports: \"" << TTF_GetError() << "\"" << std::endl;
					failedLoading = true;
				}

				if (font)
					LRUQueue.push_back(GetRef());

				return font;
			}
		}

		Font::Font()
		{
			*this = defaultFonts[2];
		}

		Font::Font(std::string filename, int ptSize, bool underlined, int faceIndex) : 
	     	     filename(filename), ptSize(ptSize), underlined(underlined), faceIndex(faceIndex)
		{
			changed = true;
		}

		void Font::UpdateFontHandle()
		{
			if (changed)
			{
				std::set<Font>::iterator it = fontSet.find(*this);
				if (it != fontSet.end())
				{
					fontHandle = it->fontHandle;
				}
				else
				{
					fontHandle = new FontHandle(filename, ptSize, faceIndex);
					fontSet.insert(*this);
				}
				changed = false;
			}
		}

		float Font::GetLineHeight(float resolution)
		{
			UpdateFontHandle();
			return TextRenderer::textRenderer.GetLineHeight(fontHandle, resolution);
		}

		TextRenderer::TextDimension Font::GetTextSize(const std::string& text, float resolution)
		{
			UpdateFontHandle();
			return TextRenderer::textRenderer.GetTextSize(fontHandle, text, resolution);
		}

		int Font::RenderText(const std::string& text, TextRenderer::RenderedText& fonten)
		{
			UpdateFontHandle();
			return TextRenderer::textRenderer.RenderText(fontHandle, text, fonten);
		}

		int Font::RenderText(const std::string& text, TextRenderer::RenderedText& fonten, float resolution)
		{
			UpdateFontHandle();
			return TextRenderer::textRenderer.RenderText(fontHandle, text, fonten, resolution);
		}

		TextRenderer TextRenderer::textRenderer;

		TextRenderer::TextRenderer()
		{
			if(TTF_Init() == -1) 
			{
				std::cout << "Font Init failure: \"" << TTF_GetError() << "\"" << std::endl;
				exit(FONT_ERROR_INIT_FAILURE);
			}
			this->textCacheAmount = 128;
		}

		TextRenderer::~TextRenderer()
		{
			if (!TTF_WasInit())
				return;
			
			TTF_Quit();
		}

		float TextRenderer::GetLineHeight(gc_ptr<FontHandle> fontHandle, float resolution)
		{
			int height = TTF_FontHeight(fontHandle->GetFont());
			return (float)height * resolution;
		}

		//Fixes texture coordinates for GUI relative coordinate system
		int TextRenderer::RenderText(gc_ptr<FontHandle> fontHandle, const std::string& text, TextRenderer::RenderedText& fonten)
		{
			return RenderText(fontHandle, text, fonten, 1.0f / (float)Window::windowHeight);
		}

		TextRenderer::TextDimension TextRenderer::GetTextSize(gc_ptr<FontHandle> fontHandle, const std::string& text, float resolution)
		{
			int w = 0;
			int h = 0;
			TTF_SizeUTF8(fontHandle->GetFont(), text.c_str(), &w, &h);

			TextDimension dim;
			dim.w = (float) w * resolution;
			dim.h = (float) h * resolution;
			return dim;
		}

		int TextRenderer::RenderText(gc_ptr<FontHandle> fontHandle, const std::string& text, TextRenderer::RenderedText& fonten, float resolution)
		{
			SDL_Color color;
			color.r = 0xFF;
			color.g = 0xFF;
			color.b = 0xFF;
			color.unused = 0xFF;

			//Check whether the text is already cached
			std::map<TextKey, RenderedText>::iterator it = cachedText.find(TextKey(fontHandle, text));
			if (it != cachedText.end())
			{
				fonten = it->second;
				return SUCCESS;
			}

			SDL_Surface *text_surface = TTF_RenderUTF8_Blended(fontHandle->GetFont(),text.c_str(),color);
			if (!text_surface)
			{
				std::cout << "Render Text failed: " << TTF_GetError() << std::endl;
				return ERROR_GENERAL;
			} 
			else 
			{
				if (LRUQueue.size() == (unsigned) textCacheAmount)
				{
					//Delete the oldest
					const RenderedText& old = cachedText[LRUQueue.front()];
					glDeleteTextures(1, &old.texture);
					cachedText.erase(LRUQueue.front());
					LRUQueue.pop_front();
				}
				fonten.w = (float)text_surface->w * resolution;
				fonten.h = (float)text_surface->h * resolution;	
				
				int w2 = Utilities::power_of_two(text_surface->w);
				int h2 = Utilities::power_of_two(text_surface->h);
				/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
				   as expected by OpenGL for textures */
				SDL_Surface *surface;
				Uint32 rmask, gmask, bmask, amask;
					/* SDL interprets each pixel as a 32-bit number, so our masks must depend
				   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				rmask = 0xff000000;
				gmask = 0x00ff0000;
				bmask = 0x0000ff00;
				amask = 0x000000ff;
#else
				rmask = 0x000000ff;
				gmask = 0x0000ff00;
				bmask = 0x00ff0000;
				amask = 0xff000000;
#endif

				surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w2, h2, 32, rmask, gmask, bmask, amask);
				if (surface == NULL)
				{
					std::cout << "Font creation failure! Couldn't create SDL_Surface: " << SDL_GetError() << std::endl;
					exit(ERROR_GENERAL);
				}
				SDL_SetAlpha(text_surface, 0, SDL_ALPHA_OPAQUE);
				if (SDL_BlitSurface(text_surface, NULL, surface, NULL) == -1)
				{
					std::cout << "BLIT ERROR!!!" << std::endl;
				}
					
				GLuint texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);

				// gl parameters
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	

				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

				float maxX = (float)text_surface->w / (float)w2;
				float maxY = (float)text_surface->h / (float)h2;
				fonten.maxX = maxX;
				fonten.maxY = maxY;

				fonten.texture = texture;

				cachedText[TextKey(fontHandle, text)] = fonten;
				LRUQueue.push_front(TextKey(fontHandle, text));

				//perhaps we can reuse it, but I assume not for simplicity.
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(surface);
			}

			return SUCCESS;
		}

		int InitDefaultFont(std::string filename)
		{
			defaultFonts[0] = Font(filename, 10);
			defaultFonts[1] = Font(filename, 12);
			defaultFonts[3] = Font(filename, 16);
			defaultFonts[4] = Font(filename, 20);
			defaultFonts[2] = Font(filename, 14);
			return SUCCESS;
		}
		
		Font defaultFonts[5] = {Font(""), Font(""), Font(""), Font(""), Font("")};
	}
}

