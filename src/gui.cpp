#include "gui.h"

#include "dimension.h"
#include "window.h"
#include "console.h"
#include "terrain.h"
#include "networking.h"
#include "game.h"

using namespace std;
using namespace Window;
//Notes:
// Ifall mus kordinaterna är utanför hanterabart område ska eventen dö ut.
// Ifall tangent events saknar focuserad element att skicka till ska det skickas till spelmotorn.

//#define DEBUG_MOUSE_COORDS
//#define DEBUG_PANEL

namespace Window
{
	namespace GUI 
	{
		Uint32 last_frame;
		float time_since_last_frame;

		inline void BilinearInterpolation(Uint8 *v, Uint8* result, float x, float y)
		{
			//r0 = 0, r1 = 3, r2 = 6, r3 = 9
			Uint8 r0 = (Uint8)((v[3] - v[0]) * x + v[0]);
			Uint8 r1 = (Uint8)((v[9] - v[6]) * x + v[6]);
			Uint8 r = (Uint8)((r1 - r0) * y + r0);

			//g0 = 1, g1 = 4, g2 = 7, g3 = 10
			Uint8 g0 = (Uint8)((v[4] - v[1]) * x + v[1]);
			Uint8 g1 = (Uint8)((v[10] - v[7]) * x + v[7]);
			Uint8 g = (Uint8)((g1 - g0) * y + g0);

			//b0 = 2, b1 = 5, b2 = 8, b3 = 11
			Uint8 b0 = (Uint8)((v[5] - v[2]) * x + v[2]);
			Uint8 b1 = (Uint8)((v[11] - v[8]) * x + v[8]);
			Uint8 b = (Uint8)((b1 - b0) * y + b0);

			result[0] = r;
			result[1] = g;
			result[2] = b;
		}

		inline float LinearInterpolate(float v0, float v1, float x)
		{
			return (v1 - v0) * x + v0;
		}
		
		inline void GetPixel(float x, float y, SDL_Surface *texture, Uint8* result)
		{
			int px = (int)(x * texture->w);
			int py = (int)(y * texture->h);

			float vx = (x * texture->w) - px;
			float vy = (y * texture->h) - py;
			
			Uint8 v[12];
			Uint8 *tv = (Uint8*)&v;
			
			Uint8 *end = tv + 6;

			Uint8 *pxpos = (Uint8*)texture->pixels + texture->pitch * py + px * 3;
			for(; tv < end; tv++, pxpos++)
				*tv = *pxpos;
			
			end += 6;
			pxpos = (Uint8*)texture->pixels + texture->pitch * (py + 1) + px * 3;
			for(; tv < end; tv++, pxpos++)
				*tv = *pxpos;

			BilinearInterpolation(v, result, vx, vy);
		}

		GLuint CreateMap(SDL_Surface* texture, Game::Dimension::HeightMap* map, int mapw, int maph, int resx, int resy)
		{
			console << Console::imp << "Generating 2D terrain map from " << mapw << "x" << maph << " to " << resx << "x" << resy << "..." << Console::nl;
			Uint32 start = SDL_GetTicks();
			Uint8 *pic = new Uint8[resx * resy * 3];
			
			Uint8 *tmppic = pic;
			float xa = (mapw - 1) / (float)resx;
			float ya = (maph - 1) / (float)resy;

			float xf = 0.0f;
			float yf = 0.0f;

			for(int y = 0; y < resy ; y++, yf += ya)
			{
				int my = (int)yf;
				int my2 = (int)(yf + ya);

				float ry = yf - floorf(yf);
				xf = 0.0f;
				for(int x = 0; x < resx; x++, xf += xa)
				{
					int mx = (int)xf;
					int mx2 = (int)(xf + xa);
					float rx = xf - floorf(xf);
					if(mx == mx2 || my == my2 || my2 == my + 1 || mx2 == mx + 1) //No average calculation
					{
						if (Game::Dimension::GetTerrainHeightHighestLevel(xf, yf) > Game::Dimension::waterLevel)
						{
							Game::Dimension::UVWCoord *v0 = map->ppTexCoords[my][mx];
							Game::Dimension::UVWCoord *v1 = map->ppTexCoords[my][mx+1];
							Game::Dimension::UVWCoord *v2 = map->ppTexCoords[my+1][mx];
							Game::Dimension::UVWCoord *v3 = map->ppTexCoords[my+1][mx+1];
						
							float vx = LinearInterpolate(LinearInterpolate(v0->u, v1->u, rx), LinearInterpolate(v2->u, v3->u, rx), ry);
							float vy = LinearInterpolate(LinearInterpolate(v0->v, v1->v, rx), LinearInterpolate(v2->v, v2->v, rx), ry);

							GetPixel(vx,vy, texture, tmppic);
							tmppic += 3;
						}
						else
						{
							*tmppic = (Uint8)(Game::Dimension::waterColor[0] * 255);
							tmppic++;
							*tmppic = (Uint8)(Game::Dimension::waterColor[1] * 255);
							tmppic++;
							*tmppic = (Uint8)(Game::Dimension::waterColor[2] * 255);
							tmppic++;
						}
					}
					else
					{
						//Average calculation.
						int mxw = mx2 - mx; //points in x
						int myw = my2 - my; //points in y

						int r = 0;
						int g = 0;
						int b = 0;

						Uint8 v[3];
						for(int ay = my; ay < my2; ay++)
						{
							for(int ax = mx; ax < mx2; ax++)
							{		
								if (Game::Dimension::GetTerrainHeightHighestLevel(ax+rx, ay+ry) > Game::Dimension::waterLevel)
								{
									Game::Dimension::UVWCoord *v0 = map->ppTexCoords[ay][ax];
									Game::Dimension::UVWCoord *v1 = map->ppTexCoords[ay][ax+1];
									Game::Dimension::UVWCoord *v2 = map->ppTexCoords[ay+1][ax];
									Game::Dimension::UVWCoord *v3 = map->ppTexCoords[ay+1][ax+1];
									float vx = LinearInterpolate(LinearInterpolate(v0->u, v1->u, rx), LinearInterpolate(v2->u, v3->u, rx), ry);
									float vy = LinearInterpolate(LinearInterpolate(v0->v, v1->v, rx), LinearInterpolate(v2->v, v2->v, rx), ry);
									GetPixel(vx,vy, texture, (Uint8*)&v);
									r += (int)v[0];
									g += (int)v[1];
									b += (int)v[2];
								}
								else
								{
									r += (int)0;
									g += (int)0;
									b += (int)255;
								}
							}
						}

						r /= mxw * myw;
						g /= mxw * myw;
						b /= mxw * myw;
						
						*tmppic = (Uint8)r;
						tmppic++;
						*tmppic = (Uint8)g;
						tmppic++;
						*tmppic = (Uint8)b;
						tmppic++;
					}
				}
			}
			GLuint ogltexture;
			glGenTextures(1, &ogltexture);
			glBindTexture(GL_TEXTURE_2D, ogltexture);

			// use bilinear scaling for scaling down and trilinear scaling for scaling up
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate mipmap using GLU
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, resx, resy, GL_RGB, GL_UNSIGNED_BYTE, pic);
			Uint32 end = SDL_GetTicks();
			cout << "finished in " << end - start << " ms" << endl;
			return ogltexture;	
		}

		GUIWindow::GUIWindow()
		{
			this->x = 0.0f;
			this->y = 0.0f;
			float aspect = (float)windowWidth / (float)windowHeight;

			this->w = aspect;
			this->h = 1.0f;

			this->xUnit = aspect / (float)windowWidth;
			this->yUnit = 1.0f / (float)windowHeight;

			sleep = false;
			sleepms = 0;

			pMainPanel = NULL;
			returnValue = 0;
		}

		GUIWindow::~GUIWindow()
		{
		}

		void GUIWindow::SetPanel(Panel* pnl)
		{
			pMainPanel = pnl;

			if(pnl != NULL)
			{
				pMainPanel->SetCoordinateSystem(this->x, this->y, this->w, this->h, this->xUnit, this->yUnit);
				pMainPanel->SetGlobalCoordinateSystem(this->x, this->y, this->w, this->h);
			}
		}

