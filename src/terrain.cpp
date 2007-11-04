#include "terrain.h"

#include "unit.h"
#include "game.h"
#include "networking.h"
#include "paths.h"
#include <iostream>
#include <fstream>
#include <cmath>

namespace Game
{
	namespace Dimension
	{
		float terrainOffsetX, terrainOffsetY;
		float terrainHeight = 3.0f;
		float waterLevel = -1.35f, waterHeight = 0.1f;

		// overall quality of terrain; increase to increase terrain detail
		float quality = 15000.0;
		
		// water mipmap level, decrease to increase quality level
		int waterQuality = 1;
		
		// size of the squares that the landscape is divided into.
		// don't change if you don't really have to.
		const int q_square_size = 32;
		
		int** mipmap_levels;

		GLfloat    terrainMaterialAmbientDiffuse[2][2][4] = {
		                                                      {
		                                                        {0.1f, 0.1f, 0.1f, 1.0f},   // not seen, not lighted
		                                                        {0.2f, 0.2f, 0.2f, 1.0f}    // seen, not lighted
								      },
		                                                      {
		                                                        {0.1f, 0.1f, 0.1f, 1.0f},   // not seen, lighted
		                                                        {0.5f, 0.5f, 0.5f, 1.0f}    // seen, lighted
								      }
								    };

		GLfloat    terrainMaterialSpecular[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		GLfloat    terrainMaterialEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat    terrainMaterialShininess = 1.0;
		
		GLfloat    waterMaterialAmbientDiffuse[2][2][4] = {
		                                                    {
		                                                      {0.02f, 0.02f, 0.1f, 0.66f},   // not seen, not lighted
		                                                      {0.04f, 0.04f, 0.2f, 0.66f}    // seen, not lighted
								    },
		                                                    {
		                                                      {0.02f, 0.02f, 0.1f, 0.66f},   // not seen, lighted
		                                                      {0.1f, 0.1f, 0.5f, 0.66f}    // seen, lighted
								    }
								  };
		GLfloat    waterMaterialSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat    waterMaterialEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat    waterMaterialShininess = 100.0;
		GLfloat    waterColor[3] = { 0.0f, 0.0f, 1.0f };
		
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
	
		HeightMap HeightMipmaps[2][32];
		int highest_mipmap;

		void CreateMipmaps()
		{
			int mipmap_num = 1;
			int new_width, new_height, old_width, old_height;
			float height, weight;
#ifdef TERRAIN_HIGH_QUALITY
			float n_phi, n_theta;
#else
			float n_x, n_y, n_z;
#endif
			float t_u, t_v, t_w;

			// weights for surrounding squares (weights[1][1] is the 'current square')
			float weights[3][3] = {{1.0f, 2.0f, 1.0f},
					       {2.0f, 4.0f, 2.0f},
					       {1.0f, 2.0f, 1.0f}};

			// get the main heightmap
			HeightMipmaps[0][0].ppHeights = pWorld->ppHeight;

			new_width = pWorld->width;
			new_height = pWorld->height;

			// mipmap level 1 will be the level with next to highest quality,
			// level 2 will be the one below level 1 etc. level 1 is calculated
			// from level 0, level 2 is calculated from level 1 etc, until no
			// more mipmaps are possible to calculate.
			while (1)
			{

				// calculate the new height and width
				// if the first height is 257, the next is 129, then 65 etc
				old_width = new_width;
				old_height = new_height;
				new_width = ((new_width - 1) / 2) + 1;
				new_height = ((new_height - 1) / 2) + 1;

				// exit if it's not possible to calculate any more mipmaps
				if ((old_width != (new_width-1)*2+1) || old_height != (new_height-1)*2+1)
					break;

				cout << "\tLevel " << mipmap_num << "..."<< endl;

				HeightMipmaps[0][mipmap_num].ppHeights = new float*[new_height];
#ifdef TERRAIN_HIGH_QUALITY
				HeightMipmaps[0][mipmap_num].ppNormals = new SphereNormal**[new_height];
#else
				HeightMipmaps[0][mipmap_num].ppNormals = new XYZCoord**[new_height];
#endif
				HeightMipmaps[0][mipmap_num].ppTexCoords = new UVWCoord**[new_height];
//				HeightMipmaps[0][mipmap_num].ppSquareHasWater = new bool*[new_height];

				for(int y=0;y<new_height;y++)
				{

					HeightMipmaps[0][mipmap_num].ppHeights[y] = new float[new_width];
#ifdef TERRAIN_HIGH_QUALITY
					HeightMipmaps[0][mipmap_num].ppNormals[y] = new SphereNormal*[new_width];
#else
					HeightMipmaps[0][mipmap_num].ppNormals[y] = new XYZCoord*[new_width];
#endif
					HeightMipmaps[0][mipmap_num].ppTexCoords[y] = new UVWCoord*[new_width];
//					HeightMipmaps[0][mipmap_num].ppSquareHasWater[y] = new bool[new_width];

					for(int x=0;x<new_width;x++)
					{
						height = 0.0;
						weight = 0.0;

#ifdef TERRAIN_HIGH_QUALITY
						n_phi = 0.0;
						n_theta = 0.0;
#else
						n_x = 0.0;
						n_y = 0.0;
						n_z = 0.0;
#endif
						t_u = 0.0;
						t_v = 0.0;
						t_w = 0.0;

						// get the height, texture coords and normals
						// for the surrounding squares, and add them
						// together according to the specified weights
						// ebove.
						for (int y2=-1;y2<2;y2++)
						{
							for(int x2=-1;x2<2;x2++)
							{
								if (x+x2>=0 && y+y2>= 0 && x+x2<new_width && y+y2<new_height)
								{
									// height
									height += HeightMipmaps[0][mipmap_num-1].ppHeights[y*2+y2][x*2+x2]*weights[y2+1][x2+1];

									// normals
#ifdef TERRAIN_HIGH_QUALITY
									n_phi += HeightMipmaps[0][mipmap_num-1].ppNormals[y*2+y2][x*2+x2]->phi*weights[y2+1][x2+1];
									n_theta += HeightMipmaps[0][mipmap_num-1].ppNormals[y*2+y2][x*2+x2]->theta*weights[y2+1][x2+1];
#else
									n_x += HeightMipmaps[0][mipmap_num-1].ppNormals[y*2+y2][x*2+x2]->x*weights[y2+1][x2+1];
									n_y += HeightMipmaps[0][mipmap_num-1].ppNormals[y*2+y2][x*2+x2]->y*weights[y2+1][x2+1];
									n_z += HeightMipmaps[0][mipmap_num-1].ppNormals[y*2+y2][x*2+x2]->z*weights[y2+1][x2+1];
#endif

									// texture coords
									t_u += HeightMipmaps[0][mipmap_num-1].ppTexCoords[y*2+y2][x*2+x2]->u*weights[y2+1][x2+1];
									t_v += HeightMipmaps[0][mipmap_num-1].ppTexCoords[y*2+y2][x*2+x2]->v*weights[y2+1][x2+1];
									t_w += HeightMipmaps[0][mipmap_num-1].ppTexCoords[y*2+y2][x*2+x2]->w*weights[y2+1][x2+1];

									// weights
									weight += weights[y2+1][x2+1];
								}
							}
						}

						// divide results by weight, and store them

						height /= weight;

#ifdef TERRAIN_HIGH_QUALITY
						n_phi /= weight;
						n_theta /= weight;
#else
						n_x /= weight;
						n_y /= weight;
						n_z /= weight;
#endif

						t_u /= weight;
						t_v /= weight;
						t_w /= weight;

						HeightMipmaps[0][mipmap_num].ppHeights[y][x] = height;

						HeightMipmaps[0][mipmap_num].ppTexCoords[y][x] = new UVWCoord;

#ifdef TERRAIN_HIGH_QUALITY
						HeightMipmaps[0][mipmap_num].ppNormals[y][x] = new SphereNormal;
						HeightMipmaps[0][mipmap_num].ppNormals[y][x]->phi = n_phi;
						HeightMipmaps[0][mipmap_num].ppNormals[y][x]->theta = n_theta;
#else
						HeightMipmaps[0][mipmap_num].ppNormals[y][x] = new XYZCoord;
						HeightMipmaps[0][mipmap_num].ppNormals[y][x]->x = n_x;
						HeightMipmaps[0][mipmap_num].ppNormals[y][x]->y = n_y;
						HeightMipmaps[0][mipmap_num].ppNormals[y][x]->z = n_z;
#endif
						
						HeightMipmaps[0][mipmap_num].ppTexCoords[y][x]->u = t_u;
						HeightMipmaps[0][mipmap_num].ppTexCoords[y][x]->v = t_v;
						HeightMipmaps[0][mipmap_num].ppTexCoords[y][x]->w = t_w;

/*						HeightMipmaps[0][mipmap_num].ppSquareHasWater[y][x] =
							HeightMipmaps[0][mipmap_num-1].ppSquareHasWater[y*2][x*2];

						if (x != new_width-1)
						{
							HeightMipmaps[0][mipmap_num].ppSquareHasWater[y][x] |= HeightMipmaps[0][mipmap_num-1].ppSquareHasWater[y*2][x*2+1];
							if (y != new_height-1)
							{
								HeightMipmaps[0][mipmap_num].ppSquareHasWater[y][x] |= HeightMipmaps[0][mipmap_num-1].ppSquareHasWater[y*2+1][x*2+1];
							}
						}
						if (y != new_height-1)
						{
							HeightMipmaps[0][mipmap_num].ppSquareHasWater[y][x] |= HeightMipmaps[0][mipmap_num-1].ppSquareHasWater[y*2+1][x*2];
						}*/
					}
				}
#ifdef TERRAIN_HIGH_QUALITY
				
				HeightMipmaps[1][mipmap_num].ppHeights = new float*[old_height];
				HeightMipmaps[1][mipmap_num].ppNormals = new SphereNormal**[old_height];
				HeightMipmaps[1][mipmap_num].ppTexCoords = new UVWCoord**[old_height];

				for(int y=0;y<old_height;y++)
				{
					HeightMipmaps[1][mipmap_num].ppHeights[y] = new float[old_width];
					HeightMipmaps[1][mipmap_num].ppNormals[y] = new SphereNormal*[old_width];
					HeightMipmaps[1][mipmap_num].ppTexCoords[y] = new UVWCoord*[old_width];
				}

				for(int y=0;y<new_height;y++)
				{
					for(int x=0;x<new_width;x++)
					{
						int x2 = x<<1, y2 = y<<1;
						float **heights_1 = HeightMipmaps[0][mipmap_num].ppHeights, **heights_2 = HeightMipmaps[1][mipmap_num].ppHeights;
						SphereNormal ***normals_1 = HeightMipmaps[0][mipmap_num].ppNormals, ***normals_2 = HeightMipmaps[1][mipmap_num].ppNormals;
						UVWCoord ***texcoords_1 = HeightMipmaps[0][mipmap_num].ppTexCoords, ***texcoords_2 = HeightMipmaps[1][mipmap_num].ppTexCoords;
						normals_2[y2][x2] = new SphereNormal;
						normals_2[y2][x2]->phi = normals_1[y][x]->phi;
						normals_2[y2][x2]->theta = normals_1[y][x]->theta;
						texcoords_2[y2][x2] = new UVWCoord;
						texcoords_2[y2][x2]->u = texcoords_1[y][x]->u;
						texcoords_2[y2][x2]->v = texcoords_1[y][x]->v;
						texcoords_2[y2][x2]->w = texcoords_1[y][x]->w;
						heights_2[y2][x2] = heights_1[y][x];
						if (x != new_width-1)
						{
							normals_2[y2][x2+1] = new SphereNormal;
							normals_2[y2][x2+1]->phi = (normals_1[y][x]->phi + normals_1[y][x+1]->phi) * 0.5f;
							normals_2[y2][x2+1]->theta = (normals_1[y][x]->theta + normals_1[y][x+1]->theta) * 0.5f;
							texcoords_2[y2][x2+1] = new UVWCoord;
							texcoords_2[y2][x2+1]->u = (texcoords_1[y][x]->u + texcoords_1[y][x+1]->u) * 0.5f;
							texcoords_2[y2][x2+1]->v = (texcoords_1[y][x]->v + texcoords_1[y][x+1]->v) * 0.5f;
							texcoords_2[y2][x2+1]->w = (texcoords_1[y][x]->w + texcoords_1[y][x+1]->w) * 0.5f;
							heights_2[y2][x2+1] = (heights_1[y][x] + heights_1[y][x+1]) * 0.5f;

							if (y != new_height-1)
							{
								normals_2[y2+1][x2+1] = new SphereNormal;
								normals_2[y2+1][x2+1]->phi = (normals_1[y][x]->phi + normals_1[y+1][x]->phi + normals_1[y][x+1]->phi + normals_1[y][x+1]->phi) * 0.25f;
								normals_2[y2+1][x2+1]->theta = (normals_1[y][x]->theta + normals_1[y+1][x]->theta + normals_1[y][x+1]->theta + normals_1[y][x+1]->theta) * 0.25f;
								texcoords_2[y2+1][x2+1] = new UVWCoord;
								texcoords_2[y2+1][x2+1]->u = (texcoords_1[y][x]->u + texcoords_1[y+1][x]->u + texcoords_1[y][x+1]->u + texcoords_1[y][x+1]->u) * 0.25f;
								texcoords_2[y2+1][x2+1]->v = (texcoords_1[y][x]->v + texcoords_1[y+1][x]->v + texcoords_1[y][x+1]->v + texcoords_1[y][x+1]->v) * 0.25f;
								texcoords_2[y2+1][x2+1]->w = (texcoords_1[y][x]->w + texcoords_1[y+1][x]->w + texcoords_1[y][x+1]->w + texcoords_1[y][x+1]->w) * 0.25f;
								heights_2[y2+1][x2+1] = (heights_1[y][x] + heights_1[y+1][x] + heights_1[y][x+1] + heights_1[y+1][x+1]) * 0.25f;
							}
						}
						if (y != new_height-1)
						{
							normals_2[y2+1][x2] = new SphereNormal;
							normals_2[y2+1][x2]->phi = (normals_1[y][x]->phi + normals_1[y+1][x]->phi) * 0.5f;
							normals_2[y2+1][x2]->theta = (normals_1[y][x]->theta + normals_1[y+1][x]->theta) * 0.5f;
							texcoords_2[y2+1][x2] = new UVWCoord;
							texcoords_2[y2+1][x2]->u = (texcoords_1[y][x]->u + texcoords_1[y+1][x]->u) * 0.5f;
							texcoords_2[y2+1][x2]->v = (texcoords_1[y][x]->v + texcoords_1[y+1][x]->v) * 0.5f;
							texcoords_2[y2+1][x2]->w = (texcoords_1[y][x]->w + texcoords_1[y+1][x]->w) * 0.5f;
							heights_2[y2+1][x2] = (heights_1[y][x] + heights_1[y+1][x]) * 0.5f;
						}
					}
				}

#endif

				mipmap_num++;
			}

			highest_mipmap = mipmap_num-1;
			
			int h = pWorld->height/q_square_size;
			int w = pWorld->width/q_square_size;

			mipmap_levels = new int*[h];
			
			for(int i = 0; i < h; i++)
			{
				mipmap_levels[i] = new int[w];
				for (int j = 0; j < w; j++)
				{
					mipmap_levels[i][j] = 5;
				}
			}
			
		}

		void CreateNormals()
		{
			Utilities::Vector3D temp_normal, normal, vector1, vector2, point_up, point_right, point_down, point_left, point_cur;
			bool up, right, down, left;

#ifdef TERRAIN_HIGH_QUALITY
			HeightMipmaps[0][0].ppNormals = new SphereNormal**[pWorld->height];
#else
			HeightMipmaps[0][0].ppNormals = new XYZCoord**[pWorld->height];
#endif
			for(int y=0;y<pWorld->height;y++)
			{
#ifdef TERRAIN_HIGH_QUALITY
				HeightMipmaps[0][0].ppNormals[y] = new SphereNormal*[pWorld->width];
#else
				HeightMipmaps[0][0].ppNormals[y] = new XYZCoord*[pWorld->width];
#endif
				for(int x=0;x<pWorld->width;x++)
				{

					point_cur.set(0.0, pWorld->ppHeight[y][x], 0.0);

					if (y != 0) // don't do any calculations on the 'up' vector if y = 0, as then it doesn't exist
					{
						point_up.set(0.0, pWorld->ppHeight[y-1][x], -0.125);
						up = true;
					}
					else
					{
						// flag it as 'non-existant'
						up = false;
					}

					if (x != pWorld->width-1)
					{
						point_right.set(0.125, pWorld->ppHeight[y][x+1], 0.0);
						right = true;
					}
					else
					{
						right = false;
					}

					if (y != pWorld->height-1)
					{
						point_down.set(0.0, pWorld->ppHeight[y+1][x], 0.125);
						down = true;
					}
					else
					{
						down = false;
					}

					if (x != 0)
					{
						point_left.set(-0.125, pWorld->ppHeight[y][x-1], 0.0);
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

#ifdef TERRAIN_HIGH_QUALITY
					HeightMipmaps[0][0].ppNormals[y][x] = new SphereNormal;
					
					HeightMipmaps[0][0].ppNormals[y][x]->phi = acos(normal.z);
					float S = sqrt(normal.x * normal.x + normal.y * normal.y);
					if (normal.x < 0)
					{
						HeightMipmaps[0][0].ppNormals[y][x]->theta = (float)PI - asin(normal.y / S);
					}
					else
					{
						HeightMipmaps[0][0].ppNormals[y][x]->theta = asin(normal.y / S);
					}
#else
					HeightMipmaps[0][0].ppNormals[y][x] = new XYZCoord;
					
					HeightMipmaps[0][0].ppNormals[y][x]->x = normal.x;
					HeightMipmaps[0][0].ppNormals[y][x]->y = normal.y;
					HeightMipmaps[0][0].ppNormals[y][x]->z = normal.z;
#endif

				}
			}
		}

		void CreateTexCoords()
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

		}

		void CalculateSteepness()
		{
			int n, steepness;
			float val = 0;
			int start_x, start_y;
			int end_x, end_y;
			pWorld->ppSteepness = new int*[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{

				pWorld->ppSteepness[y] = new int[pWorld->width];

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
							val += fabs(pWorld->ppHeight[y2][x2] - pWorld->ppHeight[y][x]);
							n++;
						}
					}

					val /= (float) n-1; // to compensate for the middle point, which will always be 0

					steepness = (int) (val * 1000);

					pWorld->ppSteepness[y][x] = steepness;

				}

			}
		}

