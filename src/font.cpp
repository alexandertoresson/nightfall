#include "font.h"

#include "utilities.h"
#include "errors.h"
#include "game.h"
#include "paths.h"
#include <sstream>

using namespace std;

namespace Window
{
	namespace GUI
	{
		FontCache Fonts;

		FontCache::FontCache()
		{
			if(TTF_Init() == -1) 
			{
				cout << "Font Init failure: \"" << TTF_GetError() << "\"" << endl;
				exit(FONT_ERROR_INIT_FAILURE);
			}
//			atexit(TTF_Quit); << We're already using this function for Window::OnExit.
			this->CacheAmount = 128;
			this->textColor.r = 255;
			this->textColor.g = 255;
			this->textColor.b = 255;
			this->textColor.unused = 0;
			this->fontcolor = 255 << 16 | 255 << 8 | 255;
			this->fonttype = 2;
			this->font = new TTF_Font*[5];
		}

		FontCache::FontCache(int CacheAmount)
		{
			FontCache();
			this->CacheAmount = CacheAmount;
		}
		
		int FontCache::GetPointSize(float size)
		{
			float factor = (float)Window::windowHeight / 480.0f;
			return (int)floor((factor * size) + 0.5f) + 1;
		}
		
		float FontCache::GetLineHeight(float resolution)
		{
			int height = TTF_FontHeight(this->font[this->fonttype]);
			return (float)height * resolution;
		}

		int FontCache::LoadFont(string filename)
		{
			std::string path = Utilities::GetDataFile(filename);

			if (!path.length())
			{
				cout << "Could not find font \"" << filename << "\"!" << endl;
				return FONT_ERROR_FILE_LOAD;
			}

			cout << "Loading font file \"" << path << "\"" << endl;
			//10 pt at 640 x 480, relative font calculation
			float factor = (float)Window::windowHeight / 480.0f;
			int pt = (int)floor((factor * 14.0f) + 0.5f) + 1;
			cout << pt << endl;

			this->font[2] = TTF_OpenFont(path.c_str(), GetPointSize(14));

			if(!this->font[2])
			{
				cout << path << endl;
				cout << "Font Init failure: Couldn't open font file \"" << path << "\". SDL Reports: \"" << TTF_GetError() << "\"" << endl;
				return FONT_ERROR_FILE_LOAD;
			}

			this->font[0] = TTF_OpenFont(path.c_str(), GetPointSize(10));
			this->font[1] = TTF_OpenFont(path.c_str(), GetPointSize(12));
			this->font[3] = TTF_OpenFont(path.c_str(), GetPointSize(16));
			this->font[4] = TTF_OpenFont(path.c_str(), GetPointSize(20));
			return SUCCESS;
		}

		//Fixes texture coordinates for GUI relative coordinate system
		int FontCache::RenderText(string text, FontCache::RenderedText& Fonten)
		{
			return RenderText(text, Fonten, 1.0f / (float)Window::windowHeight);
		}

		FontCache::TextDimension FontCache::GetTextSize(string text, float resolution)
		{
			int w = 0;
			int h = 0;
			TTF_SizeUTF8(font[fonttype],text.c_str(), &w, &h);

			TextDimension dim;
			dim.w = (float) w * resolution;
			dim.h = (float) h * resolution;
			return dim;
		}

