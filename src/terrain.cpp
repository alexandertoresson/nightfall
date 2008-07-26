#include "terrain.h"

#include "unit.h"
#include "game.h"
#include "networking.h"
#include "paths.h"
#include "utilities.h"
#include "window.h"
#include "materialxml.h"
#include "selectorxml.h"
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

namespace Game
{
	namespace Dimension
	{
		float terrainOffsetX, terrainOffsetY;
		float terrainHeight = 9.0f;
		float waterLevel = -1.35f, waterHeight = 0.1f;

		GLfloat    terrainMaterialModifiers[2][2] = {
		                                              {
		                                                0.35f,   // not seen, not lighted
		                                                0.6f    // seen, not lighted
		                                              },
		                                              {
		                                                0.35f,   // not seen, lighted
		                                                1.0f    // seen, lighted
		                                              }
		                                            };
							 
		// overall quality of terrain; increase to increase terrain detail
		float quality = 15000.0;
		
		// size of the squares that the landscape is divided into.
		// don't change if you don't really have to.
		const int q_square_size = 32;
		
		gc_root_ptr<HeightMap>::type heightMap;

		void GetMapCoord(Utilities::Vector3D pos_vector_near, Utilities::Vector3D pos_vector_far, float py, float &mx, float &my)
		{
			Utilities::Vector3D hit_pos;
			bool processed = false;
			Utilities::Vector3D v_templ[2], v[2][2][2];
			v_templ[0].set(-terrainOffsetX, py, -terrainOffsetY);
			v_templ[1].set(+terrainOffsetX, py + terrainHeight, +terrainOffsetY);
			for (int x=0;x<2;x++)
			{
				for (int y=0;y<2;y++)
				{
					for (int z=0;z<2;z++)
					{
						v[z][y][x].set(v_templ[x].x, v_templ[y].y, v_templ[z].z);
					}
				}
			}

			hit_pos = pos_vector_far;

			// intersection with the ground

			if (CheckLineIntersect(v[0][0][0], v[0][0][1], v[1][0][0], pos_vector_near, hit_pos, hit_pos))
				processed = true;
			
			// intersection with the upper 'wall'

			if (CheckLineIntersect(v[0][1][0], v[0][0][1], v[0][0][0], pos_vector_near, hit_pos, hit_pos))
				processed = true;

			// intersection with the lower 'wall'

			if (CheckLineIntersect(v[1][1][0], v[1][0][0], v[1][0][1], pos_vector_near, hit_pos, hit_pos))
				processed = true;

			// intersection with the left 'wall'

			if (CheckLineIntersect(v[1][1][0], v[0][0][0], v[1][0][0], pos_vector_near, hit_pos, hit_pos))
				processed = true;

			// intersection with the right 'wall'

			if (CheckLineIntersect(v[1][1][1], v[1][0][1], v[0][0][1], pos_vector_near, hit_pos, hit_pos))
				processed = true;

			if (!processed)
			{
//				cout << "FAIL!!!"; // no intersection at all
			}

			mx = hit_pos.x;
			my = hit_pos.z;

		}
	
		void CreateNormals()
		{
			Utilities::Vector3D temp_normal, normal, vector1, vector2, point_up, point_right, point_down, point_left, point_cur;
			bool up, right, down, left;

			heightMap->normals = pWorld->height;
			for(int y=0;y<pWorld->height;y++)
			{
				heightMap->normals[y] = pWorld->width;
				for(int x=0;x<pWorld->width;x++)
				{

					point_cur.set(0.0, heightMap->heights[y][x], 0.0);

					if (y != 0) // don't do any calculations on the 'up' vector if y = 0, as then it doesn't exist
					{
						point_up.set(0.0, heightMap->heights[y-1][x], -0.125);
						up = true;
					}
					else
					{
						// flag it as 'non-existant'
						up = false;
					}

					if (x != pWorld->width-1)
					{
						point_right.set(0.125, heightMap->heights[y][x+1], 0.0);
						right = true;
					}
					else
					{
						right = false;
					}

					if (y != pWorld->height-1)
					{
						point_down.set(0.0, heightMap->heights[y+1][x], 0.125);
						down = true;
					}
					else
					{
						down = false;
					}

					if (x != 0)
					{
						point_left.set(-0.125, heightMap->heights[y][x-1], 0.0);
						left = true;
					}
					else
					{
						left = false;
					}

					// reset the normal
					normal.set(0, 0, 0);

					/*   /|\
					    / | \
					   /nw|ne\
					   ---+---
					   \sw|se/
					    \ | /
					     \|/    */
					
					if (right && up)
					{
						// calculate the normal for the north-east triangle of the point
						// cross(v1 - v0, v0 - v2)
						// note that the polygon coordinates must be supplied clockwise or the normal will be inverted.
						vector1 = point_up - point_cur;
						vector2 = point_cur - point_right;
						vector1.cross(vector2);
						vector1.normalize();
						// add the north-east normal to the final normal
						normal += vector1;
					}
					
					if (down && right)
					{
						// calculate the normal for the south-east triangle of the point
						vector1 = point_right - point_cur;
						vector2 = point_cur - point_down;
						vector1.cross(vector2);
						vector1.normalize();
						// add the south-east normal to the final normal
						normal += vector1;
					}
					
					if (left && down)
					{
						// calculate the normal for the south-west triangle of the point
						vector1 = point_down - point_cur;
						vector2 = point_cur - point_left;
						vector1.cross(vector2);
						vector1.normalize();
						// add the south-west normal to the final normal
						normal += vector1;
					}
					
					if (up && left)
					{
						// calculate the normal for the north-west triangle of the point
						vector1 = point_left - point_cur;
						vector2 = point_cur - point_up;
						vector1.cross(vector2);
						vector1.normalize();
						// add the north-west normal to the final normal
						normal += vector1;
					}
					
					// normalize the final normal
					normal.normalize();

					// store it

					heightMap->normals[y][x].x = normal.x;
					heightMap->normals[y][x].y = normal.y;
					heightMap->normals[y][x].z = normal.z;

				}
			}
		}

/*		void CreateTexCoords()
		{

			HeightMipmaps[0][0].ppTexCoords = new UVWCoord**[pWorld->height];
			for(int y=0;y<pWorld->height;y++)
			{

				HeightMipmaps[0][0].ppTexCoords[y] = new UVWCoord*[pWorld->width];

				for(int x=0;x<pWorld->width;x++)
				{
					HeightMipmaps[0][0].ppTexCoords[y][x] = new UVWCoord;
					// set u coordinate according to steepness in terrain
					// handles edge cases
					if (x != 0 && x != pWorld->width-1 && y != 0 && y != pWorld->height-1)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = (fabs(pWorld->ppHeight[y][x+1] - pWorld->ppHeight[y][x-1]) +
											    fabs(pWorld->ppHeight[y-1][x] - pWorld->ppHeight[y+1][x]));
					}
					// y is at an edge
					else if (x != 0 && x != pWorld->width-1)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = (fabs(pWorld->ppHeight[y][x+1] - pWorld->ppHeight[y][x-1])) * 2;
					}
					// x is at an edge
					else if (y != 0 && y != pWorld->height-1)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = (fabs(pWorld->ppHeight[y+1][x] - pWorld->ppHeight[y-1][x])) * 2;
					}
					// x and y is at an edge
					else
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = 0.0;
					}
					// set v coordinate according to height
					HeightMipmaps[0][0].ppTexCoords[y][x]->v = 1 - (pWorld->ppHeight[y][x] / terrainHeight + 0.5f);

					HeightMipmaps[0][0].ppTexCoords[y][x]->u += (float) ((double) rand() / RAND_MAX - 0.5) * 0.01f;
					HeightMipmaps[0][0].ppTexCoords[y][x]->v += (float) ((double) rand() / RAND_MAX - 0.5) * 0.01f;

					// u and v must not be lower than 0.01 or 0.99 for artifacts to not happen
					if (HeightMipmaps[0][0].ppTexCoords[y][x]->u > 0.99f)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = 0.99f;
					}
					if (HeightMipmaps[0][0].ppTexCoords[y][x]->v > 0.99f)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->v = 0.99f;
					}
					if (HeightMipmaps[0][0].ppTexCoords[y][x]->u < 0.01f)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->u = 0.01f;
					}
					if (HeightMipmaps[0][0].ppTexCoords[y][x]->v < 0.01f)
					{
						HeightMipmaps[0][0].ppTexCoords[y][x]->v = 0.01f;
					}
					HeightMipmaps[0][0].ppTexCoords[y][x]->w = 0.0f;
				}
			}

		}*/