		void InitFog()
		{
			cout << "Initializing fog of war..." << endl;

			int** NumUnitsSeeingSquare;
			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				NumUnitsSeeingSquare = new int*[pWorld->height];
				for (int y = 0; y < pWorld->height; y++)
				{
					NumUnitsSeeingSquare[y] = new int[pWorld->width];
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

			int** NumLightsOnSquare;
			NumLightsOnSquare = new int*[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{
				NumLightsOnSquare[y] = new int[pWorld->width];
				for (int x = 0; x < pWorld->width; x++)
				{
					NumLightsOnSquare[y][x] = 0;
				}
			}
			pWorld->NumLightsOnSquare = NumLightsOnSquare;
		}
		
		bool** big_square_has_water;
		
		int levelmap_width, levelmap_height;

		int water_cur_front = 1, water_cur_back = 0, water_interpolated = 2;

		XYZCoord*** water_normals;

		void InitWater()
		{
			int numwater = 0, numbigwater = 0;
			HeightMipmaps[0][0].ppSquareHasWater = new bool*[pWorld->height];
			pWorld->ppWater[0] = new float*[pWorld->height];
			pWorld->ppWater[1] = new float*[pWorld->height];
			pWorld->ppWater[2] = new float*[pWorld->height];
			water_normals = new XYZCoord**[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{

				HeightMipmaps[0][0].ppSquareHasWater[y] = new bool[pWorld->width];
				pWorld->ppWater[0][y] = new float[pWorld->width];
				pWorld->ppWater[1][y] = new float[pWorld->width];
				pWorld->ppWater[2][y] = new float[pWorld->width];
				water_normals[y] = new XYZCoord*[pWorld->width];

				for (int x = 0; x < pWorld->width; x++)
				{

					water_normals[y][x] = new XYZCoord;
					HeightMipmaps[0][0].ppSquareHasWater[y][x] = (waterLevel + waterHeight / 2 >= pWorld->ppHeight[y][x]);
					numwater += HeightMipmaps[0][0].ppSquareHasWater[y][x];

					pWorld->ppWater[0][y][x] = 0.0f;
					pWorld->ppWater[1][y][x] = 0.0f;

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
							if (HeightMipmaps[0][0].ppSquareHasWater[y*q_square_size+y2][x*q_square_size+x2])
							{
								cout << x*q_square_size+x2 << " " << y*q_square_size+y2 << endl;
								big_square_has_water[y][x] = true;
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
			for (int i = 0; i < 120; i++)
			{
				CalculateWater();
			}
		}

		float GetWaterDepth(int x, int y)
		{
			float res = waterLevel + pWorld->ppWater[water_cur_front][y][x] - pWorld->ppHeight[y][x];
			if (res < 0)
			{
				res = 0;
			}
			return res;
		}

		void CalculateWater()
		{
			int height, width;
			float** ppWater[2];
			ppWater[0] = pWorld->ppWater[0];
			ppWater[1] = pWorld->ppWater[1];
			height = pWorld->height-1;
			width = pWorld->width-1;

			for (int i = 0; i < 1000; i++)
			{
				int y = rand() % (height-1) + 1;
				int x = rand() % (width-1) + 1;
				if (waterLevel > pWorld->ppHeight[y][x])
				{
					ppWater[water_cur_front][y][x] += (float) ((double) rand() / RAND_MAX - 0.5) * (waterLevel - pWorld->ppHeight[y][x]) * 0.1f;
				}
			}

			for (int y = 1; y < height; y++)
			{
				for (int x = 1; x < width; x++)
				{
					if (HeightMipmaps[0][0].ppSquareHasWater[y][x])
					{
						ppWater[water_cur_front][y][x] = (ppWater[water_cur_back][y-1][x] +
										  ppWater[water_cur_back][y+1][x] +
										  ppWater[water_cur_back][y][x-1] +
										  ppWater[water_cur_back][y][x+1]) / 2 -
										  ppWater[water_cur_front][y][x];

						ppWater[water_cur_front][y][x] *= 0.95f;
/*						if (waterLevel < pWorld->ppHeight[y][x])
						{
							if (waterLevel + ppWater[water_cur_front][y][x] < pWorld->ppHeight[y][x])
							{
								ppWater[water_cur_front][y][x] = 0.0;
							}
						}
						else
						{*/
							if (waterLevel + ppWater[water_cur_front][y][x] < pWorld->ppHeight[y][x])
							{
								ppWater[water_cur_front][y][x] = pWorld->ppHeight[y][x] - waterLevel + 0.01f;
							}
//						}
						if (waterLevel + 0.01f < pWorld->ppHeight[y][x])
						{
							ppWater[water_cur_front][y][x] = 0.0f;
						}
/*						if (waterLevel + ppWater[water_cur_front][y-1][x] < pWorld->ppHeight[y-1][x] &&
						    waterLevel + ppWater[water_cur_front][y+1][x] < pWorld->ppHeight[y+1][x] &&
						    waterLevel + ppWater[water_cur_front][y][x-1] < pWorld->ppHeight[y][x-1] &&
						    waterLevel + ppWater[water_cur_front][y][x+1] < pWorld->ppHeight[y][x+1])
						{
							ppWater[water_cur_front][y][x] = 0.0;
						}*/

					}
				}
			}
			water_cur_front ^= 1;
			water_cur_back ^= 1;
		}

		float **q_levels;
		float **new_q_levels;
		
		// array storing the start and end of displayed big squares, for every column of big squares
		int *is_visible[2];
		
		int InternalWorldGetNextValue(std::ifstream& file)
		{
			char buffer[16];
			int  i = 0;
						
			while (file.peek() < '0' || file.peek() > '9')
				file.get();
			
			while (file.peek() >= '0' && file.peek() < '9' && i < 16)
				buffer[i++] = (char) file.get();
			buffer[i] = '\0';
			
			return atoi(buffer);
		}
		
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
			
				width = InternalWorldGetNextValue(file);
				if (width == 0)
				{
					cout << "Failed!!! Invalid width!" << endl;
				}

				height = InternalWorldGetNextValue(file);
				if (height == 0)
				{
					cout << "Failed!!! Invalid height!" << endl;
				}
				
				std::cout << "Width: " << width << ", height: " << height << std::endl;
				
				InternalWorldGetNextValue(file); // << wierd integer
			}
			else
			{
				file >> width;
				
				// sök till nästa tal...
				while (file.peek() < '0' || file.peek() > '9')
					file.get();

				file >> height;
			}

//			pWorld = new World;

			pWorld->height = height;
			pWorld->width = width;
			terrainOffsetY = float(height / 16);
			terrainOffsetX = float(width / 16);
			
			pWorld->ppHeight = new float*[height];
			
			// Läs från fil
			for (int y = 0; y < height; y++)
			{
				// scanline per scanline
				pWorld->ppHeight[y] = new float[width];
				for (int x = 0; x < width; x++)
				{

					while (file.peek() < '0' || file.peek() > '9')
					file.get();
			
					file >> temp;
					pWorld->ppHeight[y][x] = ((float) temp / 255-0.5f)*terrainHeight;
		
				}
			}

			file.close();
			
			q_levels = new float*[height/q_square_size];
			big_square_has_water = new bool*[height/q_square_size];
			new_q_levels = new float*[height/q_square_size];
			for (int y=0;y<height/q_square_size;y++)
			{
				q_levels[y] = new float[width/q_square_size];
				big_square_has_water[y] = new bool[width/q_square_size];
				new_q_levels[y] = new float[width/q_square_size];
				for (int x=0;x<height/q_square_size;x++)
				{
					q_levels[y][x] = -1;
					big_square_has_water[y][x] = false;
				}
			}

			levelmap_height = height/q_square_size;
			levelmap_width = width/q_square_size;

			is_visible[0] = new int[levelmap_height];
			is_visible[1] = new int[levelmap_height];

			if (!Game::Rules::noGraphics)
			{

				cout << "Calculating normals for heightmap..." <<  endl;
				
				CreateNormals();

				cout << "Calculating texture coordinates for heightmap..." <<  endl;

				CreateTexCoords();
				
				cout << "Initializing water..." << endl;

				InitWater();

				cout << "Calculating mipmaps for heightmap..." <<  endl;
				
				CreateMipmaps();

			}
			else
			{
				HeightMipmaps[0][0].ppHeights = pWorld->ppHeight;
			}

			cout << "Calculating steepness..." << endl;

			CalculateSteepness();

			InitFog();
		
			InitLight();

			pppElements = new Unit**[pWorld->height];
			for (int y = 0; y < pWorld->height; y++)
			{
				pppElements[y] = new Unit*[pWorld->width];
				for (int x = 0; x < pWorld->width; x++)
				{
					pppElements[y][x] = 0;
				}
			}	

			return SUCCESS;
		}


		// Gets the x, y and z coords for a square, at the highest quality
		Utilities::Vector3D GetSquareCoord(float x, float y)
		{
			return Utilities::Vector3D(x / 8 - terrainOffsetX, pWorld->ppHeight[(int) floor(y)][(int) floor(x)], y / 8 - terrainOffsetY);
		}

#ifdef TERRAIN_HIGH_QUALITY
		// get the terrain height at the specified location with the correct quality level
/*		float GetTerrainHeight(int x, int y, float level)
		{
			float** heights;
			int bsx, bsy;
			int ssx, ssy;
			int mx, my;
			float xmix, ymix;
			int level;

			bsx = (int) floor(x / 32);
			bsy = (int) floor(y / 32);

			if (x == pWorld->width-1) // special cases where x or y is at the end of absolute 'upper' edge of the map.
				bsx -= 1;         // ie for example 256 on a 257x257 map
			if (y == pWorld->height-1)
				bsy -= 1;

			level = mipmap_levels[bsy][bsx];

			heights = HeightMipmaps[0][level].ppHeights;

			ssx = (int) floor((x - bsx * 32) / 32 * (32 >> level));
			ssy = (int) floor((y - bsy * 32) / 32 * (32 >> level));

			xmix = (x - bsx * 32 - (ssx << level)) / (1 << level);
			ymix = (y - bsy * 32 - (ssy << level)) / (1 << level);

			mx = bsx * (32 >> level) + (ssx << level);
			my = bsy * (32 >> level) + (ssy << level);

			return (heights[my][mx] * (1 - xmix) + heights[my][mx+1] * xmix) * (1 - ymix) +
			       ((heights[my+1][mx] * (1 - xmix) + heights[my+1][mx+1] * xmix) * ymix);
		}*/
#endif
#ifdef TERRAIN_HIGH_QUALITY
		void GetTerrainNormal(int x, int y, int higher_level, float mix, SphereNormal& normal)
		{
			SphereNormal ***normals_1, ***normals_2;

			normals_1 = HeightMipmaps[0][higher_level].ppNormals;
			normals_2 = HeightMipmaps[1][higher_level+1].ppNormals;

			normal.phi = normals_1[y][x]->phi + (normals_2[y][x]->phi - normals_1[y][x]->phi) * mix;
			normal.theta = normals_1[y][x]->theta + (normals_2[y][x]->theta - normals_1[y][x]->theta) * mix;
		}
		
		float GetTerrainHeight(int x, int y, int higher_level, float mix)
		{
			float **heights_1, **heights_2;
			
			heights_1 = HeightMipmaps[0][higher_level].ppHeights;
			heights_2 = HeightMipmaps[1][higher_level+1].ppHeights;

			return heights_1[y][x] + (heights_2[y][x] - heights_1[y][x]) * mix;
		}
		
		void GetTerrainTexCoord(int x, int y, int higher_level, float mix, UVWCoord& texcoord)
		{
			UVWCoord ***texcoords_1, ***texcoords_2;
			
			texcoords_1 = HeightMipmaps[0][higher_level].ppTexCoords;
			texcoords_2 = HeightMipmaps[1][higher_level+1].ppTexCoords;

			texcoord.u = texcoords_1[y][x]->u + (texcoords_2[y][x]->u - texcoords_1[y][x]->u) * mix;
			texcoord.v = texcoords_1[y][x]->v + (texcoords_2[y][x]->v - texcoords_1[y][x]->v) * mix;
		}
#endif
		
		// get the terrain height at the specified location, interpolated and all and at the current mipmap level
		float GetTerrainHeight(float x, float y)
		{
			float** heights;
			int bsx, bsy;
			int ssx, ssy;
			int mx, my;
			float xmix, ymix;
			int level;

			bsx = (int) x / 32;
			bsy = (int) y / 32;

			if (x >= pWorld->width-1)      // special cases where x or y is at the end of absolute 'upper' edge of the map.
				bsx -= 1; // ie for example 256 on a 257x257 map
			if (y >= pWorld->height-1)
				bsy -= 1;

#ifdef TERRAIN_HIGH_QUALITY
			level = (int) q_levels[bsy][bsx];
			if (level < 0) level = 0;
#else
			level = mipmap_levels[bsy][bsx];
#endif

			heights = HeightMipmaps[0][level].ppHeights;

			ssx = (int) floor((x - (float) bsx * 32) / float(1 << level));
			ssy = (int) floor((y - (float) bsy * 32) / float(1 << level));

			xmix = (x - float(bsx * 32 + (ssx << level))) / float(1 << level);
			ymix = (y - float(bsy * 32 + (ssy << level))) / float(1 << level);

			mx = bsx * (32 >> level) + ssx;
			my = bsy * (32 >> level) + ssy;

			if (mx >= (pWorld->width-1)>>level)
			{
				xmix = 1.0;
				mx = ((pWorld->width-1)>>level)-1;
			}

			if (my >= (pWorld->height-1)>>level)
			{
				ymix = 1.0;
				my = ((pWorld->height-1)>>level)-1;
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

		Utilities::Vector3D GetTerrainNormal(int x, int y)
		{
#ifdef TERRAIN_HIGH_QUALITY
			SphereNormal*** normals;
#else
			XYZCoord*** normals;
#endif
			int bsx, bsy;
			int level;

			bsx = (int) floor((float) x / 32.0f);
			bsy = (int) floor((float) y / 32.0f);

			if (x == pWorld->width-1)
				bsx -= 1;
			if (y == pWorld->height-1)
				bsy -= 1;

#ifdef TERRAIN_HIGH_QUALITY
			level = (int) q_levels[bsy][bsx];
			if (level < 0) level = 0;
#else
			level = mipmap_levels[bsy][bsx];
#endif

			normals = HeightMipmaps[0][level].ppNormals;

			return Utilities::Vector3D(normals[y][x]);
		}

		// get the terrain normal at the specified location, interpolated and all and at the current mipmap level
		Utilities::Vector3D GetTerrainNormal(float x, float y)
		{
#ifdef TERRAIN_HIGH_QUALITY
			SphereNormal*** normals;
#else
			XYZCoord*** normals;
#endif
			int bsx, bsy;
			int ssx, ssy;
			int mx, my;
			float xmix, ymix;
			int level;

			bsx = (int) floor(x / 32);
			bsy = (int) floor(y / 32);

			if (x >= pWorld->width-1)
				bsx -= 1;
			if (y >= pWorld->height-1)
				bsy -= 1;

#ifdef TERRAIN_HIGH_QUALITY
			level = (int) q_levels[bsy][bsx];
			if (level < 0) level = 0;
#else
			level = mipmap_levels[bsy][bsx];
#endif

			normals = HeightMipmaps[0][level].ppNormals;

			ssx = (int) floor((x - (float) bsx * 32) / float(1 << level));
			ssy = (int) floor((y - (float) bsy * 32) / float(1 << level));

			xmix = (x - float(bsx * 32 + (ssx << level))) / float(1 << level);
			ymix = (y - float(bsy * 32 + (ssy << level))) / float(1 << level);

			mx = bsx * (32 >> level) + ssx;
			my = bsy * (32 >> level) + ssy;

			if (mx >= (pWorld->width-1)>>level)
			{
				xmix = 1.0;
				mx = ((pWorld->width-1)>>level)-1;
			}

			if (my >= (pWorld->height-1)>>level)
			{
				ymix = 1.0;
				my = ((pWorld->height-1)>>level)-1;
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
			       (Utilities::Vector3D(normals[my+1][mx]) * (1 - xmix) + Utilities::Vector3D(normals[my+1][mx+1]) * xmix) * ymix;
		}

		// get the terrain height at the specified location, interpolated and all and at the highest mipmap level
		float GetTerrainHeightHighestLevel(float x, float y)
		{
			float** heights;
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

			heights = HeightMipmaps[0][0].ppHeights;

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

		// get the terrain height at the specified location
		float GetTerrainHeightHighestLevel(int x, int y)
		{
			return HeightMipmaps[0][0].ppHeights[y][x];
		}

		Utilities::Vector3D GetTerrainCoord(float x, float y)
		{
			return Utilities::Vector3D((x * 0.125f) - terrainOffsetX, GetTerrainHeight(x, y), (y * 0.125f) - terrainOffsetY);
		}

		Utilities::Vector3D GetTerrainCoordHighestLevel(float x, float y)
		{
			return Utilities::Vector3D((x * 0.125f) - terrainOffsetX, GetTerrainHeightHighestLevel(x, y), (y * 0.125f) - terrainOffsetY);
		}

		Dimension::Position GetPosition(Utilities::Vector3D* v)
		{
			return Dimension::Position(8.0f * (v->x + terrainOffsetX), 8.0f * (v->z + terrainOffsetY));
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

		// Interpolates source[] from size source_size to dest_size, and puts the
		// results into dest[]. Does no 'mipmapping' or simlar down-scalin algorithm;
		// was made for scaling up
		void interpolate_float_array(float source[], int source_size, float dest[], int dest_size)
		{
			float mix, ratio, ratio_inv;
			float x2;
			int x2_i;
			// ratio between source size and dest size
			ratio = (float) (source_size - 1) / (float) (dest_size - 1);
			ratio_inv = 1 / ratio;
			for (int x = 0; x < dest_size; x++)
			{
				x2 = (float) x * ratio;
				x2_i = (int) floor(x2);
				if (x2_i == source_size)
				{
					x2_i = source_size-1;
					x2 = (float) source_size-1;
				}
				// mix: the weight of the last vs the next value.
				// 0.0 means pure source [x2_i], 1.0 means pure source[x2_i+1]
				mix = x2 - (float) x2_i;
				dest[x] = source[x2_i] * (1 - mix) + source[x2_i+1] * mix;
			}
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

		GLuint terraintexture;

		// buffers used for interpolating at the 'seams'
		float height_dest_buffer[q_square_size+1], height_src_buffer[q_square_size+1];
		float nx_dest_buffer[q_square_size+1], ny_dest_buffer[q_square_size+1], nz_dest_buffer[q_square_size+1];
		float nx_src_buffer[q_square_size+1], ny_src_buffer[q_square_size+1], nz_src_buffer[q_square_size+1];
		float tu_dest_buffer[q_square_size+1], tv_dest_buffer[q_square_size+1], tw_dest_buffer[q_square_size+1];
		float tu_src_buffer[q_square_size+1], tv_src_buffer[q_square_size+1], tw_src_buffer[q_square_size+1];

		// current position of the terrain
		Utilities::Vector3D cur_pos(0.0f, -3.0f, -13.0f);
			
		int DrawTerrain()
		{
			/*
			static int direction = 0;
			static float roty = 0.0f;
			*/
			float q_level, ratio;

			// size of a small square in the 'world'
			float world_square_size;
			// real x1, y1, x2 and y2 coords of a small square
			float wx1, wy1, wx2, wy2;
			// base index coords of the big squares that the landscape is divided into
			int mx, my;
			// index coords of the small squares -- upper x, upper y, lower x, lower y
			int mx1, my1, mx2, my2;
			// the current mipmap level and the size a big square has at that quality level
			int scaled_square_size, mipmap_level;
			int h = pWorld->height/q_square_size;
			int w = pWorld->width/q_square_size;

#ifdef TERRAIN_HIGH_QUALITY
			float qualities[2][2];
			float qualities_interp[33][33];
#endif

			bool is_seen, is_lighted;

			int** NumUnitsSeeingSquare = Dimension::currentPlayerView->NumUnitsSeeingSquare;

//			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
			glBindTexture(GL_TEXTURE_2D, terraintexture);

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, terrainMaterialAmbientDiffuse[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, terrainMaterialAmbientDiffuse[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, terrainMaterialSpecular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, terrainMaterialEmission);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, terrainMaterialShininess);

			glEnable(GL_COLOR_MATERIAL);

			for(int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					mipmap_levels[i][j] = 5;
				}
			}
			
			// array of heights, normals and texcoords for the current mipmap level
			float** heights;

#ifdef TERRAIN_HIGH_QUALITY
			SphereNormal*** normals;
#else
			XYZCoord*** normals;
			XYZCoord* normal;
			UVWCoord* texcoord;
#endif
			UVWCoord*** texcoords;
			
			Utilities::Vector3D pos_vector_near, pos_vector_far, cur_mod_pos, void_pos;
			
			float fmx_low_v[2][64], fmy_low_v[2][64], fmx_high_v[2][64], fmy_high_v[2][64];

			float fmx_low_h[64][2], fmy_low_h[64][2], fmx_high_h[64][2], fmy_high_h[64][2];

			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				is_visible[0][y] = 65536;
				is_visible[1][y] = -65536;
			}

			/*
			glTranslatef(cur_pos.x, cur_pos.y, cur_pos.z);
			glRotatef(roty, 0.0f, 1.0f, 0.0f);

			roty += 0.1;
			*/
			
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

			/*
			if (direction)
				cur_pos.z += 0.1;
			else
				cur_pos.z -= 0.1;

			if (cur_pos.z > 6.0)
				direction = 0;

			if (cur_pos.z < -20.0)
				direction = 1;
			*/

			// get the map mesh position of the viewer by getting the near
			// viewing plane in the middle of the screen
			Utilities::WindowCoordToVector((Window::windowWidth-1) * 0.5, (Window::windowHeight-1) * 0.5, cur_mod_pos, void_pos);

			// calculate relative quality levels
			
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				for (int x=0;x<pWorld->width/q_square_size;x++)
				//for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{
					// base quality levels on distance from viewer
					new_q_levels[y][x] = cur_mod_pos.distance(GetSquareCoord(((float) x + 0.5f) * q_square_size, ((float) y + 0.5f) * q_square_size)) * 0.01f;
				}
			}

#if 0
			for (int y=0;y<h;y++)
			{
				for (int x=0;x<w;x++)
				{
					if (x > 0)
					{
						if (floor(new_q_levels[y][x] - 0.0001) > floor(new_q_levels[y][x-1]))
							new_q_levels[y][x] = floor(new_q_levels[y][x-1])+1;
						if (floor(new_q_levels[y][x]) < floor(new_q_levels[y][x-1] - 0.0001))
							new_q_levels[y][x] = floor(new_q_levels[y][x-1]);
					}
					if (y > 0)
					{
						if (floor(new_q_levels[y][x] - 0.0001) > floor(new_q_levels[y-1][x]))
							new_q_levels[y][x] = floor(new_q_levels[y-1][x])+1;
						if (floor(new_q_levels[y][x]) < floor(new_q_levels[y-1][x] - 0.0001))
							new_q_levels[y][x] = floor(new_q_levels[y-1][x]);
					}
					if (y > 0 && x > 0)
					{
						if (floor(new_q_levels[y][x] - 0.0001) > floor(new_q_levels[y-1][x-1]))
							new_q_levels[y][x] = floor(new_q_levels[y-1][x-1])+1;
						if (floor(new_q_levels[y][x]) < floor(new_q_levels[y-1][x-1] - 0.0001))
							new_q_levels[y][x] = floor(new_q_levels[y-1][x-1]);
					}
				}
			}
#endif
			
			q_level = 0.0;
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{
					// calculate overall quality (higher quality gets a
					// higher value, for ease of computation; when
					// distance is used as a measure of quality, a lower
					// value means higher quality, also, the lower the
					// mipmap level, the higher the quality) as in number
					// of quads that will need to be rendered
					q_level += pow((float) ((q_square_size >> (int) new_q_levels[y][x]) + 1), 2);
				}
			}

/*#ifdef TERRAIN_HIGH_QUALITY
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				for (int x=0;x<pWorld->width/q_square_size;x++)
				{
					bool passed = false;
					if (x >= is_visible[0][y] && x <= is_visible[1][y])
					{
						passed = true;
					}
					if (x-1 >= is_visible[0][y] && x-1 <= is_visible[1][y])
					{
						passed = true;
					}
					if (y != 0 && (x >= is_visible[0][y-1] && x <= is_visible[1][y-1]))
					{
						passed = true;
					}
					if (y != 0 && (x-1 >= is_visible[0][y-1] && x-1 <= is_visible[1][y-1]))
					{
						passed = true;
					}
					if (!passed)
					{
						q_levels[y][x] = -1;
					}
				}
			}
#else
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				for (int x=0;x<pWorld->width/q_square_size;x++)
				{
					if (!(x >= is_visible[0][y] || x <= is_visible[1][y]))
					{
						q_levels[y][x] = -1;
					}
				}
			}
#endif */

			int loops = 0;

			// calculate the ratio between the calculated quality levels and the wanted quality level
			while (q_level > quality)
			{

				ratio = q_level / quality;

				for (int y=0;y<pWorld->height/q_square_size;y++)
				{
					for (int x=0;x<pWorld->width/q_square_size;x++)
					//for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
					{
						// increase the quality level depending on the ratio
						new_q_levels[y][x] *= ratio;

#ifdef TERRAIN_HIGH_QUALITY
						q_levels[y][x] = new_q_levels[y][x];
#else
						if (!(x >= is_visible[0][y] && x <= is_visible[1][y]))
						{
							q_levels[y][x] = new_q_levels[y][x];
						}
						else
						{
							q_levels[y][x] = new_q_levels[y][x] * 0.2 + q_levels[y][x] * 0.8;
						}
#endif
						
						if (q_levels[y][x] > highest_mipmap)
						{
							q_levels[y][x] = (float) highest_mipmap;
						}
						if (q_levels[y][x] > 5)
						{
							q_levels[y][x] = 5;
						}

						// convert into actual mipmap levels (and check so it's not too high)
						mipmap_levels[y][x] = (int) floor(q_levels[y][x] + 0.5);
						if (mipmap_levels[y][x] > highest_mipmap)
						{
							mipmap_levels[y][x] = highest_mipmap;
						}
						if (mipmap_levels[y][x] > 5)
						{
							mipmap_levels[y][x] = 5;
						}
					}
				}
			
				q_level = 0.0;
				for (int y=0;y<pWorld->height/q_square_size;y++)
				{
					for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
					{
						// calculate overall quality (higher quality gets a
						// higher value, for ease of computation; when
						// distance is used as a measure of quality, a lower
						// value means higher quality, also, the lower the
						// mipmap level, the higher the quality) as in number
						// of quads that will need to be rendered
						q_level += pow((float) ((q_square_size >> (int) new_q_levels[y][x]) + 1), 2);
					}
				}
				
				loops++;
				
				if (loops > 100)
					break;

			}
				
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				for (int x=0;x<pWorld->width/q_square_size;x++)
				{

#ifdef TERRAIN_HIGH_QUALITY
					q_levels[y][x] = new_q_levels[y][x];
#else
					if (!(x >= is_visible[0][y] && x <= is_visible[1][y]))
					{
						q_levels[y][x] = new_q_levels[y][x];
					}
					else
					{
						q_levels[y][x] = new_q_levels[y][x] * 0.2 + q_levels[y][x] * 0.8;
					}
#endif
					
					if (q_levels[y][x] > highest_mipmap)
					{
						q_levels[y][x] = (float) highest_mipmap;
					}
					if (q_levels[y][x] > 5)
					{
						q_levels[y][x] = 5;
					}

					// convert into actual mipmap levels (and check so it's not too high)
					mipmap_levels[y][x] = (int) floor(q_levels[y][x] + 0.5);
					if (mipmap_levels[y][x] > highest_mipmap)
					{
						mipmap_levels[y][x] = (int) highest_mipmap;
					}
					if (mipmap_levels[y][x] > 5)
					{
						mipmap_levels[y][x] = 5;
					}
				}
			}

#ifdef TERRAIN_HIGH_QUALITY
			for (int y=0;y<h;y++)
			{
				for (int x=0;x<w;x++)
				{
					if (y > 0 && x > 0)
					{
						if (floor(q_levels[y][x] - 0.0001f) > floor(q_levels[y-1][x-1]))
							q_levels[y][x] = floor(q_levels[y-1][x-1])+1;
						if (floor(q_levels[y][x]) < floor(q_levels[y-1][x-1] - 0.0001f))
							q_levels[y][x] = floor(q_levels[y-1][x-1] - 0.0001f);
					}
					if (y > 0)
					{
						if (floor(q_levels[y][x] - 0.0001f) > floor(q_levels[y-1][x]))
							q_levels[y][x] = floor(q_levels[y-1][x])+1;
						if (floor(q_levels[y][x]) < floor(q_levels[y-1][x] - 0.0001f))
							q_levels[y][x] = floor(q_levels[y-1][x] - 0.0001f);
					}
					if (x > 0)
					{
						if (floor(q_levels[y][x] - 0.0001f) > floor(q_levels[y][x-1]))
							q_levels[y][x] = floor(q_levels[y][x-1])+1;
						if (floor(q_levels[y][x]) < floor(q_levels[y][x-1] - 0.0001f))
							q_levels[y][x] = floor(q_levels[y][x-1] - 0.0001f);
					}
				}
			}
#endif
			float calc_heights[33][33];
			XYZCoord calc_normals[33][33];
			RGBA calc_colours[33][33];
			UVWCoord calc_texcoords[33][33];

			// draw each big square
			for (int y=0;y<pWorld->height/q_square_size;y++)
			{
				//for (int x=0;x<pWorld->width/q_square_size;x++)
				for (int x=is_visible[0][y];x<=is_visible[1][y];x++)
				{

#ifdef TERRAIN_HIGH_QUALITY
					qualities[0][0] = q_levels[y][x];

					if (x != w-1)
						qualities[0][1] = q_levels[y][x+1];
					else
						qualities[0][1] = q_levels[y][x];

					if (y != h-1)
						qualities[1][0] = q_levels[y+1][x];
					else
						qualities[1][0] = q_levels[y][x];

					if (x != w-1 && y != h-1)
						qualities[1][1] = q_levels[y+1][x+1];
					else if (x != w-1)
						qualities[1][1] = q_levels[y][x+1];
					else if (y != h-1)
						qualities[1][1] = q_levels[y+1][x];
					else
						qualities[1][1] = q_levels[y][x];

					float highest_quality = qualities[0][0];
					if (qualities[0][1] < highest_quality)  highest_quality = qualities[0][1];
					if (qualities[1][0] < highest_quality)  highest_quality = qualities[1][0];
					if (qualities[1][1] < highest_quality)  highest_quality = qualities[1][1];
					
					int base_quality = (int) highest_quality;
					if (base_quality < 0) base_quality = 0;
					
					for (int i = 0; i < 2; i++)
						for (int j = 0; j < 2; j++)
							qualities[i][j] -= (float) base_quality;
					mipmap_level = base_quality;
#else
					mipmap_level = mipmap_levels[y][x];
					if (mipmap_level < 0)
						mipmap_level = 0;
#endif

					// calculate the number of small squares in a big square at this mipmap level
					scaled_square_size = q_square_size >> mipmap_level;

					// calculate real size of a small square
					world_square_size = (float) 16 / (float) (128 >> mipmap_level);

					// calculate the base index x and y coords into the height, normal and texcoord arrays at the current mipmap level
					mx = x * (q_square_size >> mipmap_level);
					my = y * (q_square_size >> mipmap_level);

					// get the heights, normals and texcoords for the current mipmap level
					heights = HeightMipmaps[0][mipmap_level].ppHeights;
					normals = HeightMipmaps[0][mipmap_level].ppNormals;
					texcoords = HeightMipmaps[0][mipmap_level].ppTexCoords;

#ifdef TERRAIN_HIGH_QUALITY

					float ssc = (float) scaled_square_size;
					float one_step = (1.00f / ssc);
					float low_x_q = qualities[0][0];// + (qualities[1][0] - qualities[0][0]) * interp_consts[y];
					float high_x_q = qualities[0][1];// + (qualities[1][1] - qualities[0][1]) * interp_consts[y];
					float low_x_q_diff = (qualities[1][0] - qualities[0][0]) * one_step;
					float high_x_q_diff = (qualities[1][1] - qualities[0][1]) * one_step;
					for (int ny = 0; ny <= scaled_square_size; ny++)
					{
						float q = low_x_q;
						float q_diff = (high_x_q - low_x_q) * one_step;
						for (int nx = 0; nx <= scaled_square_size; nx++)
						{
							qualities_interp[ny][nx] = q;
							q += q_diff;
						}
						low_x_q += low_x_q_diff;
						high_x_q += high_x_q_diff;
					}
#endif

#ifdef TERRAIN_HIGH_QUALITY
#if 1
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							float q = qualities_interp[y2][x2];
							calc_heights[y2][x2] = GetTerrainHeight(mx1, my1, base_quality, q);

							is_seen = NumUnitsSeeingSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							is_lighted = pWorld->NumLightsOnSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							
							calc_colours[y2][x2].r = terrainMaterialAmbientDiffuse[is_lighted][is_seen][0];
							calc_colours[y2][x2].g = terrainMaterialAmbientDiffuse[is_lighted][is_seen][1];
							calc_colours[y2][x2].b = terrainMaterialAmbientDiffuse[is_lighted][is_seen][2];
							calc_colours[y2][x2].a = terrainMaterialAmbientDiffuse[is_lighted][is_seen][3];
							
							SphereNormal temp;
							GetTerrainNormal(mx1, my1, base_quality, q, temp);
							
							float S = sin(temp.phi);
							calc_normals[y2][x2].x = S * cos(temp.theta);
							calc_normals[y2][x2].y = S * sin(temp.theta);
							calc_normals[y2][x2].z = cos(temp.phi);

							GetTerrainTexCoord(mx1, my1, base_quality, q, calc_texcoords[y2][x2]);
						}
					}
#else
					float **heights_2 = HeightMipmaps[1][mipmap_level+1].ppHeights;
					SphereNormal ***normals_2 = HeightMipmaps[1][mipmap_level+1].ppNormals;
					UVWCoord ***texcoords_2 = HeightMipmaps[1][mipmap_level+1].ppTexCoords;
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							is_seen = NumUnitsSeeingSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							is_lighted = pWorld->NumLightsOnSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							
							calc_colours[y2][x2].r = terrainMaterialAmbientDiffuse[is_lighted][is_seen][0];
							calc_colours[y2][x2].g = terrainMaterialAmbientDiffuse[is_lighted][is_seen][1];
							calc_colours[y2][x2].b = terrainMaterialAmbientDiffuse[is_lighted][is_seen][2];
							calc_colours[y2][x2].a = terrainMaterialAmbientDiffuse[is_lighted][is_seen][3];
						}
					}
					
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							float q = qualities_interp[y2][x2];
							calc_heights[y2][x2] = (heights[my1][mx1] + (heights_2[my1][mx1] - heights[my1][mx1]) * q);

						}
					}
					
					SphereNormal temp;
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							float q = qualities_interp[y2][x2];
							temp.phi = (normals[my1][mx1]->phi + (normals_2[my1][mx1]->phi - normals[my1][mx1]->phi) * q);
							temp.theta = (normals[my1][mx1]->theta + (normals_2[my1][mx1]->theta - normals[my1][mx1]->theta) * q);
							
							float S = sin(temp.phi);
							calc_normals[y2][x2].x = S * cos(temp.theta);
							calc_normals[y2][x2].y = S * sin(temp.theta);
							calc_normals[y2][x2].z = cos(temp.phi);

						}
					}
					
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							float q = qualities_interp[y2][x2];
							calc_texcoords[y2][x2].u = texcoords[my1][mx1]->u + (texcoords_2[my1][mx1]->u - texcoords[my1][mx1]->u) * q;
						}
					}
					
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						my1 = my + y2;

						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							
							float q = qualities_interp[y2][x2];
							calc_texcoords[y2][x2].v = texcoords[my1][mx1]->v + (texcoords_2[my1][mx1]->v - texcoords[my1][mx1]->v) * q;
						}
					}