		int FontCache::RenderText(string text, FontCache::RenderedText& Fonten, float resolution)
		{
			//Check so that it isn't cached
			map<string, FontCache::FontContainer>::iterator location = CachedText.find(text);
			if(location != CachedText.end())
			{
				FontTexture* Cached = (*location).second.first;
				while(Cached != NULL)
				{
					if(Cached->fontcolor == fontcolor && Cached->fonttype == fonttype)
					{
//						FontCache::RenderedText FontTexten; <--- unused
						Fonten.w = Cached->w;
						Fonten.h = Cached->h;
						Fonten.Texture = Cached->Texture;
						Fonten.texCoords = Cached->texCoords;
						return SUCCESS;
					}
					Cached = Cached->nxt;
				}
			}

			SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font[fonttype],text.c_str(),textColor);
			if(!text_surface)
			{
				cout << "Render Text failed: " << TTF_GetError() << endl;
				return ERROR_GENERAL;
			} 
			else 
			{
				if(CachedTexture.size() + 1 > (unsigned) this->CacheAmount)
				{
					//Delete the oldest
					FontTexture* data = (FontTexture*)(CachedTexture.front());
					CachedTexture.pop();
					map<string, FontCache::FontContainer>::iterator loc = CachedText.find(data->Text);
					if(loc != CachedText.end())
					{
						if((*loc).second.first->nxt == NULL)
						{
							CachedText.erase(loc);
						}
						else
						{
							FontTexture *prev = (*loc).second.first;
							FontTexture *ptr = prev;
							while(ptr != NULL)
							{
								if(ptr == data)
									break;
							}
							if(ptr == prev)
							{
								(*loc).second.first = ptr;
							}
							else
							{
								if(ptr->nxt != NULL)
									prev->nxt = NULL;
								else
									prev->nxt = ptr->nxt;
							}
						}
					}
					glDeleteTextures(1, &data->Texture);
					delete [] data->texCoords;
					delete data;
				}
				FontCache::FontTexture *fonttexture = new FontCache::FontTexture;				
				fonttexture->w = (float)text_surface->w * resolution;
				fonttexture->h = (float)text_surface->h * resolution;	
				fonttexture->Text = text;
				fonttexture->fonttype = fonttype;
				fonttexture->fontcolor = fontcolor;
				fonttexture->nxt = NULL;
				//int w = 0;
				//int h = 0;
				//TTF_SizeUTF8(font, text.c_str(), w&, h&)
				
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
				if(surface == NULL) {
					cout << "Font creatation failure! Couldn't create SDL_Surface: " << SDL_GetError() << endl;
					exit(ERROR_GENERAL);
				}
				SDL_SetAlpha(text_surface, 0, SDL_ALPHA_OPAQUE);
				if(SDL_BlitSurface(text_surface, NULL, surface, NULL) == -1)
				{
					cout << "BLIT ERROR!!!" << endl;
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
				fonttexture->texCoords = new GLfloat[8];
				fonttexture->texCoords[0] = 0.0f;
				fonttexture->texCoords[1] = 0.0f;

				fonttexture->texCoords[2] = maxX;
				fonttexture->texCoords[3] = 0.0f;

				fonttexture->texCoords[4] = maxX;
				fonttexture->texCoords[5] = maxY;

				fonttexture->texCoords[6] = 0.0f;
				fonttexture->texCoords[7] = maxY;

				fonttexture->Texture = texture;

				map<string, FontCache::FontContainer>::iterator newlocation = CachedText.find(text);
				if(newlocation == CachedText.end())
				{
					FontContainer cont;
					cont.first = fonttexture;
					CachedText[text] = cont;
				}
				else
				{
					fonttexture->nxt = (*newlocation).second.first;
					(*newlocation).second.first = fonttexture;
				}
				this->CachedTexture.push(fonttexture);
					
				//perhaps we can reuse it, but I assume not for simplicity.
				SDL_FreeSurface(text_surface);
				SDL_FreeSurface(surface);

				Fonten.w = fonttexture->w;
				Fonten.h = fonttexture->h;
				Fonten.Texture = fonttexture->Texture;
				Fonten.texCoords = fonttexture->texCoords;
			}

			return SUCCESS;
		}

		void FontCache::SetColor(Uint8 r, Uint8 g, Uint8 b)
		{
			this->textColor.r = r;
			this->textColor.g = g;
			this->textColor.b = b;
			this->textColor.unused = 0;
			this->fontcolor = (int)r << 16 | (int)g << 8 | (int)b;
		}

		void FontCache::SetFontType(int type)
		{
			this->fonttype = type;
		}

		FontCache::~FontCache()
		{
			//Deallocate all textures
		}

		int InitFont(std::string fontName)
		{
			if(Fonts.LoadFont(fontName) == SUCCESS)
				return SUCCESS;
			return FONT_ERROR_FILE_LOAD;
		}

		void KillFontSystem(void)
		{
			if (!TTF_WasInit())
				return;
			
			TTF_Quit();
		}
	}
}