		void CalculateSteepness()
		{
			int n, steepness;
			float val = 0;
			int start_x, start_y;
			int end_x, end_y;
			heightMap->steepness = pWorld->height;
			for (int y = 0; y < pWorld->height; y++)
			{

				heightMap->steepness[y] = pWorld->width;

				for (int x = 0; x < pWorld->width; x++)
				{

					n = 0;
					start_y = y - 1 < 0 ? 0 : y - 1;
					start_x = x - 1 < 0 ? 0 : x - 1;

					end_y = y + 1 > pWorld->height-1 ? pWorld->height-1 : y + 1;
					end_x = x + 1 > pWorld->width-1 ? pWorld->width-1 : x + 1;

					for (int y2 = start_y; y2 <= end_y; y2++)
					{
						for (int x2 = start_x; x2 <= end_x; x2++)
						{
							val += fabs(heightMap->heights[y2][x2] - heightMap->heights[y][x]);
							n++;
						}
					}

					val /= (float) n-1; // to compensate for the middle point, which will always be 0

					steepness = (int) (val * 1000); // Scale the value

					if (steepness > 65535)
					{
						steepness = 65535;
					}

					heightMap->steepness[y][x] = steepness;

				}

			}
		}

		void InitFog()
		{
			cout << "Initializing fog of war..." << endl;

			Uint16** NumUnitsSeeingSquare;
			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				NumUnitsSeeingSquare = new Uint16*[pWorld->height];
				for (int y = 0; y < pWorld->height; y++)
				{
					NumUnitsSeeingSquare[y] = new Uint16[pWorld->width];
					for (int x = 0; x < pWorld->width; x++)
					{
						NumUnitsSeeingSquare[y][x] = 0;
					}
				}
				pWorld->vPlayers.at(i)->NumUnitsSeeingSquare = NumUnitsSeeingSquare;
			}
		}
		