#endif
							
#else
					for (int y2=0;y2<=scaled_square_size;y2++)
					{
						mx1 = mx + 0;
						my1 = my + y2;
						my2 = my + y2 + 1;
						for (int x2=0;x2<=scaled_square_size;x2++)
						{
							mx1 = mx + x2;
							my1 = my + y2;
							mx2 = mx + x2 + 1;
							my2 = my + y2 + 1;

							calc_heights[y2][x2] = heights[my1][mx1];

							is_seen = NumUnitsSeeingSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							is_lighted = pWorld->NumLightsOnSquare[my1 << mipmap_level][mx1 << mipmap_level] > 0 ? 1 : 0;
							
							calc_colours[y2][x2].r = terrainMaterialAmbientDiffuse[is_lighted][is_seen][0];
							calc_colours[y2][x2].g = terrainMaterialAmbientDiffuse[is_lighted][is_seen][1];
							calc_colours[y2][x2].b = terrainMaterialAmbientDiffuse[is_lighted][is_seen][2];
							calc_colours[y2][x2].a = terrainMaterialAmbientDiffuse[is_lighted][is_seen][3];

							normal = normals[my1][mx1];
							texcoord = texcoords[my1][mx1];
							calc_normals[y2][x2] = *normal;
							calc_texcoords[y2][x2] = *texcoord;
						}
					}
