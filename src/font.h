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
#ifndef FONT_H
#define FONT_H

#ifdef DEBUG_DEP
#warning "font.h"
#endif

#include "font-pre.h"

#include "sdlheader.h"
#include <queue>
#include <map>
#include <string>

namespace Window
{
	namespace GUI
	{
		extern FontCache Fonts;

		class FontCache
		{
			private:
				TTF_Font **font;
				int CacheAmount;
				SDL_Color textColor;
				int fonttype;
				int fontcolor;
				
				struct FontTexture
				{
					float w; //0 - 1
					float h; //0 - 1
					GLuint Texture;
					GLfloat *texCoords;
					std::string Text;
					int fonttype;
					int fontcolor;
					FontTexture *nxt;
				};
				
				struct FontContainer
				{
					FontTexture *first;
				};

				int GetPointSize(float size);
				std::map<std::string, FontContainer> CachedText;
				std::queue<FontTexture*> CachedTexture;
			public:
				struct RenderedText
				{
					float w;
					float h;
					GLuint Texture;
					GLfloat *texCoords;
				};

				struct TextDimension
				{
					float w;
					float h;
				};

				FontCache();
				FontCache(int);
				~FontCache();
				int LoadFont(std::string path);

				float GetLineHeight(float resolution);
				TextDimension GetTextSize(std::string text, float resolution);
				int RenderText(std::string text, FontCache::RenderedText& Fonten);
				int RenderText(std::string text, FontCache::RenderedText& Fonten, float resolution);
				void SetColor(Uint8 r, Uint8 b, Uint8 g);
				void SetFontType(int type);
		};

		int InitFont(std::string);
		void KillFontSystem(void);
	}
}

#ifdef DEBUG_DEP
#warning "font.h-end"
#endif

#endif