		void InitLight()
		{
			cout << "Initializing light..." << endl;

			Uint16** NumLightsOnSquare;
			NumLightsOnSquare = new Uint16*[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{
				NumLightsOnSquare[y] = new Uint16[pWorld->width];
				for (int x = 0; x < pWorld->width; x++)
				{
					NumLightsOnSquare[y][x] = 0;
				}
			}
			pWorld->NumLightsOnSquare = NumLightsOnSquare;
		}
		
		int levelmap_width, levelmap_height;

		int water_cur_front = 1, water_cur_back = 0, water_interpolated = 2;

		int numwater;

		void InitWater()
		{
			int numbigwater = 0;
			std::vector<unsigned> dims;

			numwater = 0;

			dims.push_back(3);
			dims.push_back(pWorld->height);
			dims.push_back(pWorld->width);

			heightMap->water = dims;

			dims.erase(dims.begin());

			heightMap->squareHasWater = dims;
			heightMap->waterNormals = dims;

			for (int y = 0; y < pWorld->height; y++)
			{
				for (int x = 0; x < pWorld->width; x++)
				{
					if (waterHeight / 2 + waterLevel >= heightMap->heights[y][x])
					{
						heightMap->squareHasWater[y][x] = true;
					}
				}
			}

			for (int y = 0; y < levelmap_height; y++)
			{
				for (int x = 0; x < levelmap_width; x++)
				{
					for (int y2 = 0; y2 <= q_square_size; y2++)
					{
						for (int x2 = 0; x2 <= q_square_size; x2++)
						{
							if (heightMap->squareHasWater[y*q_square_size+y2][x*q_square_size+x2])
							{
								heightMap->bigSquareHasWater[y][x] = true;
								numbigwater += 1;
								goto next_big_square;
							}
						}
					}
					next_big_square:;
				}
			}

			cout << numwater << " water squares" << endl;
			cout << numbigwater << " big water squares" << endl;
			for (int i = 0; i < 100; i++)
			{
				CalculateWater();
			}
		}

		float GetWaterDepth(int x, int y)
		{
			float res = waterLevel + heightMap->water[water_cur_front][y][x] - heightMap->heights[y][x];
			if (res < 0)
			{
				res = 0;
			}
			return res;
		}

		void CalculateWater()
		{
			int height, width;
			float matrix[5][5] = {{0.05, 0.25, 0.33, 0.25, 0.05},
			                      {0.25, 0.53, 0.66, 0.53, 0.25},
			                      {0.33, 0.66, 1.00, 0.66, 0.33},
			                      {0.25, 0.53, 0.66, 0.53, 0.25},
			                      {0.05, 0.25, 0.33, 0.25, 0.05}};
			gc_array<float, 3>& ppWater = heightMap->water;
			height = pWorld->height-1;
			width = pWorld->width-1;

			int newCurFront = (water_cur_front + 1) % 3;
			int newCurBack = (water_cur_back + 1) % 3;

			int i = 0;

			for (int y = 1; y < height; y++)
			{
				for (int x = 1; x < width; x++)
				{
					if (heightMap->squareHasWater[y][x])
					{
						float theight = heightMap->heights[y][x];
						ppWater[newCurFront][y][x] = (ppWater[water_cur_front][y-1][x] +
						                              ppWater[water_cur_front][y+1][x] +
						                              ppWater[water_cur_front][y][x-1] +
						                              ppWater[water_cur_front][y][x+1]) / 2 -
						                              ppWater[water_cur_back][y][x];

						ppWater[newCurFront][y][x] *= 0.95f;
						if (ppWater[newCurFront][y][x] < theight - waterLevel)
						{
							ppWater[newCurFront][y][x] = theight - waterLevel;
						}
						if (waterLevel < theight)
						{
							ppWater[newCurFront][y][x] = 0.0f;
						}

					}

					i++;
				}
			}

			for (int i = 0; i < 1000; i++)
			{
				int y = rand() % (height-5) + 3;
				int x = rand() % (width-5) + 3;
				float height = heightMap->heights[y][x];
				if (waterLevel > height)
				{
					float val = (float) ((double) rand() / RAND_MAX - 0.5) * (waterLevel - height) * 0.02f;
					for (int y2 = 0; y2 < 5; y2++)
					{
						for (int x2 = 0; x2 < 5; x2++)
						{
							ppWater[newCurFront][y+y2-2][x+x2-2] += val * matrix[y2][x2];
						}
					}
				}
			}

			water_cur_back = newCurBack;
			water_cur_front = newCurFront;

		}

		void InitVBOs()
		{
			float world_square_size = 0.125f;

			std::vector<unsigned> dims;
			dims.push_back(levelmap_height);
			dims.push_back(levelmap_width);

			heightMap->bsvbos = dims;

			for (int y = 0; y < levelmap_height; y++)
			{
				for (int x = 0; x < levelmap_width; x++)
				{
					Scene::Render::VBO& positions = heightMap->bsvbos[y][x].positions;
					Scene::Render::VBO& normals = heightMap->bsvbos[y][x].normals;
					int len = (q_square_size+1)*(q_square_size+1)*3;

					positions.data.floats = new GLfloat[len];
					positions.size = len * sizeof(GLfloat);

					normals.data.floats = new GLfloat[len];
					normals.size = len * sizeof(GLfloat);

					int i = 0;
					int basey = y * q_square_size;
					int basex = x * q_square_size;

					for (int y2 = 0; y2 <= q_square_size; y2++)
					{
						for (int x2 = 0; x2 <= q_square_size; x2++)
						{
							positions.data.floats[i] = (basex + x2) * world_square_size - terrainOffsetX;
							normals.data.floats[i] = heightMap->normals[basey+y2][basex+x2].x;
							i++;
							positions.data.floats[i] = heightMap->heights[basey+y2][basex+x2];
							normals.data.floats[i] = heightMap->normals[basey+y2][basex+x2].y;
							i++;
							positions.data.floats[i] = (basey + y2) * world_square_size - terrainOffsetY;
							normals.data.floats[i] = heightMap->normals[basey+y2][basex+x2].z;
							i++;
						}
					}
				}
			}
			
			Scene::Render::VBO& light = heightMap->light;

			int flen = (q_square_size+1)*(q_square_size+1);

			light.data.floats = new GLfloat[flen];
			light.size = flen * sizeof(GLfloat);

			Scene::Render::VBO& texCoords = heightMap->texCoords;

			texCoords.data.floats = new GLfloat[flen*2];
			texCoords.size = flen * 2 * sizeof(GLfloat);

			int i = 0;
			for (int y2 = 0; y2 <= q_square_size; y2++)
			{
				for (int x2 = 0; x2 <= q_square_size; x2++)
				{
					texCoords.data.floats[i] = (float) x2 / pWorld->width;
					i++;
					texCoords.data.floats[i] = (float) y2 / pWorld->height;
					i++;
				}
			}

			Scene::Render::VBO& waterBack = heightMap->waterBack;
			Scene::Render::VBO& waterFront = heightMap->waterFront;

			waterBack.data.floats = new GLfloat[flen];
			waterBack.size = flen * sizeof(GLfloat);
			waterFront.data.floats = new GLfloat[flen];
			waterFront.size = flen * sizeof(GLfloat);

			Scene::Render::VBO& index = heightMap->index;

			int len = (q_square_size+1)*q_square_size*2+(q_square_size-1)*4;
			index.data.ushorts = new GLushort[len];
			index.size = len * sizeof(GLushort);
			index.numVals = len;

			i = 0;
			for (int y2 = 0; y2 < q_square_size; y2++)
			{
				if (y2 != 0)
				{
					index.data.ushorts[i] = y2*(q_square_size+1);
					i++;
					index.data.ushorts[i] = y2*(q_square_size+1);
					i++;
				}
				for (int x2 = 0; x2 <= q_square_size; x2++)
				{
					index.data.ushorts[i] = y2*(q_square_size+1)+x2;
					i++;
					index.data.ushorts[i] = (y2+1)*(q_square_size+1)+x2;
					i++;
				}
				if (y2 != q_square_size-1)
				{
					index.data.ushorts[i] = (y2+1)*(q_square_size+1)+q_square_size;
					i++;
					index.data.ushorts[i] = (y2+1)*(q_square_size+1)+q_square_size;
					i++;
				}
			}

			std::cout << len << " " << i << std::endl;
		}

		// array storing the start and end of displayed big squares, for every column of big squares
		int *is_visible[2];
		
		int LoadWorld(std::string filename)
		{
			int width = 0, height = 0, temp;
			std::string filepath = Utilities::GetDataFile(filename);
			std::ifstream file;

			if (!filepath.length())
			{
				cout << "Could not find heightmap " << filename << "!!" <<  endl;
				return ERROR_GENERAL;
			}

			cout << "Loading heightmap from " << filepath << "..." <<  endl;

			file.open(filepath.c_str());
			
			if (file.bad())
				return FILE_DOES_NOT_EXIST;

			if (file.peek() == 'P')
			{
				file.get(); file.get(); // << P2 - skip header!
			
				file >> width;
				if (width == 0)
				{
					cout << "Failed!!! Invalid width!" << endl;
				}

				file >> height;
				if (height == 0)
				{
					cout << "Failed!!! Invalid height!" << endl;
				}
				
				std::cout << "Width: " << width << ", height: " << height << std::endl;
				
				int highest;

				file >> highest;
			}
			else
			{
				file >> width;
				
				file >> height;
			}

//			pWorld = new World;

			pWorld->height = height;
			pWorld->width = width;
			terrainOffsetY = float(height / 16);
			terrainOffsetX = float(width / 16);
			
			heightMap = new HeightMap;

			heightMap->heights = height;
			
			// Läs från fil
			for (int y = 0; y < height; y++)
			{
				// scanline per scanline
				heightMap->heights[y] = width;
				for (int x = 0; x < width; x++)
				{
					file >> temp;
					heightMap->heights[y][x] = ((float) temp / 255-0.5f)*terrainHeight*0.5;
				}
			}

			file.close();
			
			levelmap_height = height/q_square_size;
			levelmap_width = width/q_square_size;

			std::vector<unsigned> dims;
			dims.push_back(levelmap_height);
			dims.push_back(levelmap_width);

			heightMap->bigSquareHasWater = dims;

			is_visible[0] = new int[levelmap_height];
			is_visible[1] = new int[levelmap_height];

			if (!Game::Rules::noGraphics)
			{

				cout << "Calculating normals for heightmap..." <<  endl;
				
				CreateNormals();

/*				cout << "Calculating texture coordinates for heightmap..." <<  endl;

				CreateTexCoords();*/
				
				cout << "Initializing water..." << endl;

				InitWater();

/*				cout << "Calculating mipmaps for heightmap..." <<  endl;
				
				CreateMipmaps();*/

				cout << "Initializing VBOs..." << endl;

				InitVBOs();

			}

			cout << "Calculating steepness..." << endl;

			CalculateSteepness();

			InitFog();
		
			InitLight();

			pppElements = new gc_ptr<Unit>*[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{
				pppElements[y] = new gc_ptr<Unit>[pWorld->width];
				for (int x = 0; x < pWorld->width; x++)
				{
					pppElements[y][x] = NULL;
				}
			}	

			return SUCCESS;
		}


		// Gets the x, y and z coords for a square, at the highest quality
		Utilities::Vector3D GetSquareCoord(float x, float y)
		{
			return Utilities::Vector3D(x / 8 - terrainOffsetX, heightMap->heights[(int) floor(y)][(int) floor(x)], y / 8 - terrainOffsetY);
		}

		// get the terrain height at the specified location, interpolated
		float GetTerrainHeight(float x, float y)
		{
			int bsx, bsy;
			int ssx, ssy;
			int mx, my;
			float xmix, ymix;

			bsx = (int) x / 32;
			bsy = (int) y / 32;

			if (x >= pWorld->width-1)
				bsx -= 1;
			if (y >= pWorld->height-1)
				bsy -= 1;

			const gc_array<float, 2>& heights = heightMap->heights;

			ssx = (int) floor(x - float(bsx * 32));
			ssy = (int) floor(y - float(bsy * 32));

			xmix = (x - float(bsx * 32 + ssx));
			ymix = (y - float(bsy * 32 + ssy));

			mx = bsx * 32 + ssx;
			my = bsy * 32 + ssy;

			if (mx >= pWorld->width-1)
			{
				xmix = 1.0;
				mx = pWorld->width-2;
			}

			if (my >= pWorld->height-1)
			{
				ymix = 1.0;
				my = pWorld->height-2;
			}

			if (mx < 0)
			{
				xmix = 0.0;
				mx = 0;
			}

			if (my < 0)
			{
				ymix = 0.0;
				my = 0;
			}

			return (heights[my][mx] * (1 - xmix) + heights[my][mx+1] * xmix) * (1 - ymix) +
			       ((heights[my+1][mx] * (1 - xmix) + heights[my+1][mx+1] * xmix) * ymix);
		}

		// get the terrain normal at the specified location, interpolated
		Utilities::Vector3D GetTerrainNormal(float x, float y)
		{
			int bsx, bsy;
			int ssx, ssy;
			int mx, my;
			float xmix, ymix;

			bsx = (int) x / 32;
			bsy = (int) y / 32;

			if (x >= pWorld->width-1)
				bsx -= 1;
			if (y >= pWorld->height-1)
				bsy -= 1;

			const gc_array<XYZCoord, 2>& normals = heightMap->normals;

			ssx = (int) floor(x - float(bsx * 32));
			ssy = (int) floor(y - float(bsy * 32));

			xmix = (x - float(bsx * 32 + ssx));
			ymix = (y - float(bsy * 32 + ssy));

			mx = bsx * 32 + ssx;
			my = bsy * 32 + ssy;

			if (mx >= pWorld->width-1)
			{
				xmix = 1.0;
				mx = pWorld->width-2;
			}

			if (my >= pWorld->height-1)
			{
				ymix = 1.0;
				my = pWorld->height-2;
			}

			if (mx < 0)
			{
				xmix = 0.0;
				mx = 0;
			}

			if (my < 0)
			{
				ymix = 0.0;
				my = 0;
			}

			return (Utilities::Vector3D(normals[my][mx]) * (1 - xmix) + Utilities::Vector3D(normals[my][mx+1]) * xmix) * (1 - ymix) +
			       ((Utilities::Vector3D(normals[my+1][mx]) * (1 - xmix) + Utilities::Vector3D(normals[my+1][mx+1]) * xmix) * ymix);
		}

		// get the terrain height at the specified location
		float GetTerrainHeight(int x, int y)
		{
			return heightMap->heights[y][x];
		}

		Utilities::Vector3D GetTerrainCoord(float x, float y)
		{
			return Utilities::Vector3D((x * 0.125f) - terrainOffsetX, GetTerrainHeight(x, y), (y * 0.125f) - terrainOffsetY);
		}

		Dimension::Position GetPosition(const Utilities::Vector3D& v)
		{
			return Dimension::Position(8.0f * (v.x + terrainOffsetX), 8.0f * (v.z + terrainOffsetY));
		}

		bool BigSquareIsRendered(int x, int y)
		{
			x >>= 5;
			y >>= 5;
			if (y >= 0 && y < levelmap_height)
				return (is_visible[0][y] <= x) && (is_visible[1][y] >= x);
			else if (y == levelmap_height)
				return (is_visible[0][levelmap_height-1] <= x) && (is_visible[1][levelmap_height-1] >= x);
			else
				return false;
		}

		bool BigSquaresAreRendered(int x1, int y1, int x2, int y2)
		{
			x1 >>= 5;
			y1 >>= 5;
			x2 >>= 5;
			y2 >>= 5;
			for (int y = y1; y < y2; y++)
			{
				if (y >= 0 && y < levelmap_height)
				{
					if ((is_visible[0][y] <= x1) && (is_visible[1][y] >= x1))
						return true;
					else if ((is_visible[0][y] <= x2) && (is_visible[1][y] >= x2))
						return true;
					else if ((is_visible[0][y] > x1) && (is_visible[1][y] < x2))
						return true;
				}
			}
			return false;
		}

		// Draws an 'inclusion line' from (x1, y1) to (x2, y2)
		void DefineInclusionLine(float x1, float y1, float x2, float y2)
		{
			int x1_i, y1_i, x2_i, y2_i, temp_i;
			float delta, newx, temp;

			// scale to big squares
			x1 = (x1 + terrainOffsetX) * 8 / 32;
			y1 = (y1 + terrainOffsetY) * 8 / 32;
			x2 = (x2 + terrainOffsetX) * 8 / 32;
			y2 = (y2 + terrainOffsetY) * 8 / 32;

			// check for too big and too small coordinates
			if (x1 < 0.0f) x1 = 0.0f;
			if (x1 >= levelmap_width) x1 = (float) levelmap_width-0.0001f;
			if (y1 < 0.0f) y1 = 0.0f;
			if (y1 >= levelmap_height) y1 = (float) levelmap_height-0.0001f;
			if (x2 < 0.0f) x2 = 0.0f;
			if (x2 >= levelmap_width) x2 = (float) levelmap_width-0.0001f;
			if (y2 < 0.0f) y2 = 0.0f;
			if (y2 >= levelmap_height) y2 = (float) levelmap_height-0.0001f;

			// convert to integers
			x1_i = (int) floor(x1);
			y1_i = (int) floor(y1);
			x2_i = (int) floor(x2);
			y2_i = (int) floor(y2);

			if (x1_i < 0 || x2_i < 0 || y1_i < 0 || y2_i < 0 || x1_i >= levelmap_width || x2_i >= levelmap_width || y1_i >= levelmap_height || y2_i >= levelmap_height)
			{
				return;
			}

			if (x1_i == x2_i) // special case: x is constant
			{
				if (y2_i >= y1_i) // two cases: y2 > y1 and y1 < y2
				{
					for (int y = y1_i; y <= y2_i; y++)
					{
						if (is_visible[0][y] > x1_i)
							is_visible[0][y] = x1_i; // set start
						if (is_visible[1][y] < x1_i)
							is_visible[1][y] = x1_i; // set end
					}
				}
				else
				{
					for (int y = y2_i; y <= y1_i; y++)
					{
						if (is_visible[0][y] > x1_i)
							is_visible[0][y] = x1_i; // set start
						if (is_visible[1][y] < x1_i)
							is_visible[1][y] = x1_i; // set end
					}
				}
			}
			else
			{
				// swap the points if y2 is lower than y1
				if (y2 < y1)
				{
					temp = y1;
					y1 = y2;
					y2 = temp;
					temp_i = y1_i;
					y1_i = y2_i;
					y2_i = temp_i;

					temp = x1;
					x1 = x2;
					x2 = temp;
					temp_i = x1_i;
					x1_i = x2_i;
					x2_i = temp_i;
				}

				delta = fabs(y2 - y1) / (x2 - x1); // calculate delta

				for (int y = y1_i; y != y2_i; y++)
				{
					newx = x1 + (float(y+1) - y1) / delta; // calculate new x
					// (y+1) - y1 is the distance to the next integer y, 
					// floor(y1+1)

					x1_i = (int) floor(x1); // start x and end x on this y
					x2_i = (int) floor(newx);

					if (x1_i > levelmap_width-1) x1_i = levelmap_width-1; // check for overflow
					if (x2_i > levelmap_width-1) x2_i = levelmap_width-1;
					if (x1_i < 0) x1_i = 0;
					if (x2_i < 0) x2_i = 0;

					if (x1_i < x2_i)
					{
						// set start x and end x
						if (is_visible[0][y] > x1_i)
							is_visible[0][y] = x1_i;
						if (is_visible[1][y] < x2_i)
							is_visible[1][y] = x2_i;
					}
					else
					{
						if (is_visible[0][y] > x2_i)
							is_visible[0][y] = x2_i;
						if (is_visible[1][y] < x1_i)
							is_visible[1][y] = x1_i;
					}

					x1 = newx;
					y1 = float(y+1);
				}

				// the last y must be handled in a special way, as the line
				// quite probably doesn't end on floor(y1+1), but on y2.
				newx = x1 + (y2 - y1) / delta;

				x1_i = (int) floor(x1);
				x2_i = (int) floor(newx);

				if (x1_i > levelmap_width-1) x1_i = levelmap_width-1; // check for overflow
				if (x2_i > levelmap_width-1) x2_i = levelmap_width-1;
				if (x1_i < 0) x1_i = 0;
				if (x2_i < 0) x2_i = 0;

				if (x1_i < x2_i)
				{
					if (is_visible[0][y2_i] > x1_i)
						is_visible[0][y2_i] = x1_i;
					if (is_visible[1][y2_i] < x2_i)
						is_visible[1][y2_i] = x2_i;
				}
				else
				{
					if (is_visible[0][y2_i] > x2_i)
						is_visible[0][y2_i] = x2_i;
					if (is_visible[1][y2_i] < x1_i)
						is_visible[1][y2_i] = x1_i;
				}
			}
		}

		void DrawWater()
		{
/*			int mipmap_level, scaled_square_size;
			float world_square_size;
			int mx, my;
			int mx1, my1, mx2, my2;
			float wx1, wy1, wx2, wy2;
			float mix = Rules::time_passed_since_last_water_pass * 3;
			int step_size;
			XYZCoord* normal;
			bool is_seen, is_lighted;
			Uint16** NumUnitsSeeingSquare = Dimension::currentPlayerView->NumUnitsSeeingSquare;
			
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, waterMaterialSpecular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, waterMaterialShininess);

			glEnable(GL_COLOR_MATERIAL);
			
			// draw each big square
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				//for (int x=0;x<pWorld->width/q_square_size;x++)
				for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{

					mipmap_level = waterQuality;

					// calculate the number of small squares in a big square at this mipmap level
					scaled_square_size = q_square_size >> mipmap_level;

					// calculate real size of a small square
					world_square_size = (float) 16 / (float) 128;

					// calculate the base index x and y coords into the height, normal and texcoord arrays at the current mipmap level
					mx = x * q_square_size;
					my = y * q_square_size;
					step_size = 1 << mipmap_level;
					
					if (big_square_has_water[y][x])
					{

						for (int y2=0;y2<=q_square_size;y2+=step_size)
						{

							for (int x2=0;x2<=q_square_size;x2+=step_size)
							{
								mx1 = mx + x2;
								my1 = my + y2;
								pWorld->ppWater[water_interpolated][my1][mx1] = pWorld->ppWater[water_cur_front][my1][mx1] * (1 - mix) + pWorld->ppWater[water_cur_back][my1][mx1] * mix;
							}

						}

						int start_x, start_y, end_x, end_y;
						Utilities::Vector3D x_vector, y_vector;

						start_x = mx == 0 ? 1 : mx;
						start_y = my == 0 ? 1 : my;
						end_x = mx + q_square_size == pWorld->width-1 ? pWorld->width-2 : mx + q_square_size;
						end_y = my + q_square_size == pWorld->height-1 ? pWorld->height-2 : my + q_square_size;

						for (int y2=start_y;y2<=end_y;y2+=step_size)
						{

							for (int x2=start_x;x2<=end_x;x2+=step_size)
							{
								y_vector = Utilities::Vector3D(0.0, pWorld->ppWater[water_interpolated][y2+1][x2] - pWorld->ppWater[water_interpolated][y2-1][x2], 0.250);
								x_vector = Utilities::Vector3D(0.250, pWorld->ppWater[water_interpolated][y2][x2+1] - pWorld->ppWater[water_interpolated][y2][x2-1], 0.0);
								y_vector.cross(x_vector);
								y_vector.normalize();
								water_normals[y2][x2]->x = y_vector.x;
								water_normals[y2][x2]->y = y_vector.y;
								water_normals[y2][x2]->z = y_vector.z;
							}

						}

						glDisable(GL_TEXTURE_2D);

						// we will use quad strips for rendering
						glBegin(GL_QUAD_STRIP);

						glColor3f(waterColor[0], waterColor[1], waterColor[2]);
						glNormal3f(0.0f, 1.0f, 0.0f);

						// 'scanline' by 'scanline'...
						for (int y2=0;y2<q_square_size;y2+=step_size)
						{

							mx1 = mx + 0;
							my1 = my + y2;
							my2 = my + y2 + step_size;
							wx1 = (float) mx1 * world_square_size - terrainOffsetX;
							wy1 = (float) my1 * world_square_size - terrainOffsetY;
							wy2 = (float) my2 * world_square_size - terrainOffsetY;

							if (y2 != 0)
							{
								// code for rendering invisible polygons that go to the starting position of the next scanline
								normal = water_normals[my1][mx1];
//								texcoord = texcoords[my1][mx1];
								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx1, waterLevel + pWorld->ppWater[water_interpolated][my1][mx1], wy1);

//								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx1, waterLevel + pWorld->ppWater[water_interpolated][my1][mx1], wy1);
								
							}

							is_seen = NumUnitsSeeingSquare[my1][mx1] > 0 ? 1 : 0;
							is_lighted = pWorld->NumLightsOnSquare[my1][mx1] > 0 ? 1 : 0;
							glColor4f(waterMaterialAmbientDiffuse[is_lighted][is_seen][0],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][1],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][2],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][3]);
							// initial coordinates
							normal = water_normals[my1][mx1];
//							texcoord = texcoords[my1][mx1];
							glNormal3f(normal->x, normal->y, normal->z);
//							glTexCoord2f(texcoord->u, texcoord->v);
							glVertex3f(wx1, waterLevel + pWorld->ppWater[water_interpolated][my1][mx1], wy1);
							

							is_seen = NumUnitsSeeingSquare[my2][mx1] > 0 ? 1 : 0;
							is_lighted = pWorld->NumLightsOnSquare[my2][mx1] > 0 ? 1 : 0;
							normal = water_normals[my2][mx1];
							glColor4f(waterMaterialAmbientDiffuse[is_lighted][is_seen][0],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][1],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][2],
								  waterMaterialAmbientDiffuse[is_lighted][is_seen][3]);
//							texcoord = texcoords[my2][mx1];
							glNormal3f(normal->x, normal->y, normal->z);
//							glTexCoord2f(texcoord->u, texcoord->v);
							glVertex3f(wx1, waterLevel + pWorld->ppWater[water_interpolated][my2][mx1], wy2);

							for (int x2=0;x2<q_square_size;x2+=step_size)
							{
								// map x1, y1, x2 and y2 for the current scaled sqaure
								mx1 = mx + x2;
								my1 = my + y2;
								mx2 = mx + x2 + step_size;
								my2 = my + y2 + step_size;
								// world x1, y1, x2 and y2
								wx1 = (float) mx1 * world_square_size - terrainOffsetX;
								wy1 = (float) my1 * world_square_size - terrainOffsetY;
								wx2 = (float) mx2 * world_square_size - terrainOffsetX;
								wy2 = (float) my2 * world_square_size - terrainOffsetY;
							
								is_seen = NumUnitsSeeingSquare[my1][mx2] > 0 ? 1 : 0;
								is_lighted = pWorld->NumLightsOnSquare[my1][mx2] > 0 ? 1 : 0;
							
								glColor4f(waterMaterialAmbientDiffuse[is_lighted][is_seen][0],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][1],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][2],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][3]);
								// render it!
								normal = water_normals[my1][mx2];
//								texcoord = texcoords[my1][mx2];
								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, waterLevel + pWorld->ppWater[water_interpolated][my1][mx2], wy1);

								is_seen = NumUnitsSeeingSquare[my2][mx2] > 0 ? 1 : 0;
								is_lighted = pWorld->NumLightsOnSquare[my2][mx2] > 0 ? 1 : 0;

								glColor4f(waterMaterialAmbientDiffuse[is_lighted][is_seen][0],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][1],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][2],
									  waterMaterialAmbientDiffuse[is_lighted][is_seen][3]);
								normal = water_normals[my2][mx2];
//								texcoord = texcoords[my2][mx2];
								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, waterLevel + pWorld->ppWater[water_interpolated][my2][mx2], wy2);
//								glVertex3f(wx2, waterLevel, wy2);
								
								
							}
							
							if (y != q_square_size)
							{
								// code for rendering invisible polygons that go to the starting position of the next scanline
//								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, waterLevel + pWorld->ppWater[water_interpolated][my2][mx2], wy2);
//								glVertex3f(wx2, waterLevel, wy2);

//								glNormal3f(normal->x, normal->y, normal->z);
//								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, waterLevel + pWorld->ppWater[water_interpolated][my2][mx2], wy2);
//								glVertex3f(wx2, waterLevel, wy2);
							}
							
						}

						glEnd();

						glEnable(GL_TEXTURE_2D);

					}
				}
			}
			
			glDisable(GL_COLOR_MATERIAL);*/
			
		}

		void UnloadTerrain()
		{
			heightMap = NULL;
		}

		TerrainNode::TerrainNode() : GLStateNode(new Scene::Render::GLState)
		{
			myGLState->material = Utilities::LoadMaterialXML(material);
		}

		std::string TerrainNode::material = "materials/terrain";

		void TerrainNode::Render()
		{
			matrices[MATRIXTYPE_MODELVIEW].Apply();

			Utilities::Vector3D pos_vector_near, pos_vector_far, cur_mod_pos, void_pos;
			
			float fmx_low_v[2][64], fmy_low_v[2][64], fmx_high_v[2][64], fmy_high_v[2][64];

			float fmx_low_h[64][2], fmy_low_h[64][2], fmx_high_h[64][2], fmy_high_h[64][2];

			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				is_visible[0][y] = 65536;
				is_visible[1][y] = -65536;
			}

			// Get the map coords at a few different places on the screen,
			// at a resolution of 2x64 by default.
			// Note: what is returned is not the pure map coords, rather coords in
			// the terrain mesh.
			// fm[xy]_low positions are derived from the case when the terrain is
			// at its lowest possible height, -1.5f.
			// fm[xy]_high is derived from when the terrain is at its highest
			// possible height, +1.5f
			for (int y=0;y<64;y++)
			{
				for (int x=0;x<2;x++)
				{
					// first get vector of ray into the screen at the
					// appropriate screen position
					Utilities::WindowCoordToVector((Window::windowWidth-1) * x, ((Window::windowHeight-1)/63) * y, pos_vector_near, pos_vector_far);
					// then get map mesh coords
					Dimension::GetMapCoord(pos_vector_near, pos_vector_far, -terrainHeight/2, fmx_low_v[x][y], fmy_low_v[x][y]);
					if (cur_mod_pos.y < terrainHeight/2)
					{
						Dimension::GetMapCoord(pos_vector_near, pos_vector_far, pos_vector_near.y, fmx_high_v[x][y], fmy_high_v[x][y]);
					}
					else
					{
						Dimension::GetMapCoord(pos_vector_near, pos_vector_far, +terrainHeight/2, fmx_high_v[x][y], fmy_high_v[x][y]);
					}
				}
			}
			
			for (int y=0;y<2;y++)
			{
				for (int x=0;x<64;x++)
				{
					// first get vector of ray into the screen at the
					// appropriate screen position
					Utilities::WindowCoordToVector(((Window::windowWidth-1)/63) * x, (Window::windowHeight-1) * y, pos_vector_near, pos_vector_far);
					// then get map mesh coords
					Dimension::GetMapCoord(pos_vector_near, pos_vector_far, -terrainHeight/2, fmx_low_h[x][y], fmy_low_h[x][y]);
					if (cur_mod_pos.y < terrainHeight/2)
					{
						Dimension::GetMapCoord(pos_vector_near, pos_vector_far, pos_vector_near.y, fmx_high_h[x][y], fmy_high_h[x][y]);
					}
					else
					{
						Dimension::GetMapCoord(pos_vector_near, pos_vector_far, +terrainHeight/2, fmx_high_h[x][y], fmy_high_h[x][y]);
					}
				}
			}
			
			// Draw inclusion lines for the left and right sides of the low and
			// high view frustums
			for (int y=0;y<63;y++)
			{
				for (int x=0;x<2;x++)
				{
					DefineInclusionLine(fmx_low_v[x][y], fmy_low_v[x][y], fmx_low_v[x][y+1], fmy_low_v[x][y+1]);
					DefineInclusionLine(fmx_high_v[x][y], fmy_high_v[x][y], fmx_high_v[x][y+1], fmy_high_v[x][y+1]);
				}
			}
			
			for (int y=0;y<2;y++)
			{
				for (int x=0;x<63;x++)
				{
					DefineInclusionLine(fmx_low_h[x][y], fmy_low_h[x][y], fmx_low_h[x+1][y], fmy_low_h[x+1][y]);
					DefineInclusionLine(fmx_high_h[x][y], fmy_high_h[x][y], fmx_high_h[x+1][y], fmy_high_h[x+1][y]);
				}
			}
			
			// Draw the inclusion vectors for the upper and lower sides of the
			// view frustums
			DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], fmx_low_v[1][0], fmy_low_v[1][0]);
			DefineInclusionLine(fmx_low_v[0][63], fmy_low_v[0][63], fmx_low_v[1][63], fmy_low_v[1][63]);
			DefineInclusionLine(fmx_high_v[0][0], fmy_high_v[0][0], fmx_high_v[1][0], fmy_high_v[1][0]);
			DefineInclusionLine(fmx_high_v[0][63], fmy_high_v[0][63], fmx_high_v[1][63], fmy_high_v[1][63]);