#endif

					// we will use quad strips for rendering
					glBegin(GL_QUAD_STRIP);
					
					// 'scanline' by 'scanline'...
					for (int y2=0;y2<scaled_square_size;y2=y2+1)
					{

						mx1 = mx + 0;
						my1 = my + y2;
						my2 = my + y2 + 1;
						wx1 = (float) mx1 * world_square_size - terrainOffsetX;
						wy1 = (float) my1 * world_square_size - terrainOffsetY;
						wy2 = (float) my2 * world_square_size - terrainOffsetY;

						glColor4f(calc_colours[y2][0].r,
						          calc_colours[y2][0].g,
						          calc_colours[y2][0].b,
						          calc_colours[y2][0].a);
						glNormal3f(calc_normals[y2][0].x, calc_normals[y2][0].y, calc_normals[y2][0].z);
						glTexCoord2f(calc_texcoords[y2][0].u, calc_texcoords[y2][0].v);

						if (y2 != 0)
						{
							// code for rendering invisible polygons that go to the starting position of the next scanline
							glVertex3f(wx1, calc_heights[y2][0], wy1);

							glVertex3f(wx1, calc_heights[y2][0], wy1);
							
						}

						// initial coordinates
						glVertex3f(wx1, calc_heights[y2][0], wy1);

						glColor4f(calc_colours[y2+1][0].r,
						          calc_colours[y2+1][0].g,
						          calc_colours[y2+1][0].b,
						          calc_colours[y2+1][0].a);
						glNormal3f(calc_normals[y2+1][0].x, calc_normals[y2+1][0].y, calc_normals[y2+1][0].z);
						glTexCoord2f(calc_texcoords[y2+1][0].u, calc_texcoords[y2+1][0].v);
						glVertex3f(wx1, calc_heights[y2+1][0], wy2);

						int x2;
						for (x2=0;x2<scaled_square_size;x2++)
						{
							// map x1, y1, x2 and y2 for the current scaled sqaure
							mx1 = mx + x2;
							my1 = my + y2;
							mx2 = mx + x2 + 1;
							my2 = my + y2 + 1;
							// world x1, y1, x2 and y2
							wx1 = (float) mx1 * world_square_size - terrainOffsetX;
							wy1 = (float) my1 * world_square_size - terrainOffsetY;
							wx2 = (float) mx2 * world_square_size - terrainOffsetX;
							wy2 = (float) my2 * world_square_size - terrainOffsetY;
						
							// render it!
							glColor4f(calc_colours[y2][x2+1].r,
								  calc_colours[y2][x2+1].g,
								  calc_colours[y2][x2+1].b,
								  calc_colours[y2][x2+1].a);
							glNormal3f(calc_normals[y2][x2+1].x, calc_normals[y2][x2+1].y, calc_normals[y2][x2+1].z);
							glTexCoord2f(calc_texcoords[y2][x2+1].u, calc_texcoords[y2][x2+1].v);
							glVertex3f(wx2, calc_heights[y2][x2+1], wy1);

							glColor4f(calc_colours[y2+1][x2+1].r,
								  calc_colours[y2+1][x2+1].g,
								  calc_colours[y2+1][x2+1].b,
								  calc_colours[y2+1][x2+1].a);
							glNormal3f(calc_normals[y2+1][x2+1].x, calc_normals[y2+1][x2+1].y, calc_normals[y2+1][x2+1].z);
							glTexCoord2f(calc_texcoords[y2+1][x2+1].u, calc_texcoords[y2+1][x2+1].v);
							glVertex3f(wx2, calc_heights[y2+1][x2+1], wy2);
							
							
						}
						
						if (y != scaled_square_size)
						{
							mx2 = mx + scaled_square_size;
							wx2 = (float) mx2 * world_square_size - terrainOffsetX;
							// code for rendering invisible polygons that go to the starting position of the next scanline
							glVertex3f(wx2, calc_heights[y2+1][x2], wy2);

							glVertex3f(wx2, calc_heights[y2+1][x2], wy2);
						}
						
					}

					glEnd();