		TranslatedMouse* GUIWindow::TranslateMouseCoords(Uint16 x, Uint16 y)
		{
			TranslatedMouse* coord = new TranslatedMouse;
			coord->x = (float)x * this->xUnit;
			coord->y = (float)y * this->yUnit;
			return coord;
		}

		bool GUIWindow::ProcessEvents()
		{
			//SDL Events
			SDL_Event *event = new SDL_Event;
			while (SDL_PollEvent(event))
			{
				if(pMainPanel != NULL)
				{
					switch(event->type)
					{
						case SDL_KEYDOWN:
						{
							if(event->key.keysym.sym == SDLK_ESCAPE)
								go = false;
							else
								pMainPanel->HandleEvent(KB_DOWN, event, NULL);
							break;
						}
						case SDL_KEYUP:
						{
							pMainPanel->HandleEvent(KB_UP, event, NULL);
							break;
						}
						case SDL_MOUSEMOTION:
						{
							//Translate position
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_MOVE, event, ptr = TranslateMouseCoords(event->motion.x, event->motion.y));
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONDOWN:
						{
							TranslatedMouse *ptr;							
							pMainPanel->HandleEvent(MOUSE_DOWN, event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							delete ptr;
							break;
						}
						case SDL_MOUSEBUTTONUP:
						{
							TranslatedMouse *ptr;
							pMainPanel->HandleEvent(MOUSE_UP,event, ptr = TranslateMouseCoords(event->button.x, event->button.y));
							delete ptr;
							break;
						}
					}
				}

				switch(event->type)
				{
					case SDL_QUIT:
					{
						go = false;
						break;	
					}
				}
			}

			delete event;
			return true;
		}

		bool GUIWindow::PaintAll()
		{
			if(pMainPanel != NULL)
			{
				pMainPanel->Paint();
			}
			return true;
		}

		int GUIWindow::RunLoop()
		{
			Window::GUI::last_frame = SDL_GetTicks();
			glClearColor( 0.2f, 0.2f, 0.2f, 0.7f );
			go = true;
			while(go)
			{
				// nollställ backbufferten och depthbufferten
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// nollställ vyn
				glLoadIdentity();
				Utilities::SwitchTo2DViewport(w, h);
				ProcessEvents();
				PerformPreFrame();

				PaintAll();
				Utilities::RevertViewport();
				SDL_GL_SwapBuffers();

				if(sleep)
				{
					SDL_Delay(sleepms);
				}

				Uint32 this_frame = SDL_GetTicks();

				Window::GUI::time_since_last_frame = (this_frame - last_frame) / 1000.0;
				Window::GUI::last_frame = this_frame;
			}
			return returnValue;
		}

		float PixelAlign(float value, float unit)
		{
			return floor((value / unit) + 0.5f) * unit;
		}

		Panel::Panel()
		{	
			x = 0.0f;
			y = 0.0f;
			w = 1.0f;
			h = 1.0f;

			parent = NULL;

			counter = 0;
			Focused.pPanel = NULL;
			FocusedType = typeEnd;
			lastMouseOver = NULL;
			lastMouseCoords.x = 0.0f;
			lastMouseCoords.y = 0.0f;
			bgColor[0] = 0.0f;
			bgColor[1] = 0.0f;
			bgColor[2] = 0.0f;
			bgColor[3] = 0.0f;
			focusOnClick = true;
			currentMouseDown = NULL;
			hasTexture = false;
		}

		Panel::~Panel()
		{
			Clear();
		}
		
		void Panel::SetCoordinateSystem(float x, float y, float w, float h, float xUnit, float yUnit)
		{
			this->x = PixelAlign(x, xUnit);
			this->y = PixelAlign(y, yUnit);
			this->w = PixelAlign(w, xUnit);
			this->h = PixelAlign(h, yUnit);
			this->xUnit = xUnit;
			this->yUnit = yUnit;
		}

		void Panel::SetGlobalCoordinateSystem(float gx, float gy, float gw, float gh)
		{
			this->gx = gx;
			this->gy = gy;
			this->gw = gw;
			this->gh = gh;
		}

		void Panel::FixCoordinateSystem(float x, float y, float sx, float sy)
		{
			glPushMatrix();
			glTranslatef(x, y, 0);
			glScalef(sx, sy, 0.0f);
		}

		void Panel::RevertCoordinateSystem()
		{
			glPopMatrix();
		}
	
		float Panel::GetWidth()
		{
			return this->w;
		}

		float Panel::GetHeight()
		{
			return this->h;
		}

		void Panel::ReformMouseCoords(TranslatedMouse* coords)
		{
			coords->x -= this->x; 
			coords->y -= this->y; 
			coords->x /= this->w / (this->w / this->h);
			coords->y /= this->h;
		}

		int Panel::Add(Widget* pWidget)
		{
			counter++;
			ObjectInfo *info = new ObjectInfo;
			info->type = typeWidget;
			info->x = 0.0f;
			info->y = 0.0f;
			info->w = 1.0f;
			info->h = 1.0f;
			info->target_aspect = 1.0f;
			info->sx = 1.0f;
			info->sy = 1.0f;
			info->visible = true;
			info->object.pWidget = pWidget;
			info->object.pWidget->parent = this;
			info->object.pWidget->id = counter;

			ObjectInformation.push_back(info);
			ObjectMap[counter] = info;
			return counter;
		}

		int Panel::Add(Panel* pPnl)
		{
			counter++;
			ObjectInfo *info = new ObjectInfo;
			info->type = typePanel;
			info->x = 0.0f;
			info->y = 0.0f;
			info->w = 1.0f;
			info->h = 1.0f;
			info->target_aspect = 1.0f;
			info->sx = 1.0f;
			info->sy = 1.0f;
			info->visible = true;
			info->object.pPanel = pPnl;
			info->object.pPanel->parent = this;
			info->object.pPanel->id = counter;

			ObjectInformation.push_back(info);
			ObjectMap[counter] = info;
			return counter;			
		}

		void Panel::SetTexture(GLuint texture)
		{
			this->texture = texture;
			this->hasTexture = true;
		}

		void Panel::SetVisible(int id, bool value)
		{
			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(id);
			if(iter != ObjectMap.end())
			{
				ObjectInfo *info = (*iter).second;
				info->visible = value;
			}
		}
	
		int Panel::Delete(int panel_widget)
		{
			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(panel_widget);
			if(iter != ObjectMap.end())
			{		
				ObjectInfo *info = (*iter).second;
				for(vector<ObjectInfo*>::iterator Object = this->ObjectInformation.begin(); Object != this->ObjectInformation.end(); Object++)
				{	
					if((*Object) == info)
					{
						ObjectInformation.erase(Object);
						break;
					}
				}
				ObjectMap.erase(iter);
				return SUCCESS;
			}

			return ERROR_GENERAL;
		}

		int Panel::Clear()
		{
			ObjectInformation.clear();
			ObjectMap.clear();
			return SUCCESS;
		}

		void Panel::SetConstraintPercent(int panel_widget, float x, float y, float w, float h)
		{
			SetConstraint(panel_widget, x * this->w, y * this->h, w * this->w, h * this->h);
		}

		void Panel::SetConstraint(int panel_widget, float x, float y, float w, float h)
		{
			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(panel_widget);
			if(iter != ObjectMap.end())
			{
				ObjectInfo *info = (*iter).second;
				info->x = PixelAlign(x, this->xUnit);
				info->y = PixelAlign(y, this->yUnit);
				info->w = PixelAlign(x + w, this->xUnit) - info->x;
				info->h = PixelAlign(y + h, this->yUnit) - info->y;

				//Calculate aspects
				info->target_aspect = info->w / info->h;
				float source_aspect = this->w / this->h;
				
				//Pre calculate scale factors
				info->sx = (info->w / this->w) / (info->target_aspect / source_aspect);
				info->sy = info->h / this->h;

				if(info->type == typePanel)
				{
					info->object.pPanel->SetCoordinateSystem(x, y, info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					info->object.pPanel->SetGlobalCoordinateSystem(this->gx + (x / this->w) * this->gw, this->gy + y * this->gh, (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
				else
				{
					info->object.pWidget->SetCoordinateSystem(info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					info->object.pWidget->SetGlobalCoordinateSystem(this->gx + (x / this->w) * this->gw, this->gy + y * this->gh, (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
			}
		}

		TranslatedMouse Panel::GetInverseCoordinates(int id, float relX, float relY)
		{
			TranslatedMouse location;
			location.x = 0.0f;
			location.y = 0.0f;

			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(id);
			if(iter != ObjectMap.end())
			{
				ObjectInfo *info = (*iter).second;
				location.x = relX / info->sx + info->x;
				location.y = relY / info->sy + info->y;
			}
			return location;
		}

		void Panel::CopyCoordianteSystem(int panel_widget, PanelWidget obj, GUIType typ)
		{
			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(panel_widget);
			if(iter != ObjectMap.end())
			{
				ObjectInfo *info = (*iter).second;

				if(info->type == typePanel && typ == typePanel)
				{
					obj.pPanel->SetCoordinateSystem(0.0f, 0.0f, info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					obj.pPanel->SetGlobalCoordinateSystem(this->gx + (info->x / this->w) * this->gw, this->gy + (info->y * this->gh), (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
				else if(info->type == typeWidget && typ == typeWidget)
				{
					obj.pWidget->SetCoordinateSystem(info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					obj.pWidget->SetGlobalCoordinateSystem(this->gx + (info->x / this->w) * this->gw, this->gy + info->y * this->gh, (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
				else if(info->type == typeWidget && typ == typePanel)
				{
					obj.pPanel->SetCoordinateSystem(0.0f, 0.0f, info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					obj.pPanel->SetGlobalCoordinateSystem(this->gx + (info->x / this->w) * this->gw, this->gy + (info->y * this->gh), (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
				else if(info->type == typePanel && typ == typeWidget)
				{
					obj.pWidget->SetCoordinateSystem(info->target_aspect, 1.0f, xUnit / info->h, yUnit / info->h);
					obj.pWidget->SetGlobalCoordinateSystem(this->gx + (info->x / this->w) * this->gw, this->gy + info->y * this->gh, (info->w / this->w) * this->gw, (info->h / this->h) * this->gh);
				}
			}
		}

		void Panel::SetElement(int panel_widget, PanelWidget obj, GUIType type)
		{
			map<int, ObjectInfo*>::iterator iter = ObjectMap.find(panel_widget);
			if(iter != ObjectMap.end())
			{
				ObjectInfo *info = (*iter).second;
				info->type = type;
				info->object = obj;
			}			
		}
		
		void Panel::SetFocusEnabled(bool value)
		{
			focusOnClick = value;
		}

		void Panel::MouseOutCheck(ObjectInfo *obj, TranslatedMouse* MouseCoords)
		{	
			if(lastMouseOver != obj)
			{
				if(lastMouseOver != NULL)
					MouseOutCall();
				lastMouseOver = obj;
			}

			lastMouseCoords.x = MouseCoords->x;
			lastMouseCoords.y = MouseCoords->y;
		}

		void Panel::MouseOutCall()
		{
			if(lastMouseOver->type == typePanel)
			{
				lastMouseOver->object.pPanel->HandleEvent(MOUSE_OUT, NULL, &lastMouseCoords);
			}
			else
			{
				lastMouseOver->object.pWidget->HandleEvent(MOUSE_OUT, NULL, &lastMouseCoords);	
			}
		}
		
		int Panel::HandleEvent(EventType evtType,SDL_Event* sdlEvent,TranslatedMouse* MouseCoords)
		{
			switch(evtType)
			{
				case MOUSE_DOWN:
				{
					//Find which widget/panel and set focus at this one.
					ObjectInfo *obj;
					if(GetWidgetPanelInPoint(MouseCoords, obj) == true)
					{
						MouseOutCheck(obj, MouseCoords);
						ReformWidgetMouseCoords(MouseCoords, obj);

						if(focusOnClick)
						{
							if(Focused.pPanel != obj->object.pPanel)
							{
								if(Focused.pPanel)
								{
									if(FocusedType == typePanel)
									{
										Focused.pPanel->HandleEvent(FOCUS_OUT, NULL, NULL);
									}
									else
									{
										Focused.pWidget->HandleEvent(FOCUS_OUT, NULL, NULL);
									}
								}
							}

							FocusedType = obj->type;
							Focused = obj->object;

							if(FocusedType == typeWidget)
							{
								obj->object.pWidget->HandleEvent(FOCUS, NULL, MouseCoords);
							}
						}

						this->currentMouseDown = obj;						
						if(obj->type == typePanel)
						{
							obj->object.pPanel->HandleEvent(evtType, sdlEvent, MouseCoords);
						}
						else
						{
							obj->object.pWidget->HandleEvent(evtType, sdlEvent, MouseCoords);	
						}
					}
					else
					{
						currentMouseDown = NULL;
					}
					break;
				}
				case MOUSE_UP:
				{
					if(this->currentMouseDown != NULL)
					{
						ObjectInfo *obj = this->currentMouseDown;
						if(obj->type == typePanel)
						{
							ReformWidgetMouseCoords(MouseCoords, obj);
							obj->object.pPanel->HandleEvent(evtType, sdlEvent, MouseCoords);
						}
						else
						{
							ReformWidgetMouseCoords(MouseCoords, obj);
							obj->object.pWidget->HandleEvent(evtType, sdlEvent, MouseCoords);
						}
						currentMouseDown = NULL;
					}
					break;
				}
				case MOUSE_SCROLL:
				case MOUSE_PRESS:
				case MOUSE_MOVE:
				{

					ObjectInfo *obj = currentMouseDown;
					if(obj != NULL)
					{
						if(obj->type == typePanel)
						{
							ReformWidgetMouseCoords(MouseCoords, obj);
							obj->object.pPanel->HandleEvent(evtType, sdlEvent, MouseCoords);
						}
						else
						{
							ReformWidgetMouseCoords(MouseCoords, obj);
							obj->object.pWidget->HandleEvent(evtType, sdlEvent, MouseCoords);
						}						
					}
					else
					{
#ifdef DEBUG_MOUSE_COORDS
						cout << "MX: " << MouseCoords->x << " MY: " << MouseCoords->y << endl;
#endif
						//Find which widget/panel and send to this one.
						if(GetWidgetPanelInPoint(MouseCoords, obj) == true)
						{
							MouseOutCheck(obj, MouseCoords);
							if(obj->type == typePanel)
							{
								ReformWidgetMouseCoords(MouseCoords, obj);
								obj->object.pPanel->HandleEvent(evtType, sdlEvent, MouseCoords);
							}
							else
							{
								ReformWidgetMouseCoords(MouseCoords, obj);
								obj->object.pWidget->HandleEvent(evtType, sdlEvent, MouseCoords);
							}
						}
						else
						{
							MouseOutCheck(NULL, MouseCoords);
						}
					}
					break;
				}
				case MOUSE_OUT:
				{
					if(lastMouseOver != NULL)
					{
						MouseOutCall();
						lastMouseOver = NULL;
					}
					break;
				}
				case KB_DOWN:
				case KB_UP:
				{
					//Send to focused or to game engine.
					if(FocusedType != typeEnd)
					{
						if(FocusedType == typePanel)
						{
							Focused.pPanel->HandleEvent(evtType, sdlEvent, NULL);
						}
						else
						{
							Focused.pWidget->HandleEvent(evtType, sdlEvent, NULL);
						}
					}
					break;
				}
				case FOCUS_OUT:
				{
					if(Focused.pPanel)
					{
						if(FocusedType == typePanel)
						{
							Focused.pPanel->HandleEvent(FOCUS_OUT, NULL, NULL);
						}
						else
						{
							Focused.pWidget->HandleEvent(FOCUS_OUT, NULL, NULL);
						}

						Focused.pPanel = NULL;
						FocusedType = typeEnd;
					}
					break;
				}
				default:
				{
					break;
				}
			}

			return SUCCESS;
		}

		void Panel::ReformWidgetMouseCoords(TranslatedMouse* coords, ObjectInfo *info)
		{
			coords->x -= info->x;
			coords->y -= info->y;
			coords->x /= info->w / (info->w / info->h);
			coords->y /= info->h;
		}

		bool Panel::GetWidgetPanelInPoint(TranslatedMouse* Coord, ObjectInfo*& obj)
		{
			//Iterate through all widgets / panels
			for(vector<ObjectInfo*>::iterator Object = this->ObjectInformation.begin(); Object != this->ObjectInformation.end(); Object++)
			{
				float widgetW = (*Object)->w;
				float widgetH = (*Object)->h;
				if((*Object)->visible == true)
				{
					if(Coord->x >= (*Object)->x &&  Coord->x <= ((*Object)->x + widgetW ))
					{
						if(Coord->y >= (*Object)->y && Coord->y <= ((*Object)->y + widgetH))
						{
							obj = (*Object);
							return true;
						}
					}
				}
			}
			return false;
		}

		void Panel::SetFocus(int id)
		{
			ObjectInfo *info = ObjectMap[id];

			if(Focused.pPanel)
			{
				if(FocusedType == typePanel)
				{
					Focused.pPanel->HandleEvent(FOCUS_OUT, NULL, NULL);
				}
				else
				{
					Focused.pWidget->HandleEvent(FOCUS_OUT, NULL, NULL);
				}

				Focused.pPanel = NULL;
				FocusedType = typeEnd;
			}

			this->Focused = info->object;
			this->FocusedType = info->type;

			if(FocusedType == typeWidget)
			{
				this->Focused.pWidget->HandleEvent(FOCUS, NULL, NULL);
			}
		}

		void Panel::PaintBackground()
		{
#ifdef DEBUG_PANEL
			glColor4f(0.5,0.0f,0.0f, 0.5f);
			glBegin(GL_QUADS);
			glVertex2f(0.0f, 0.0f);
			glVertex2f(this->w, 0.0f);
			glVertex2f(this->w, this->h);
			glVertex2f(0.0f, this->h);
			glEnd();
#endif
			if(hasTexture == true)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, this->texture);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0.0f, 0.0f);

					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(this->w, 0.0f);

					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(this->w, this->h);

					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0.0f, this->h);
				glEnd();
				glDisable(GL_TEXTURE_2D);
			}
			else
			{
				if(bgColor[3] != 0.0f)
				{
					glColor4f(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
					glBegin(GL_QUADS);
						glVertex2f(0.0f, 0.0f);
						glVertex2f(this->w, 0.0f);
						glVertex2f(this->w, this->h);
						glVertex2f(0.0f, this->h);
					glEnd();
				}
			}
		}

		void Panel::Paint()
		{
			PaintBackground();

			for(vector<ObjectInfo*>::reverse_iterator Object = this->ObjectInformation.rbegin(); Object != this->ObjectInformation.rend(); Object++)
			{
				if((*Object)->visible == true)
				{
					FixCoordinateSystem((*Object)->x, (*Object)->y, (*Object)->sx, (*Object)->sy);
					if((*Object)->type == typePanel)
						(*Object)->object.pPanel->Paint();
					else
						(*Object)->object.pWidget->Paint();
					RevertCoordinateSystem();
				}
			}

			PaintTop();
		}

		void Panel::PaintTooltip()
		{
			for(vector<ObjectInfo*>::reverse_iterator Object = this->ObjectInformation.rbegin(); Object != this->ObjectInformation.rend(); Object++)
			{
				FixCoordinateSystem((*Object)->x, (*Object)->y, (*Object)->sx, (*Object)->sy);
				if((*Object)->type == typePanel)
					(*Object)->object.pPanel->PaintTooltip();
				else
					(*Object)->object.pWidget->PaintTooltip();
				RevertCoordinateSystem();
			}
		}

		Widget::Widget()
		{
			parent = NULL;

			w = 1.0f;
			h = 1.0f;
			gx = 0.0f;
			gy = 0.0f;
			gw = 1.0f;
			gh = 1.0f;
			xUnit = 1.0f;
			yUnit = 1.0f;

		}
		
		void Widget::SetCoordinateSystem(float w, float h, float xUnit, float yUnit)
		{
			this->w = w;
			this->h = h;
			this->xUnit = xUnit;
			this->yUnit = yUnit;
		}

		void Widget::SetGlobalCoordinateSystem(float gx, float gy, float gw, float gh)
		{
			this->gx = gx;
			this->gy = gy;
			this->gw = gw;
			this->gh = gh;
		}

		void Widget::ConvertToGlobalCoordinateSystem(float& x, float& y, float& w, float& h)
		{
			x = this->gx + x / this->xUnit * Window::guiResolution;
			y = this->gy + y / this->xUnit * Window::guiResolution;
			w = w / this->xUnit * Window::guiResolution;
			h = h / this->xUnit * Window::guiResolution;
		}

		int Widget::Paint()
		{
			glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
			glBegin(GL_QUADS);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(w, 0.0f);
				glVertex2f(w, h);
				glVertex2f(0.0f, h);
			glEnd();

			return SUCCESS;
		}

		int Widget::HandleEvent(EventType evtType, SDL_Event *evt, TranslatedMouse* MouseCoord)
		{
			//Send to game core
#ifdef DEBUG_MOUSE_COORDS
			glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
			glBegin(GL_QUADS);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(0.1f, 0.0f);
				glVertex2f(0.1f, 0.1f);
				glVertex2f(0.0f, 0.1f);
			glEnd();

			if(MouseCoord != NULL)
			{
				cout << "WMX: " << MouseCoord->x << " WMY: " << MouseCoord->y << endl;
			}
#endif
			return SUCCESS;
		}

		Widget::~Widget()
		{
			
		}
		
		Label::Label()
		{
			this->text = "";
			this->type = 2;
			this->painttooltip = false;
		}

		void Label::SetText(string text)
		{
			this->text = text;
		}

		void Label::PaintLabel()
		{
			glEnable(GL_TEXTURE_2D);

			Window::GUI::FontCache::RenderedText RenderedInfo;
			Window::GUI::Fonts.SetFontType(this->type);
			Window::GUI::Fonts.RenderText(this->text , RenderedInfo, xUnit);
			float xPos = floor((w / 2.0f - RenderedInfo.w / 2.0f) / xUnit + 0.5f) * xUnit;
			float yPos = floor((0.5f - RenderedInfo.h / 2.0f) / yUnit + 0.5f) * yUnit;
			
			PaintLabel(RenderedInfo, xPos, yPos);
		}

		void Label::PaintLabel(Window::GUI::FontCache::RenderedText RenderedInfo, float tx, float ty)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, RenderedInfo.Texture);
			glPushMatrix();
			glTranslatef(tx, ty, 0.0f);
			glBegin(GL_QUADS);
				glColor4f(1.0f,1.0f,1.0f,1.0f);
				glTexCoord2f(RenderedInfo.texCoords[0], RenderedInfo.texCoords[1]);
				glVertex2f(0.0f,0.0f);

				glTexCoord2f(RenderedInfo.texCoords[2], RenderedInfo.texCoords[3]);
				glVertex2f(RenderedInfo.w, 0.0f);

				glTexCoord2f(RenderedInfo.texCoords[4], RenderedInfo.texCoords[5]);
				glVertex2f(RenderedInfo.w, RenderedInfo.h);

				glTexCoord2f(RenderedInfo.texCoords[6], RenderedInfo.texCoords[7]);
				glVertex2f(0.0f, RenderedInfo.h);
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
		}

		int Label::Paint()
		{
			PaintLabel();
			return SUCCESS;
		}
		
		int Label::HandleEvent(EventType evtType,SDL_Event* event,TranslatedMouse* MouseCoords)
		{
			switch(evtType)
			{
				case MOUSE_OUT:
				{
					painttooltip = false;
					break;
				}
				case MOUSE_MOVE:
				{
					painttooltip = true;
					mx = MouseCoords->x;
					my = MouseCoords->y;
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		TextBox::TextBox()
		{
			maxSize = 256;
			marker = 0;
			markerbyte = 0;
			translateX = 0.0f;
			markerX = 0.0f;
			text = "";
			drawMarker = false;
			SDL_EnableUNICODE(1);
			SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		}

		int TextBox::Paint()
		{
			glBegin(GL_QUADS);
				glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(this->w, 0.0f);
				glVertex2f(this->w, this->h);
				glVertex2f(0.0f, this->h);
			glEnd();

			if(text.length() > 0)
			{
				if(textSize.w > this->w)
				{
					float notVisible = textSize.w - this->w;

					FontCache::RenderedText rendered;
					Fonts.RenderText(text, rendered, xUnit);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, rendered.Texture);
					glPushMatrix();

					float min = (translateX / textSize.w) * rendered.texCoords[2];
					float max = ((textSize.w + translateX - notVisible) / textSize.w) * rendered.texCoords[2];

					glTranslatef(0.0f, 0.0f, 0.0f);
					glBegin(GL_QUADS);
					glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(min, rendered.texCoords[1]);
						glVertex2f(0.0f,0.0f);

						glTexCoord2f(max, rendered.texCoords[3]);
						glVertex2f(this->w, 0.0f);

						glTexCoord2f(max, rendered.texCoords[5]);
						glVertex2f(this->w, rendered.h);

						glTexCoord2f(min, rendered.texCoords[7]);
						glVertex2f(0.0f, rendered.h);
					glEnd();

					glPopMatrix();
					glDisable(GL_TEXTURE_2D);

					if(drawMarker)
					{
						glBegin(GL_QUADS);
							glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
							glVertex2f(markerX - translateX, 0.0f);
							glVertex2f(markerX  - translateX + xUnit, 0.0f);
							glVertex2f(markerX  - translateX + xUnit, this->h);
							glVertex2f(markerX  - translateX, this->h);
						glEnd();
					}
				}
				else
				{
					FontCache::RenderedText rendered;
					Fonts.RenderText(text, rendered, xUnit);				
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, rendered.Texture);
					glPushMatrix();

					glTranslatef(0.0f, 0.0f, 0.0f);
					glBegin(GL_QUADS);
						glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(rendered.texCoords[0], rendered.texCoords[1]);
						glVertex2f(0.0f,0.0f);

						glTexCoord2f(rendered.texCoords[2], rendered.texCoords[3]);
						glVertex2f(rendered.w, 0.0f);

						glTexCoord2f(rendered.texCoords[4], rendered.texCoords[5]);
						glVertex2f(rendered.w, rendered.h);

						glTexCoord2f(rendered.texCoords[6], rendered.texCoords[7]);
						glVertex2f(0.0f, rendered.h);
					glEnd();

					glPopMatrix();
					glDisable(GL_TEXTURE_2D);
					if(drawMarker)
					{
						glBegin(GL_QUADS);
							glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
							glVertex2f(markerX, 0.0f);
							glVertex2f(markerX + xUnit, 0.0f);
							glVertex2f(markerX + xUnit, this->h);
							glVertex2f(markerX, this->h);
						glEnd();
					}
				}
			}
			else
			{
				if(drawMarker)
				{
					glBegin(GL_QUADS);
						glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
						glVertex2f(markerX, 0.0f);
						glVertex2f(markerX + xUnit, 0.0f);
						glVertex2f(markerX + xUnit, this->h);
						glVertex2f(markerX, this->h);
					glEnd();
				}
			}
			return SUCCESS;
		}

		int UTF8_GetByteCount(Uint16 value)
		{
			if(value > 2047)
			{
				return 3;
			}
			else if(value > 127)
			{
				return 2;
			}
			else
			{
				return 1;
			}
		}

		//This function isn't made from the real standard, but it is enough for our purposes. supports 16 bit conversion to UTF8
		void UTF8_Encode(Uint16 value, Uint8 *output)
		{
			if(value > 2047)
			{
				//00000000 xxxxyyyy yyzzzzzz 	1110xxxx(E0-EF) 10yyyyyy 10zzzzzz
				//3byte
				output[0] = 0xE0 + ((value & 0xF000) >> 12);
				output[1] = 0x80 + ((value & 0x0FC0) >> 6);
				output[2] = 0x80 + (value & 0x003F);
			}
			else if(value > 127)
			{
				//00000000 00000yyy yyzzzzzz 	110yyyyy(C2-DF) 10zzzzzz(80-BF)
				//2byte
				output[0] = 0xC0 + ((value & 0x07C0) >> 6);
				output[1] = 0x80 + (value & 0x003F);
			}
			else
			{
				//byte
				output[0] = (Uint8)value;
			}
		}

		void TextBox::Backspace()
		{
			Uint16 val = textBlocks.at(marker - 1);
			int bytes = UTF8_GetByteCount(val);
			text.erase(markerbyte - bytes, bytes);
			textBlocks.erase(textBlocks.begin() + (marker - 1));
			markerbyte -= bytes;
			marker--;
		}

		void TextBox::Delete()
		{
			Uint16 val = textBlocks.at(marker);
			int bytes = UTF8_GetByteCount(val);
			if(marker == 0)
			{
				text.erase(0, bytes);
			}
			else
			{
				text.erase((markerbyte + UTF8_GetByteCount(textBlocks.at(marker - 1))) - bytes, bytes);
			}
			textBlocks.erase(textBlocks.begin() + marker);
		}

		void TextBox::Left()
		{
			Uint16 val = textBlocks.at(marker - 1);
			int bytes = UTF8_GetByteCount(val);
			markerbyte -= bytes;
			marker--;
			
		}

		void TextBox::Right()
		{
			Uint16 val = textBlocks.at(marker);
			int bytes = UTF8_GetByteCount(val);
			markerbyte += bytes;
			marker++;
		}

		void TextBox::UpdateGraphicalMarker()
		{
			FontCache::TextDimension dim = Fonts.GetTextSize(text.substr(0, markerbyte), xUnit);
			markerX = dim.w;
			if(dim.w > this->w)
			{
				translateX = dim.w - this->w;
			}
			else
			{
				translateX = 0.0f;
			}
			textSize = Fonts.GetTextSize(text, xUnit);
		}

		int TextBox::HandleEvent(EventType evtType, SDL_Event* evt, TranslatedMouse* mouseCoord)
		{
			switch(evtType)
			{
				case FOCUS:
				{
					drawMarker = true;
					break;
				}
				case FOCUS_OUT:
				{
					drawMarker = false;
					break;
				}
				case MOUSE_UP:
				{
					break;
				}
				case KB_DOWN:
				{
					switch(evt->key.keysym.sym)
					{
						case SDLK_HOME:
						{
							marker = 0;
							markerbyte = 0;
							break;
						}
						case SDLK_END:
						{
							marker = textBlocks.size();
							markerbyte = text.length();
							break;
						}
						case SDLK_BACKSPACE:
						{
							if(marker > 0)
							{
								if(evt->key.keysym.mod & KMOD_CTRL)
								{
									while(marker > 0)
									{
										Uint16 val = textBlocks.at(marker - 1);
										Backspace();
										if(val == 32)
											break;
									}
								}
								else
								{
									Backspace();
								}
							}
							break;
						}
						case SDLK_DELETE:
						{
							if(marker < textBlocks.size())
							{
								if(evt->key.keysym.mod & KMOD_CTRL)
								{
									while(marker < textBlocks.size())
									{
										Uint16 val = textBlocks.at(marker);
										Delete();
										if(val == 32)
											break;
									}
								}
								else
								{
									Delete();
								}
							}
							break;
						}
						case SDLK_LEFT:
						{
							if(marker > 0)
							{
								if(evt->key.keysym.mod & KMOD_CTRL)
								{
									Uint16 val = textBlocks.at(marker - 1);
									Left();
									if(val == 32)
										break;

									while(marker > 0)
									{
										Uint16 val = textBlocks.at(marker - 1);
										if(val == 32)
											break;
										Left();
									}
								}
								else
								{
									Left();
								}
							}
							break;
						}
						case SDLK_RIGHT:
						{
							if(marker < textBlocks.size())
							{
								if(evt->key.keysym.mod & KMOD_CTRL)
								{
									Uint16 val = textBlocks.at(marker);
									Right();
									if(val == 32)
										break;

									while(marker < textBlocks.size())
									{
										Uint16 val = textBlocks.at(marker);
										if(val == 32)
											break;
										Right();

									}
								}
								else
								{
									Right();
								}
							}
							break;
						}
						default:
						{
							Uint16 chr = evt->key.keysym.unicode;
							if(chr > 31)
							{
								int bytes = UTF8_GetByteCount(chr);
								if(text.length() + bytes > this->maxSize)
								{
									break;
								}

								if(bytes == 1)
								{
									text.insert(text.begin() + markerbyte, 1, (Uint8)chr);
								}
								else
								{
									if(bytes == 2)
									{
										Uint8 buffer[2];
										UTF8_Encode(chr, (Uint8*)&buffer);
										text.insert(text.begin() + markerbyte, 1, buffer[1]);
										text.insert(text.begin() + markerbyte, 1, buffer[0]);
									}
									else
									{
										Uint8 buffer[3] = {0, 0, 0};
										UTF8_Encode(chr, (Uint8*)&buffer);
										text.insert(text.begin() + markerbyte, 1, buffer[2]);
										text.insert(text.begin() + markerbyte, 1, buffer[1]);
										text.insert(text.begin() + markerbyte, 1, buffer[0]);
									}
								}
								textBlocks.insert(textBlocks.begin() + marker, 1, chr);
								markerbyte += bytes;
								marker++;
								UpdateGraphicalMarker();
							}
						}
					}
					UpdateGraphicalMarker();
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		void TextBox::SetMaxLen(int len)
		{
			maxSize = len;
		}

		void TextBox::SetText(string text)
		{
			textBlocks.clear();
			for(unsigned i = 0; i < text.length(); i++)
			{
				textBlocks.push_back(text.at(i));
			}
			this->text = text;
			marker = textBlocks.size();
			markerbyte = text.length();
			UpdateGraphicalMarker();
		}

		string TextBox::GetText()
		{
			return text;
		}

		ConsoleBuffer::ConsoleBuffer()
		{
			scrollSize = 0.05f;
			scrollWidth = 0.025f;
			mouseDown = false;
			mouseCorrect = false;
		}

		void ConsoleBuffer::PrepareBuffer()
		{
			Fonts.SetFontType(this->type);
			float lineHeight = Fonts.GetLineHeight(this->yUnit);
			console.SetLineHeight(lineHeight, (int)(this->h / lineHeight));
		}

		int ConsoleBuffer::Paint()
		{
			glBegin(GL_QUADS);
				glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
				glVertex2f(0.0f, 0.0f);
				glVertex2f(this->w, 0.0f);
				glVertex2f(this->w, this->h);
				glVertex2f(0.0f, this->h);
			glEnd();

			float y = 0.0f;

			const string* lines = console.GetBuffer();
			if (lines == NULL) return SUCCESS;

			int location = console.start + console.translate;
			if(location >= console.lineCount)
				location -= console.lineCount;

			int c = 0;

			while(c < console.visibleLines)
			{
				if(lines[location] != "" && lines[location].size() != 0)
				{
					Window::GUI::FontCache::RenderedText RenderedInfo;
					Window::GUI::Fonts.SetFontType(this->type);
					string output = lines[location];
					bool revert = false;
					char first = output.at(0);
					char* print;
					if (first == Console::err || first == Console::imp)
					{
						if (first == Console::err)
							Fonts.SetColor(CONSOLE_COLOR_ERROR);
						else
							Fonts.SetColor(CONSOLE_COLOR_IMPORTANT);
						revert = true;
						print = (char*)output.c_str();
						print++;
					}
					else
						print = (char*)output.c_str();

					Window::GUI::Fonts.RenderText(print, RenderedInfo, xUnit);
					float xPos = 0.0f;
					float yPos = y;
					PaintLabel(RenderedInfo, xPos, yPos);

					if (revert)
						Fonts.SetColor(255, 255, 255);
				}

				y += floor(console.lineHeight / yUnit + 0.5f) * yUnit;

				c++;
				location++;
				if(location == console.lineCount)
					location = 0;
			}

			glBegin(GL_QUADS);
				glColor4f(0.8f, 0.8f, 1.0f, 0.5f);
				glVertex2f(this->w - scrollWidth, 0.0f);
				glVertex2f(this->w, 0.0f);
				glVertex2f(this->w, this->h);
				glVertex2f(this->w - scrollWidth, this->h);
			glEnd();
			
			float pos = (float)console.translate / (float)(console.lineCount - console.visibleLines);
			
			pos = (1.0f - scrollSize) * pos;
			pos += (scrollSize / 2);

			glBegin(GL_QUADS);
				glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
				glVertex2f(this->w - scrollWidth, pos - (scrollSize / 2));
				glVertex2f(this->w, pos - (scrollSize / 2));
				glVertex2f(this->w, pos + (scrollSize / 2));
				glVertex2f(this->w - scrollWidth, pos + (scrollSize / 2));
			glEnd();

			return SUCCESS;
		}

		int ConsoleBuffer::HandleEvent(Window::GUI::EventType evtType, SDL_Event* evt, Window::GUI::TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_DOWN:
				{
					if(MouseCoord->x > this->w - scrollWidth)
					{
						mouseDown = true;
					}
					break;
				}
				case MOUSE_MOVE:
				{
					if(mouseDown == true)
					{
						if(MouseCoord->y < (scrollSize / 2)) //also protects from divide by zero.
						{
							console.translate = 0;
						}
						else if(MouseCoord->y > this->h - (scrollSize / 2))
						{
							console.translate = console.lineCount - console.visibleLines;
						}
						else
						{
							float y = MouseCoord->y - (scrollSize / 2);
							y /= (this->h - scrollSize);
							console.translate = (int)((console.lineCount - console.visibleLines) * y);
						}
					}
					break;
				}
				case MOUSE_UP:
				{
					mouseDown = false;
					break;
				}
				case MOUSE_SCROLL:
				default:
					break;
			}
			return SUCCESS;
		}

		void Tooltip::PaintTooltip()
		{
			if(painttooltip == true)
			{
				if(tooltip.size() != 0)
				{
					glEnable(GL_TEXTURE_2D);
	
					Window::GUI::FontCache::RenderedText RenderedInfo;
					Window::GUI::Fonts.SetFontType(this->tooltiptype);
					Window::GUI::Fonts.RenderText(this->tooltip , RenderedInfo, xUnit);

					float x = mx - RenderedInfo.w / 2;
					float y = my - RenderedInfo.h;
					float w = RenderedInfo.w;
					float h = RenderedInfo.h;

					ConvertToGlobalCoordinateSystem(x,y,w,h);

					float xPos = mx - RenderedInfo.w / 2;
					float yPos = my - RenderedInfo.h;

					if(x < 0)
						xPos = 0.0f;
					else if(x + (((RenderedInfo.w) / this->xUnit * Window::guiResolution)) > Window::guiWidth)
						xPos = this->w - RenderedInfo.w;
					
					if(y < 0)
						yPos = 0.0f;
					else if(y > Window::guiHeight)
						yPos = Window::guiHeight - RenderedInfo.h;

					xPos = PixelAlign(xPos, xUnit);
					yPos = PixelAlign(yPos, xUnit);

					glTranslatef(xPos, yPos, 0.0f);

					glDisable(GL_TEXTURE_2D);
					glBegin(GL_QUADS);
						glColor4f(0.0f,0.0f,0.0f, 0.7f);
						glVertex2f(0.0f, 0.0f);
						glVertex2f(RenderedInfo.w, 0.0f);
						glVertex2f(RenderedInfo.w, RenderedInfo.h);
						glVertex2f(0.0f, RenderedInfo.h);
					glEnd();
					glEnable(GL_TEXTURE_2D);

					glBindTexture(GL_TEXTURE_2D, RenderedInfo.Texture);
					glBegin(GL_QUADS);
						glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(RenderedInfo.texCoords[0], RenderedInfo.texCoords[1]);
						glVertex2f(0.0f,0.0f);
	
						glTexCoord2f(RenderedInfo.texCoords[2], RenderedInfo.texCoords[3]);
						glVertex2f(RenderedInfo.w, 0.0f);
					
						glTexCoord2f(RenderedInfo.texCoords[4], RenderedInfo.texCoords[5]);
						glVertex2f(RenderedInfo.w, RenderedInfo.h);
	
						glTexCoord2f(RenderedInfo.texCoords[6], RenderedInfo.texCoords[7]);
						glVertex2f(0.0f, RenderedInfo.h);
					glEnd();
					glDisable(GL_TEXTURE_2D);

				}
			}
		}
	
		void Tooltip::SetTooltip(string tooltip)
		{
			this->tooltip = tooltip;
		}

		InfoLabel::InfoLabel()
		{
			this->type = 0;
		}

		int InfoLabel::Paint()
		{
			if(datasource)
			{
				text = (*datasource)();
			}
			PaintLabel();
			return SUCCESS;
		}

		void InfoLabel::AttachHandler(InfoLabelfptr fptr)
		{
			datasource = fptr;
		}
		
		Button::Button()
		{
			onEvent = NULL; 
			tag = NULL;
			mouseDown = false;
			fadeValue = 0.0f;
			painttooltip = false;
			mouseOver = false;
		}

		int Button::HandleEvent(EventType evtType, SDL_Event *evt, TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_UP:
				{
					if(mouseDown == true)
					{
						if(MouseCoord->x > 0 && MouseCoord->y > 0 && MouseCoord->x <= this->w && MouseCoord->y <= this->h)
						{
							if(onEvent)
							{
								if(evtType == MOUSE_UP)
								{
									(*onEvent)(evtType, tag);
								}
							}
						}
						mouseDown = false;
					}
					break;
				}
				case MOUSE_DOWN:
				{
					mouseDown = true;
					fadeValue = 0.7f;
					break;
				}
				case MOUSE_MOVE:
				{
					painttooltip = true;
					mouseOver = true;
					mx = MouseCoord->x;
					my = MouseCoord->y;
					break;
				}
				case MOUSE_OUT:
				{
					mouseOver = false;
					mouseDown = false;
					painttooltip = false;
					break;
				}
				default:
					break;
			}

			return SUCCESS;
		}

		void Button::ResetFade()
		{
			this->fadeValue = 0.0f;
		}

		void Button::SetTag(void* ptr)
		{
			this->tag = ptr;
		}
	
		int PicTextButton::HandleEvent(Window::GUI::EventType evtType, SDL_Event *evt, Window::GUI::TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_UP:
				{
					if(mouseDown == true)
					{
						if(MouseCoord->x > 0 && MouseCoord->y > 0 && MouseCoord->x <= this->w && MouseCoord->y <= this->h)
						{
							if(onEvent)
							{
								if(evtType == MOUSE_UP)
								{
									(*onEvent)(evtType, tag);
								}
							}
						}
						mouseDown = false;
					}
					break;
				}
				case MOUSE_MOVE:
				{
					painttooltip = true;
					mouseOver = true;
					mx = MouseCoord->x;
					my = MouseCoord->y;
					break;
				}
				case MOUSE_DOWN:
				{
					mouseDown = true;
					fadeValue = 0.7f;
					break;
				}
				case MOUSE_OUT:
				{
					mouseOver = false;
					mouseDown = false;
					painttooltip = false;
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		int PicTextButton::Paint()
		{
			PaintBackground();
			PaintPicture();

			glEnable(GL_TEXTURE_2D);

			Window::GUI::FontCache::RenderedText RenderedInfo;
			Window::GUI::Fonts.SetFontType(this->type);
			Window::GUI::Fonts.RenderText(this->text , RenderedInfo, xUnit);
			float xPos = floor((w - RenderedInfo.w - 0.05f) / xUnit + 0.5f) * xUnit;
			float yPos = floor((1.0f - RenderedInfo.h - 0.05f) / yUnit + 0.5f) * yUnit;
			
			PaintLabel(RenderedInfo, xPos, yPos);
			PaintOverlay();
			return SUCCESS;
		}

		int TextButton::HandleEvent(EventType evtType, SDL_Event* event, TranslatedMouse* MouseCoord)
		{
			switch(evtType)
			{
				case MOUSE_UP:
				{
					if(mouseDown == true)
					{
						if(MouseCoord->x > 0 && MouseCoord->y > 0 && MouseCoord->x <= this->w && MouseCoord->y <= this->h)
						{
							if(onEvent)
							{
								if(evtType == MOUSE_UP)
								{
									(*onEvent)(evtType, tag);
								}
							}
						}
						mouseDown = false;
					}
					break;
				}
				case MOUSE_MOVE:
				{
					painttooltip = true;
					mouseOver = true;
					mx = MouseCoord->x;
					my = MouseCoord->y;
					break;
				}
				case MOUSE_DOWN:
				{
					mouseDown = true;
					fadeValue = 0.7f;
					break;
				}
				case MOUSE_OUT:
				{
					mouseOver = false;
					mouseDown = false;
					painttooltip = false;
					break;
				}
				default:
					break;
			}
			return SUCCESS;
		}

		void Button::PaintBorderOverlay()
		{
			if(mouseOver == true)
			{
				float borderSize = 0.025;
				float aspect = this->w / this->h;

				float bwb = borderSize / aspect;
				float bwe = 1.0f - bwb;

				glPushMatrix();
				glScalef(this->w, this->h, 1.0f);
				glBegin(GL_QUADS);
					glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
					glVertex2f(0.0f, 0.0f);
					glVertex2f(1.0f, 0.0f);
					glVertex2f(1.0f, borderSize);
					glVertex2f(0.0f, borderSize);

					glVertex2f(0.0f, 1.0f - borderSize);
					glVertex2f(1.0f, 1.0f - borderSize);
					glVertex2f(1.0f, 1.0f);
					glVertex2f(0.0f, 1.0f);

					glVertex2f(0.0f, borderSize);
					glVertex2f(bwb, borderSize);
					glVertex2f(bwb, 1.0f - borderSize);
					glVertex2f(0.0f, 1.0f - borderSize);

					glVertex2f(bwe, borderSize);
					glVertex2f(1.0f, borderSize);
					glVertex2f(1.0f, 1.0f - borderSize);
					glVertex2f(bwe, 1.0f - borderSize);
				glEnd();
				glPopMatrix();
			}
		}

		void Button::AttachHandler(funcptr fptr)
		{
			onEvent = fptr;
		}

		void Button::PaintOverlay()
		{
			if(fadeValue != 0.0f)
			{
				if(mouseDown == false)
				{
					fadeValue -= 1.4 * Window::GUI::time_since_last_frame;
					if(fadeValue < 0.0f)
					{
						fadeValue = 0.0f;
					}
				}
				glBegin(GL_QUADS);
					glColor4f(1.0f,1.0f,1.0f, fadeValue);
					glVertex2f(0.0f, 0.0f);
					glVertex2f(this->w, 0.0f);
					glVertex2f(this->w, this->h);
					glVertex2f(0.0f, this->h);
				glEnd();
			}
			PaintBorderOverlay();
		}

		int TextButton::Paint()
		{
			PaintOverlay();
			PaintLabel();
			return SUCCESS;
		}
		
		Progressbar::Progressbar()
		{
			this->vertical = false;
			this->maxValue = 1.0f;
			this->value = 0.0f;
			this->color[0] = 1.0f;
			this->color[1] = 1.0f;
			this->color[2] = 1.0f;
			this->color[3] = 1.0f;
			this->shade = 1.0f;
			this->shadeColors = false;
		}

		Progressbar::Progressbar(bool vertical)
		{
			this->vertical = vertical;
			this->maxValue = 1.0f;
			this->value = 0.0f;
			this->color[0] = 1.0f;
			this->color[1] = 1.0f;
			this->color[2] = 1.0f;
			this->color[3] = 1.0f;
		}

		void Progressbar::SetShade(bool active, float strength)
		{
			shadeColors = active;
			shade = strength;
		}
		
		void Progressbar::PaintProgressbar()
		{

			if(this->vertical == false)
			{
				float max = (this->value / this->maxValue) * this->w;
				glBegin(GL_QUADS);
					glColor4f(color[0], color[1], color[2] , color[3]);
					glVertex2f(0.0f, 0.0f);
					glVertex2f(max, 0.0f);
					if(shadeColors == true)
						glColor4f(color[0] * shade, color[1] * shade, color[2] * shade, color[3]);
					
					glVertex2f(max, h);
					glVertex2f(0.0f, h);
				glEnd();
			}
			else
			{
				float max = (this->value / this->maxValue) * this->h;
				glBegin(GL_QUADS);
					glColor4f(color[0], color[1], color[2], color[3]);
					glVertex2f(0.0f, h);
					glVertex2f(w, h);
					glVertex2f(w,h - max);
					glVertex2f(0.0f,h - max);
				glEnd();				
			}
		}

		int Progressbar::Paint()
		{
			PaintProgressbar();
			return SUCCESS;
		}

		void Progressbar::SetColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
		{
			this->color[0] = r;
			this->color[1] = g;
			this->color[2] = b;
			this->color[3] = a;
		}

		void Progressbar::Increment(float value)
		{
			this->value += value;
		}

		void Progressbar::SetMax(float value)
		{
			this->maxValue = value;
		}

		void Progressbar::SetValue(float value)
		{
			this->value = value;
		}

		Picture::Picture()
		{
			picture = 0;
			bgColor[0] = 0.0f;
			bgColor[1] = 0.0f;
			bgColor[2] = 0.0f;
			bgColor[3] = 0.0f;
			picColor[0] = 1.0f;
			picColor[1] = 1.0f;
			picColor[2] = 1.0f;
			picColor[3] = 1.0f;
			bgPicture = 0;
			quadCoords = NULL;
			texCoords = NULL;
		}

		void Picture::PaintPicture()
		{
			if(picture)
			{
				if(texCoords && quadCoords)
				{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, picture);
					glBegin(GL_QUADS);
						glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(texCoords[0], texCoords[1]);
						glVertex2f(quadCoords[0], quadCoords[1]);

						glTexCoord2f(texCoords[2], texCoords[3]);
						glVertex2f(quadCoords[2] / this->w, quadCoords[3]);

						glTexCoord2f(texCoords[4], texCoords[5]);
						glVertex2f(quadCoords[4] / this->w, quadCoords[5] / this->h);

						glTexCoord2f(texCoords[6], texCoords[7]);
						glVertex2f(quadCoords[6], quadCoords[7] / this->h);
					glEnd();
					glDisable(GL_TEXTURE_2D);
				}
				else
				{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, picture);
					glBegin(GL_QUADS);
						glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(0.0f, 0.0f);
						glVertex2f(0.0f, 0.0f);

						glTexCoord2f(1.0f, 0.0f);
						glVertex2f(this->w, 0.0f);

						glTexCoord2f(1.0f, 1.0f);
						glVertex2f(this->w, this->h);

						glTexCoord2f(0.0f, 1.0f);
						glVertex2f(0.0f, this->h);
					glEnd();
					glDisable(GL_TEXTURE_2D);

				}
			}
		}

		void Picture::PaintBackground()
		{
			if(bgPicture)
			{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, bgPicture);
					glBegin(GL_QUADS);
						glColor4f(1.0f,1.0f,1.0f,1.0f);
						glTexCoord2f(0.0f, 0.0f);
						glVertex2f(0.0f, 0.0f);

						glTexCoord2f(1.0f, 0.0f);
						glVertex2f(this->w, 0.0f);

						glTexCoord2f(1.0f, 1.0f);
						glVertex2f(this->w, this->h);

						glTexCoord2f(0.0f, 1.0f);
						glVertex2f(0.0f, this->h);
					glEnd();
					glDisable(GL_TEXTURE_2D);
			}
			else
			{
				if(bgColor[3] != 0.0f)
				{
					glBegin(GL_QUADS);
						glColor4f(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
						glVertex2f(0.0f, 0.0f);
						glVertex2f(this->w, 0.0f);
						glVertex2f(this->w, this->h);
						glVertex2f(0.0f, this->h);
					glEnd();		
				}
			}
		}

		int Picture::Paint()
		{
			PaintBackground();
			PaintPicture();
			return SUCCESS;
		}

		void Picture::SetPicture(GLuint pic)
		{
			this->picture = pic;
		}

		void Picture::SetPicture(GLuint pic, GLfloat *texCoords, GLfloat *quadCoords)
		{
			this->picture = pic;
			this->texCoords = texCoords;
			this->quadCoords = quadCoords;
		}

		void Picture::SetBackground(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
		{
			bgColor[0] = r;
			bgColor[1] = g;
			bgColor[2] = b;
			bgColor[3] = a;
		}

		void Picture::SetPictureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
		{
			picColor[0] = r;
			picColor[1] = g;
			picColor[2] = b;
			picColor[3] = a;
		}

		void Picture::SetBackground(GLuint pic)
		{
			this->bgPicture = pic;
		}

		int PicButton::Paint()
		{
			PaintBackground();
			PaintPicture();
			PaintOverlay();
			return SUCCESS;
		}
/*
		void Selector::AddSelection(string name)
		{
			selections.push_back(name);
		}

		void Selector::SetSelectedID(int id)
		{
			current = id;
		}

		int Selector::GetSelectedID()
		{
			return current;
		}

		int Selector::PaintArrows()
		{
			//1 width, at least 1 in middle else reduce width to match.
			float scale = 1.0f;
			if(w < 3.0)
			{
				
			}

			float xPos = scale / 2;
			float yPos = this->h / 2;
			
			glTranslatef(xPos, yPos, 0.0f);
			glScalef(scale, scale);
			glBegin(GL_TRIANGLES);
				glColor4f(1.0f,1.0f,1.0f,0.5f);
				glVertex2f(-0.5f, 0.5f);
				glVertex2f(0.5f, -0.5f);
				glVertex2f(0.5f, 0.5f);
			glEnd();

		}
*/
		LoadWindow::LoadWindow(float maxprogress)
		{
			if (!Game::Networking::isDedicatedServer)
			{
				glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			}
			this->progressbar = new Progressbar(false); 
			this->progressbar->SetMax(maxprogress);
			this->progressbar->SetValue(0.0f);
			this->progressbar->SetColor(0.5f, 0.0f, 0.0f, 0.75f);
			this->progressbar->SetShade(true, 0.6f);
			this->label = new Label();
			this->panel = new Panel();
			SetLayout();
			if (!Game::Networking::isDedicatedServer)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
		}

		void LoadWindow::SetProgress(float value)
		{
			this->progressbar->SetValue(value);
		}

		void LoadWindow::SetMessage(string message)
		{
			this->label->SetText(message);
		}

		void LoadWindow::Increment(float val)
		{
			this->progressbar->Increment(val);
			Update();
		}

		void LoadWindow::SetLayout()
		{
			SetPanel(panel);
			int id1 = panel->Add(progressbar);
			int id2 = panel->Add(label);
			panel->SetConstraint(id2, 0.1f, 0.8f, panel->GetWidth() - 0.2f, 0.05f);
			panel->SetConstraint(id1, 0.1f, 0.85f, panel->GetWidth() - 0.2f, 0.1f);
		}

		int LoadWindow::Update()
		{
			if (!Game::Networking::isDedicatedServer)
			{

				// nollställ backbufferten och depthbufferten
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// nollställ vyn
				glLoadIdentity();

				ProcessEvents();

				Utilities::SwitchTo2DViewport(w, h);
				PaintAll();
				Utilities::RevertViewport();

				SDL_GL_SwapBuffers();
			}

			return SUCCESS;
		}

		LoadWindow::~LoadWindow()
		{
			delete this->progressbar;
			delete this->label;
			delete this->panel;
		}

	}
}
