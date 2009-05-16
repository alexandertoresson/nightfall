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
#include <deque>
#include <map>
#include <string>

namespace Window
{
	namespace GUI
	{

		class ordered_less_comparison
		{
			private:
				enum Status
				{
					LESS,
					EQUAL,
					GREATER
				} status;

			public:

				ordered_less_comparison(Status status = EQUAL) : status(status) {}

				template <typename T1, typename T2>
				ordered_less_comparison operator () (const T1& t1, const T2& t2)
				{
					if (status == LESS || (status == EQUAL && t1 < t2))
					{
						return ordered_less_comparison(LESS);
					}
					else if (status == EQUAL && t1 == t2)
					{
						return ordered_less_comparison(EQUAL);
					}
					else
					{
						return ordered_less_comparison(GREATER);
					}
				}

				operator bool()
				{
					return status == LESS;
				}
		};

		class FontHandle : public gc_ptr_from_this<FontHandle>
		{
			private:
				static std::deque<gc_ptr<FontHandle> > LRUQueue;
				static int fontCacheAmount;
				static int nextID;

				std::string filename;
				int ptSize;
				int faceIndex;
				bool failedLoading;

				TTF_Font* font;

				int id;

				void Unload();

				int GetPointSize(float size);
			public:
				FontHandle(const std::string& filename, int ptSize, int faceIndex);
				~FontHandle();
				TTF_Font* GetFont();

				int GetID()
				{
					return id;
				}

				void shade() {}
				
				static void static_shade()
				{
					gc_shade_container(LRUQueue);
				}
		};

		class TextRenderer
		{
			private:
				TextRenderer();
				~TextRenderer();
			public:
				static TextRenderer textRenderer;

				struct RenderedText
				{
					float w;
					float h;
					float maxX, maxY;
					GLuint texture;
				};

			private:
				int textCacheAmount;
				
				struct TextKey
				{
					const gc_ptr<FontHandle> fontHandle;
					const std::string text;

					TextKey(const gc_ptr<FontHandle>& fontHandle, const std::string& text) : fontHandle(fontHandle), text(text) {}
		
					bool operator < (const TextKey& a) const
					{
						return ordered_less_comparison()(fontHandle->GetID(), a.fontHandle->GetID())(text, a.text);
					}

					void shade() const
					{
						fontHandle.shade();
					}
				};

				std::map<TextKey, RenderedText> cachedText;
				std::deque<TextKey> LRUQueue;
				
			public:
				struct TextDimension
				{
					float w;
					float h;
				};

				float GetLineHeight(gc_ptr<FontHandle> fontHandle, float resolution);
				TextDimension GetTextSize(gc_ptr<FontHandle> fontHandle, const std::string& text, float resolution);
				int RenderText(gc_ptr<FontHandle> fontHandle, const std::string& text, TextRenderer::RenderedText& fonten);
				int RenderText(gc_ptr<FontHandle> fontHandle, const std::string& text, TextRenderer::RenderedText& fonten, float resolution);
				void shade() const
				{
					gc_shade_container(LRUQueue);
					gc_shade_map_key(cachedText);
				}

				static void static_shade()
				{
					textRenderer.shade();
				}
		};

		class Font
		{
			private:
				static std::set<Font> fontSet;

				std::string filename;
				int ptSize;
				bool underlined;
				int faceIndex;

				gc_ptr<FontHandle> fontHandle;

				bool changed;

				void UpdateFontHandle();

			public:
				Font(std::string filename = "", int ptSize = 10, bool underlined = false, int faceIndex = 0);

				void SetFilename(std::string fileName)
				{
					this->filename = filename;
					changed = true;
				}

				void SetPtSize(int ptSize)
				{
					this->ptSize = ptSize;
					changed = true;
				}

				void SetUnderlined(bool underlined)
				{
					this->underlined = underlined;
				}

				void SetFaceIndex(int faceIndex)
				{
					this->faceIndex = faceIndex;
					changed = true;
				}

				bool operator < (const Font& a) const
				{
					return ordered_less_comparison()(filename, a.filename)(ptSize, a.ptSize)(faceIndex, a.faceIndex);
				}

				float GetLineHeight(float resolution);
				TextRenderer::TextDimension GetTextSize(const std::string& text, float resolution);
				int RenderText(const std::string& text, TextRenderer::RenderedText& fonten);
				int RenderText(const std::string& text, TextRenderer::RenderedText& fonten, float resolution);

				void shade() const
				{
					fontHandle.shade();
				}
				
				static void static_shade()
				{
					gc_shade_container(fontSet);
				}
		};

		int InitDefaultFont(std::string filename);

		extern Font defaultFonts[5];

	}
}

#ifdef DEBUG_DEP
#warning "font.h-end"
#endif

#endif