#ifndef TERRAIN_HIGH_QUALITY

					int left_square_size, right_square_size, upper_square_size, lower_square_size;
					int level1, level2;
					int mx_2, my_2;
					int mx1_2, mx2_2;
					int my1_2, my2_2;
					float** heights_2;
					XYZCoord*** normals_2;
					UVWCoord*** texcoords_2;

					// rendering x seams
					if (y != pWorld->height/q_square_size-1 && mipmap_levels[y][x] != mipmap_levels[y+1][x] && is_visible[0][y+1] <= x && is_visible[1][y+1] >= x)
					{
						glBegin(GL_QUAD_STRIP);
					
							// scaled square sizes of the upper and lower squares
							upper_square_size = q_square_size >> mipmap_levels[y][x];
							lower_square_size = q_square_size >> mipmap_levels[y+1][x];

							mx_2 = x * lower_square_size;
							my_2 = (y+1) * lower_square_size;
							mx = x * upper_square_size;
							my = (y+1) * upper_square_size;
					
							// get mipmap levels
							level1 = mipmap_levels[y][x];
							level2 = mipmap_levels[y+1][x];
					
							// get heights, normals and texcoords of the mipmap level of the lower square
							heights_2 = HeightMipmaps[0][level2].ppHeights;
							normals_2 = HeightMipmaps[0][level2].ppNormals;
							texcoords_2 = HeightMipmaps[0][level2].ppTexCoords;
					
							if (level1 > level2) // upper square has lower quality than the lower
							{
								// put all those values that need interpolating into arrays, for easy passing
								// into interpolate_float_array
								for (int x2 = 0; x2 <= upper_square_size; x2++)
								{
									height_src_buffer[x2] = heights[my][mx+x2];
									nx_src_buffer[x2] = normals[my][mx+x2]->x;
									ny_src_buffer[x2] = normals[my][mx+x2]->y;
									nz_src_buffer[x2] = normals[my][mx+x2]->z;
									tu_src_buffer[x2] = texcoords[my][mx+x2]->u;
									tv_src_buffer[x2] = texcoords[my][mx+x2]->v;
									tw_src_buffer[x2] = texcoords[my][mx+x2]->w;
								}
								// interpolate! increase size from upper_square_size+1 to lower_square_size+1
								interpolate_float_array(height_src_buffer, upper_square_size+1, height_dest_buffer, lower_square_size+1);
								interpolate_float_array(nx_src_buffer, upper_square_size+1, nx_dest_buffer, lower_square_size+1);
								interpolate_float_array(ny_src_buffer, upper_square_size+1, ny_dest_buffer, lower_square_size+1);
								interpolate_float_array(nz_src_buffer, upper_square_size+1, nz_dest_buffer, lower_square_size+1);
								interpolate_float_array(tu_src_buffer, upper_square_size+1, tu_dest_buffer, lower_square_size+1);
								interpolate_float_array(tv_src_buffer, upper_square_size+1, tv_dest_buffer, lower_square_size+1);
								interpolate_float_array(tw_src_buffer, upper_square_size+1, tw_dest_buffer, lower_square_size+1);

								// mesh square size of the square with the highest detail level, the lower one
								world_square_size = (float) 16 / (float) (128 >> level2);

								mx1_2 = mx_2 + 0;
								mx2_2 = mx_2 + 0 + 1;
								my2_2 = my_2;
								wy2 = (float) my2_2 * world_square_size - terrainOffsetY;
								wx1 = (float) mx1_2 * world_square_size - terrainOffsetX;
								wx2 = (float) mx2_2 * world_square_size - terrainOffsetX;

								// render stuffz...
								
								// initially needed coordinates
								glNormal3f(nx_dest_buffer[0], ny_dest_buffer[0], nz_dest_buffer[0]);
								glTexCoord2f(tu_dest_buffer[0], tv_dest_buffer[0]);
								glVertex3f(wx1, height_dest_buffer[0], wy2);

								normal = normals_2[my2_2][mx1_2];
								texcoord = texcoords_2[my2_2][mx1_2];
								glNormal3f(normal->x, normal->y, normal->z);
								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx1, heights_2[my2_2][mx1_2], wy2);

								for (int x2 = 0; x2 < lower_square_size; x2++)
								{
									mx1_2 = mx_2 + x2;
									mx2_2 = mx_2 + x2 + 1;
									wx1 = (float) mx1_2 * world_square_size - terrainOffsetX;
									wx2 = (float) mx2_2 * world_square_size - terrainOffsetX;
									
									glNormal3f(nx_dest_buffer[x2+1], ny_dest_buffer[x2+1], nz_dest_buffer[x2+1]);
									glTexCoord2f(tu_dest_buffer[x2+1], tv_dest_buffer[x2+1]);
									glVertex3f(wx2, height_dest_buffer[x2+1], wy2);
									
									normal = normals_2[my2_2][mx2_2];
									texcoord = texcoords_2[my2_2][mx2_2];
									glNormal3f(normal->x, normal->y, normal->z);
									glTexCoord2f(texcoord->u, texcoord->v);
									glVertex3f(wx2, heights_2[my2_2][mx2_2], wy2);
								
								}
							}
							else if (level1 < level2) // upper square has higher quality than the lower square
							{
								for (int x2 = 0; x2 <= lower_square_size; x2++)
								{
									height_src_buffer[x2] = heights_2[my_2][mx_2+x2];
									nx_src_buffer[x2] = normals_2[my_2][mx_2+x2]->x;
									ny_src_buffer[x2] = normals_2[my_2][mx_2+x2]->y;
									nz_src_buffer[x2] = normals_2[my_2][mx_2+x2]->z;
									tu_src_buffer[x2] = texcoords_2[my_2][mx_2+x2]->u;
									tv_src_buffer[x2] = texcoords_2[my_2][mx_2+x2]->v;
									tw_src_buffer[x2] = texcoords_2[my_2][mx_2+x2]->w;
								}
								interpolate_float_array(height_src_buffer, lower_square_size+1, height_dest_buffer, upper_square_size+1);
								interpolate_float_array(nx_src_buffer, lower_square_size+1, nx_dest_buffer, upper_square_size+1);
								interpolate_float_array(ny_src_buffer, lower_square_size+1, ny_dest_buffer, upper_square_size+1);
								interpolate_float_array(nz_src_buffer, lower_square_size+1, nz_dest_buffer, upper_square_size+1);
								interpolate_float_array(tu_src_buffer, lower_square_size+1, tu_dest_buffer, upper_square_size+1);
								interpolate_float_array(tv_src_buffer, lower_square_size+1, tv_dest_buffer, upper_square_size+1);
								interpolate_float_array(tw_src_buffer, lower_square_size+1, tw_dest_buffer, upper_square_size+1);

								world_square_size = (float) 16 / (float) (128 >> level1);
								
								mx1 = mx + 0;
								mx2 = mx + 0 + 1;
								my2 = my;
								wy2 = (float) my2 * world_square_size - terrainOffsetY;
								wx1 = (float) mx1 * world_square_size - terrainOffsetX;
								wx2 = (float) mx2 * world_square_size - terrainOffsetX;

								normal = normals[my2][mx1];
								texcoord = texcoords[my2][mx1];
								glNormal3f(normal->x, normal->y, normal->z);
								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx1, heights[my2][mx1], wy2);

								glNormal3f(nx_dest_buffer[0], ny_dest_buffer[0], nz_dest_buffer[0]);
								glTexCoord2f(tu_dest_buffer[0], tv_dest_buffer[0]);
								glVertex3f(wx1, height_dest_buffer[0], wy2);

								for (int x2 = 0; x2 < upper_square_size; x2++)
								{
									mx1 = mx + x2;
									mx2 = mx + x2 + 1;
									wx1 = (float) mx1 * world_square_size - terrainOffsetX;
									wx2 = (float) mx2 * world_square_size - terrainOffsetX;
									
									normal = normals[my2][mx2];
									texcoord = texcoords[my2][mx2];
									glNormal3f(normal->x, normal->y, normal->z);
									glTexCoord2f(texcoord->u, texcoord->v);
									glVertex3f(wx2, heights[my2][mx2], wy2);

									glNormal3f(nx_dest_buffer[x2+1], ny_dest_buffer[x2+1], nz_dest_buffer[x2+1]);
									glTexCoord2f(tu_dest_buffer[x2+1], tv_dest_buffer[x2+1]);
									glVertex3f(wx2, height_dest_buffer[x2+1], wy2);

								}
							}

						glEnd();
					}

					// rendering y seams
					if (x != pWorld->width/q_square_size-1 && mipmap_levels[y][x] != mipmap_levels[y][x+1] && is_visible[0][y] <= x+1 && is_visible[1][y] >= x+1)
					{
						glBegin(GL_QUAD_STRIP);
					
							left_square_size = q_square_size >> mipmap_levels[y][x];
							right_square_size = q_square_size >> mipmap_levels[y][x+1];

							my_2 = y * right_square_size;
							mx_2 = (x+1) * right_square_size;
							my = y * left_square_size;
							mx = (x+1) * left_square_size;
					
							level1 = mipmap_levels[y][x];
							level2 = mipmap_levels[y][x+1];
					
							heights_2 = HeightMipmaps[0][level2].ppHeights;
							normals_2 = HeightMipmaps[0][level2].ppNormals;
							texcoords_2 = HeightMipmaps[0][level2].ppTexCoords;
					
							if (level1 > level2)
							{
								for (int y2 = 0; y2 <= left_square_size; y2++)
								{
									height_src_buffer[y2] = heights[my+y2][mx];
									nx_src_buffer[y2] = normals[my+y2][mx]->x;
									ny_src_buffer[y2] = normals[my+y2][mx]->y;
									nz_src_buffer[y2] = normals[my+y2][mx]->z;
									tu_src_buffer[y2] = texcoords[my+y2][mx]->u;
									tv_src_buffer[y2] = texcoords[my+y2][mx]->v;
									tw_src_buffer[y2] = texcoords[my+y2][mx]->w;
								}
								interpolate_float_array(height_src_buffer, left_square_size+1, height_dest_buffer, right_square_size+1);
								interpolate_float_array(nx_src_buffer, left_square_size+1, nx_dest_buffer, right_square_size+1);
								interpolate_float_array(ny_src_buffer, left_square_size+1, ny_dest_buffer, right_square_size+1);
								interpolate_float_array(nz_src_buffer, left_square_size+1, nz_dest_buffer, right_square_size+1);
								interpolate_float_array(tu_src_buffer, left_square_size+1, tu_dest_buffer, right_square_size+1);
								interpolate_float_array(tv_src_buffer, left_square_size+1, tv_dest_buffer, right_square_size+1);
								interpolate_float_array(tw_src_buffer, left_square_size+1, tw_dest_buffer, right_square_size+1);

								world_square_size = (float) 16 / (float) (128 >> level2);

								my1_2 = my_2 + 0;
								my2_2 = my_2 + 0 + 1;
								mx2_2 = mx_2;
								wx2 = (float) mx2_2 * world_square_size - terrainOffsetX;
								wy1 = (float) my1_2 * world_square_size - terrainOffsetY;
								wy2 = (float) my2_2 * world_square_size - terrainOffsetY;

								normal = normals_2[my1_2][mx2_2];
								texcoord = texcoords_2[my1_2][mx2_2];
								glNormal3f(normal->x, normal->y, normal->z);
								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, heights_2[my1_2][mx2_2], wy1);

								glNormal3f(nx_dest_buffer[0], ny_dest_buffer[0], nz_dest_buffer[0]);
								glTexCoord2f(tu_dest_buffer[0], tv_dest_buffer[0]);
								glVertex3f(wx2, height_dest_buffer[0], wy1);

								for (int y2 = 0; y2 < right_square_size; y2++)
								{
									my1_2 = my_2 + y2;
									my2_2 = my_2 + y2 + 1;
									mx2_2 = mx_2;
									wy1 = (float) my1_2 * world_square_size - terrainOffsetY;
									wy2 = (float) my2_2 * world_square_size - terrainOffsetY;
									
									normal = normals_2[my2_2][mx2_2];
									texcoord = texcoords_2[my2_2][mx2_2];
									glNormal3f(normal->x, normal->y, normal->z);
									glTexCoord2f(texcoord->u, texcoord->v);
									glVertex3f(wx2, heights_2[my2_2][mx2_2], wy2);
								
									glNormal3f(nx_dest_buffer[y2+1], ny_dest_buffer[y2+1], nz_dest_buffer[y2+1]);
									glTexCoord2f(tu_dest_buffer[y2+1], tv_dest_buffer[y2+1]);
									glVertex3f(wx2, height_dest_buffer[y2+1], wy2);
									
								}
							}
							else if (level1 < level2)
							{
								for (int y2 = 0; y2 <= right_square_size; y2++)
								{
									height_src_buffer[y2] = heights_2[my_2+y2][mx_2];
									nx_src_buffer[y2] = normals_2[my_2+y2][mx_2]->x;
									ny_src_buffer[y2] = normals_2[my_2+y2][mx_2]->y;
									nz_src_buffer[y2] = normals_2[my_2+y2][mx_2]->z;
									tu_src_buffer[y2] = texcoords_2[my_2+y2][mx_2]->u;
									tv_src_buffer[y2] = texcoords_2[my_2+y2][mx_2]->v;
									tw_src_buffer[y2] = texcoords_2[my_2+y2][mx_2]->w;
								}
								interpolate_float_array(height_src_buffer, right_square_size+1, height_dest_buffer, left_square_size+1);
								interpolate_float_array(nx_src_buffer, right_square_size+1, nx_dest_buffer, left_square_size+1);
								interpolate_float_array(ny_src_buffer, right_square_size+1, ny_dest_buffer, left_square_size+1);
								interpolate_float_array(nz_src_buffer, right_square_size+1, nz_dest_buffer, left_square_size+1);
								interpolate_float_array(tu_src_buffer, right_square_size+1, tu_dest_buffer, left_square_size+1);
								interpolate_float_array(tv_src_buffer, right_square_size+1, tv_dest_buffer, left_square_size+1);
								interpolate_float_array(tw_src_buffer, right_square_size+1, tw_dest_buffer, left_square_size+1);

								world_square_size = (float) 16 / (float) (128 >> level1);

								my1 = my + 0;
								my2 = my + 0 + 1;
								mx2 = mx;
								wx2 = (float) mx2 * world_square_size - terrainOffsetX;
								wy1 = (float) my1 * world_square_size - terrainOffsetY;
								wy2 = (float) my2 * world_square_size - terrainOffsetY;

								glTexCoord2f(1.0f, 1.0f);
								
								glNormal3f(nx_dest_buffer[0], ny_dest_buffer[0], nz_dest_buffer[0]);
								glTexCoord2f(tu_dest_buffer[0], tv_dest_buffer[0]);
								glVertex3f(wx2, height_dest_buffer[0], wy1);

								normal = normals[my1][mx2];
								texcoord = texcoords[my1][mx2];
								glNormal3f(normal->x, normal->y, normal->z);
								glTexCoord2f(texcoord->u, texcoord->v);
								glVertex3f(wx2, heights[my1][mx2], wy1);

								for (int y2 = 0; y2 < left_square_size; y2++)
								{
									my1 = my + y2;
									my2 = my + y2 + 1;
									mx2 = mx;
									wy1 = (float) my1 * world_square_size - terrainOffsetY;
									wy2 = (float) my2 * world_square_size - terrainOffsetY;
									
									glNormal3f(nx_dest_buffer[y2+1], ny_dest_buffer[y2+1], nz_dest_buffer[y2+1]);
									glTexCoord2f(tu_dest_buffer[y2+1], tv_dest_buffer[y2+1]);
									glVertex3f(wx2, height_dest_buffer[y2+1], wy2);
									
									normal = normals[my2][mx2];
									texcoord = texcoords[my2][mx2];
									glNormal3f(normal->x, normal->y, normal->z);
									glTexCoord2f(texcoord->u, texcoord->v);
									glVertex3f(wx2, heights[my2][mx2], wy2);
								
								}
							}

						glEnd();
					}