			// Connect corners of high and low view frustums
			DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], fmx_high_v[0][0], fmy_high_v[0][0]);
			DefineInclusionLine(fmx_low_v[0][63], fmy_low_v[0][63], fmx_high_v[0][63], fmy_high_v[0][63]);
			DefineInclusionLine(fmx_low_v[1][63], fmy_low_v[1][63], fmx_high_v[1][63], fmy_high_v[1][63]);
			DefineInclusionLine(fmx_low_v[1][0], fmy_low_v[1][0], fmx_high_v[1][0], fmy_high_v[1][0]);

			// special cases, where the frustums include parts of two neighbouring
			// sides of the map. doing this with the high frustum is not needed,
			// as if the high frustum reaches the sides, the low frustum does too.
			// One possible scenario:
			// +----------+
			// |   |      |
			// |   |      |
			// |\  |      |
			// |  \|      |
			// |   *      |
			// +----------+
			// Well, crappy ascii art, but you prolly get the idea.
			// Anyway, in this scenario, the frustum reaches the upper and left
			// sides, and should thus include them. However, this won't happen
			// without a special case, as the normal inclusion lines will just
			// draw a line between the places where the view vectors hit the sides

			// left and upper
			if (fmx_low_v[0][0] < -terrainOffsetX+0.01 && fmy_low_v[1][0] < -terrainOffsetY+0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], -terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, -terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}
			// upper and right
			if (fmy_low_v[0][0] < -terrainOffsetY+0.01 && fmx_low_v[1][0] > terrainOffsetX-0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, -terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}
			// right and lower
			if (fmx_low_v[0][0] > terrainOffsetX-0.01 && fmy_low_v[1][0] > terrainOffsetY-0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}

			// lower and left
			if (fmy_low_v[0][0] > terrainOffsetY-0.01 && fmx_low_v[1][0] < -terrainOffsetX+0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], -terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}

			// special case where the frustum includes a full side and parts of
			// the two neighbouring sides. Example scenario:
			// +----------+
			// |          |
			// |\         |
			// | \       /|
			// |   \   /  |
			// |     *    |
			// +----------+

			// left, upper and right
			if (fmx_low_v[0][0] < -terrainOffsetX+0.01 && fmx_low_v[1][0] > terrainOffsetX-0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], -terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, -terrainOffsetY, terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, -terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}
			// rigt, lower and left
			if (fmx_low_v[0][0] > terrainOffsetX-0.01 && fmx_low_v[1][0] < -terrainOffsetX+0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, terrainOffsetY, -terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}
			// upper, right and lower
			if (fmy_low_v[0][0] < -terrainOffsetY+0.01 && fmy_low_v[1][0] > terrainOffsetY-0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, -terrainOffsetY, terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(terrainOffsetX, terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}
			// lower, left and upper
			if (fmy_low_v[0][0] > terrainOffsetY-0.01 && fmy_low_v[1][0] < -terrainOffsetY+0.01)
			{
				DefineInclusionLine(fmx_low_v[0][0], fmy_low_v[0][0], -terrainOffsetX, terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, terrainOffsetY, -terrainOffsetX, -terrainOffsetY);
				DefineInclusionLine(-terrainOffsetX, -terrainOffsetY, fmx_low_v[1][0], fmy_low_v[1][0]);
			}

			// get the map mesh position of the viewer by getting the near
			// viewing plane in the middle of the screen
			Utilities::WindowCoordToVector((Window::windowWidth-1) * 0.5, (Window::windowHeight-1) * 0.5, cur_mod_pos, void_pos);

			heightMap->index.Lock();

			glEnableClientState(GL_INDEX_ARRAY);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, heightMap->index.buffer);
			glIndexPointer(GL_SHORT, 0, NULL);

			heightMap->texCoords.Lock();

			glEnable(GL_TEXTURE_2D);

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, heightMap->texCoords.buffer);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);

			// draw each big square
			for (int y=0;y<levelmap_height;y++)
			{
				//for (int x=0;x<pWorld->width/q_square_size;x++)
				for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{

					TerrainBSVBOs& vbos = heightMap->bsvbos[y][x];

					int i = 0;
					int my = y * q_square_size;
					int basex = x * q_square_size;
					int mx = basex;
					float* data = heightMap->light.data.floats;
					Uint16** NLOS = pWorld->NumLightsOnSquare;
					Uint16** NUSS = currentPlayerView->NumUnitsSeeingSquare;

					for (int y2 = 0; y2 <= q_square_size; y2++)
					{
						mx = basex;
						for (int x2 = 0; x2 <= q_square_size; x2++)
						{
							data[i] = terrainMaterialModifiers[NLOS[my][mx]>0][NUSS[my][mx]>0];
							mx++;
							i++;
						}
						my++;
					}

					heightMap->light.SetChanged();

					vbos.positions.Lock();
					vbos.normals.Lock();
					heightMap->light.Lock();

					int loc = glGetUniformLocationARB(myGLState->material->program, "baseX");
					glUniform1fARB(loc, float(x * q_square_size) / pWorld->width);

					loc = glGetUniformLocationARB(myGLState->material->program, "baseY");
					glUniform1fARB(loc, float(y * q_square_size) / pWorld->height);

					loc = glGetAttribLocationARB(myGLState->material->program, "light");
					glEnableVertexAttribArrayARB(loc);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, heightMap->light.buffer);
					glVertexAttribPointerARB(loc, 1, GL_FLOAT, GL_FALSE, 0, NULL);

					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbos.positions.buffer);
					glVertexPointer(3, GL_FLOAT, 0, NULL);

					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbos.normals.buffer);
					glNormalPointer(GL_FLOAT, 0, NULL);

					glDrawElements(GL_QUAD_STRIP, heightMap->index.numVals, GL_UNSIGNED_SHORT, NULL);

					vbos.positions.Unlock();
					vbos.normals.Unlock();

				}
			}

			heightMap->texCoords.Unlock();

			heightMap->index.Unlock();
		}

		WaterNode::WaterNode() : GLStateNode(new Scene::Render::GLState)
		{
			myGLState->material = Utilities::LoadMaterialXML(material);
		}

		std::string WaterNode::material = "materials/water";

		void WaterNode::Render()
		{
			int loc;
			int curFront = water_cur_front;
			int curBack = water_cur_back;

			matrices[MATRIXTYPE_MODELVIEW].Apply();

			float mix = Rules::time_passed_since_last_water_pass * 3;

			heightMap->index.Lock();

			glEnableClientState(GL_INDEX_ARRAY);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, heightMap->index.buffer);
			glIndexPointer(GL_SHORT, 0, NULL);

			Scene::Render::VBO& waterBack = heightMap->waterBack;
			Scene::Render::VBO& waterFront = heightMap->waterFront;

			glEnableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			
			loc = glGetUniformLocationARB(myGLState->material->program, "mixLevels");
			glUniform1fARB(loc, mix);

			loc = glGetUniformLocationARB(myGLState->material->program, "waterLevel");
			glUniform1fARB(loc, waterLevel);

			glNormal3f(0.0, 1.0, 0.0);

			// draw each big square
			for (int y=0;y<levelmap_height;y++)
			{
				//for (int x=0;x<pWorld->width/q_square_size;x++)
				for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{
					
					if (heightMap->bigSquareHasWater[y][x])
					{
						TerrainBSVBOs& vbos = heightMap->bsvbos[y][x];
						vbos.positions.Lock();

						glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbos.positions.buffer);
						glVertexPointer(3, GL_FLOAT, 0, NULL);

						{
							int i = 0;
							int my = y * q_square_size;
							int basex = x * q_square_size;
							int mx = basex;
							float* data = heightMap->light.data.floats;
							Uint16** NLOS = pWorld->NumLightsOnSquare;
							Uint16** NUSS = currentPlayerView->NumUnitsSeeingSquare;

							for (int y2 = 0; y2 <= q_square_size; y2++)
							{
								mx = basex;
								for (int x2 = 0; x2 <= q_square_size; x2++)
								{
									data[i] = terrainMaterialModifiers[NLOS[my][mx]>0][NUSS[my][mx]>0];
									mx++;
									i++;
								}
								my++;
							}
						}

						heightMap->light.SetChanged();

						heightMap->light.Lock();

						loc = glGetAttribLocationARB(myGLState->material->program, "light");
						glEnableVertexAttribArrayARB(loc);
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, heightMap->light.buffer);
						glVertexAttribPointerARB(loc, 1, GL_FLOAT, GL_FALSE, 0, NULL);

						int basex = x * q_square_size;
						int mx = basex;
						int my = y * q_square_size;
						int i = 0;
						
						for (int y2 = 0; y2 <= q_square_size; y2++)
						{
							mx = basex;
							for (int x2 = 0; x2 <= q_square_size; x2++)
							{
								waterBack.data.floats[i] = heightMap->water[curBack][my][mx];
								waterFront.data.floats[i] = heightMap->water[curFront][my][mx];
								mx++;
								i++;
							}
							my++;
						}

						waterBack.SetChanged();
						waterFront.SetChanged();

						waterBack.Lock();
						waterFront.Lock();

						loc = glGetAttribLocationARB(myGLState->material->program, "waterBack");
						glEnableVertexAttribArrayARB(loc);
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, waterBack.buffer);
						glVertexAttribPointerARB(loc, 1, GL_FLOAT, GL_FALSE, 0, NULL);

						loc = glGetAttribLocationARB(myGLState->material->program, "waterFront");
						glEnableVertexAttribArrayARB(loc);
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, waterFront.buffer);
						glVertexAttribPointerARB(loc, 1, GL_FLOAT, GL_FALSE, 0, NULL);

						glDrawElements(GL_QUAD_STRIP, heightMap->index.numVals, GL_UNSIGNED_SHORT, NULL);
						
						waterFront.Unlock();
						waterBack.Unlock();

						vbos.positions.Unlock();
						heightMap->light.Unlock();

					}
				}
			}

			heightMap->index.Unlock();

		}

	}
}	