#endif

				}
			}

			glDisable(GL_COLOR_MATERIAL);
			
			return SUCCESS;
		}

		void DrawWater()
		{
			int mipmap_level, scaled_square_size;
			float world_square_size;
			int mx, my;
			int mx1, my1, mx2, my2;
			float wx1, wy1, wx2, wy2;
			float mix = Rules::time_passed_since_last_water_pass * 3;
			int step_size;
			XYZCoord* normal;
			bool is_seen, is_lighted;
			int** NumUnitsSeeingSquare = Dimension::currentPlayerView->NumUnitsSeeingSquare;
			
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, waterMaterialSpecular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, waterMaterialEmission);
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
			
			glDisable(GL_COLOR_MATERIAL);
			
		}

		void UnloadTerrain(void)
		{
			for(int y = 0; y < pWorld->height/q_square_size; y++)
			{
				delete [] q_levels[y];
				delete [] big_square_has_water[y];
				delete [] new_q_levels[y];
			}
			delete [] q_levels;
			delete [] big_square_has_water;
			delete [] new_q_levels;

			delete [] is_visible[0];
			delete [] is_visible[1];

			//HeightMap HeightMipmaps[2][32] <-- Const
			for(int y = 0; y < pWorld->height; y++)
			{
				for(int x = 0; x < pWorld->width; x++)
				{
					delete HeightMipmaps[0][0].ppNormals[y][x];
				}
				delete [] HeightMipmaps[0][0].ppNormals[y];
			}
			delete [] HeightMipmaps[0][0].ppNormals;

			for(int y = 0; y < pWorld->height; y++)
			{
				for(int x = 0; x < pWorld->width; x++)
				{
					delete HeightMipmaps[0][0].ppTexCoords[y][x];
				}
				delete [] HeightMipmaps[0][0].ppTexCoords[y];
			}

			delete [] HeightMipmaps[0][0].ppTexCoords;
			
			for(int y = 0; y < pWorld->height; y++)
			{
				delete [] HeightMipmaps[0][0].ppSquareHasWater[y];
			}
			delete [] HeightMipmaps[0][0].ppSquareHasWater;
		
			if (!Game::Rules::noGraphics)
			{

				for(int y = 0; y < pWorld->height; y++)
				{
					delete [] pWorld->ppWater[0][y];
					delete [] pWorld->ppWater[1][y];
					delete [] pWorld->ppWater[2][y];
				}

				delete [] pWorld->ppWater[0];
				delete [] pWorld->ppWater[1];
				delete [] pWorld->ppWater[2];

				for(int y = 0; y < pWorld->height; y++)
				{
					for(int x = 0; x < pWorld->width; x++)
					{
						delete water_normals[y][x];
					}
					delete [] water_normals[y];
				}

				delete [] water_normals;

				int mipmap_num = 1;
				int new_width, new_height, old_width, old_height;

				new_width = pWorld->width;
				new_height = pWorld->height;

				for(;;mipmap_num++)
				{
					// calculate the new height and width
					// if the first height is 257, the next is 129, then 65 etc
					old_width = new_width;
					old_height = new_height;
					new_width = ((new_width - 1) / 2) + 1;
					new_height = ((new_height - 1) / 2) + 1;

					// exit if it's not possible to calculate any more mipmaps

					if ((old_width != (new_width-1)*2+1) || old_height != (new_height-1)*2+1)
						break;

	#ifdef TERRAIN_HIGH_QUALITY
					for(int y=0;y<new_height;y++)
					{
						for(int x=0;x<new_width;x++)
						{
							int x2 = x<<1, y2 = y<<1;
							SphereNormal ***normals_2 = HeightMipmaps[1][mipmap_num].ppNormals;
							UVWCoord ***texcoords_2 = HeightMipmaps[1][mipmap_num].ppTexCoords;

							delete normals_2[y2][x2];
							delete texcoords_2[y2][x2];
							if (x != new_width-1)
							{
								delete normals_2[y2][x2+1];
								delete texcoords_2[y2][x2+1];
								if (y != new_height-1)
								{
									delete normals_2[y2+1][x2+1];
									delete texcoords_2[y2+1][x2+1];
								}
							}
							if (y != new_width-1)
							{
								delete normals_2[y2+1][x2];
								delete texcoords_2[y2+1][x2];
							}
						}
					}

					for(int y=0;y<old_height;y++)
					{
						delete [] HeightMipmaps[1][mipmap_num].ppHeights[y];
						delete [] HeightMipmaps[1][mipmap_num].ppNormals[y];
						delete [] HeightMipmaps[1][mipmap_num].ppTexCoords[y];
					}
					delete [] HeightMipmaps[1][mipmap_num].ppHeights;
					delete [] HeightMipmaps[1][mipmap_num].ppNormals;
					delete [] HeightMipmaps[1][mipmap_num].ppTexCoords;
	#endif

					for(int y=0;y<new_height;y++)
					{
						for(int x=0;x<new_height;x++)
						{
							delete HeightMipmaps[0][mipmap_num].ppTexCoords[y][x];
							delete HeightMipmaps[0][mipmap_num].ppNormals[y][x];
						}
						delete [] HeightMipmaps[0][mipmap_num].ppNormals[y];
						delete [] HeightMipmaps[0][mipmap_num].ppTexCoords[y];
						delete [] HeightMipmaps[0][mipmap_num].ppHeights[y];
					}

					delete [] HeightMipmaps[0][mipmap_num].ppHeights;
					delete [] HeightMipmaps[0][mipmap_num].ppNormals;
					delete [] HeightMipmaps[0][mipmap_num].ppTexCoords;
				}
			
				for (int y = 0; y < pWorld->height; y++)
				{
					delete [] pWorld->ppSteepness[y];
				}
				delete [] pWorld->ppSteepness;

			}

			for(int y = 0; y < pWorld->height; y++)
			{
				delete [] pWorld->ppHeight[y];
			}
			delete [] pWorld->ppHeight;

			int h = pWorld->height/q_square_size;
			for(int i = 0; i < h; i++)
			{
				delete [] mipmap_levels[i];
			}
			delete [] mipmap_levels;

			
			int** NumLightsOnSquare = pWorld->NumLightsOnSquare;
			for (int y = 0; y < pWorld->height; y++)
			{
				delete [] NumLightsOnSquare[y];
			}
			delete [] pWorld->NumLightsOnSquare;

			
			for (int y = 0; y < pWorld->height; y++)
			{
				delete [] pppElements[y];
			}
			delete [] pppElements;
		}

		void UnloadWorld(void)
		{
			if (pWorld == NULL)
				return;
			
			//Deallocate Units
			while(pWorld->vUnits.size() > 0)
				DeleteUnit(pWorld->vUnits.at(0));

//			AI::ClearPathNodeStack();

			//Deallocate Player
			for(unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				Player *pPlayer = pWorld->vPlayers.at(i);
				for(int y = 0; y < pWorld->height; y++)
				{
					delete [] pPlayer->NumUnitsSeeingSquare[y];
				}
				delete [] pPlayer->NumUnitsSeeingSquare;
				delete [] pPlayer->states;

				delete pPlayer;
			}			

			//Deallocate Terrain & Water & Heightmap
			UnloadTerrain();
			glDeleteTextures(1, &Dimension::terraintexture);
				
			delete pWorld;
		}
	}
}	
