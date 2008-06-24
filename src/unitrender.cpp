#include "unitrender.h"

#include "unit.h"
#include "unittype.h"
#include "utilities.h"
#include "unitsquares.h"
#include "game.h"
#include "aipathfinding.h"
#include "window.h"
#include "textures.h"
#include "ogrexmlmodel.h"

using namespace std;

namespace Game
{
	namespace Dimension
	{
		GLuint        genericTexture;
		
		GLfloat    unitMaterialAmbient[2][2][4] = {
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							      {0.4f, 0.4f, 0.4f, 1.0f}    // seen, not lighted
							    },
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							      {0.8f, 0.8f, 0.8f, 1.0f}    // seen, lighted
							    }
							  };
		
		GLfloat    unitMaterialDiffuse[2][2][4] = {
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							      {0.4f, 0.4f, 0.4f, 1.0f}    // seen, not lighted
							    },
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							      {0.8f, 0.8f, 0.8f, 1.0f}    // seen, lighted
							    }
							  };
		
		GLfloat    unitMaterialSpecular[2][2][4] = {
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, not lighted
							     },
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, lighted
							     }
							   };
		
		GLfloat    unitMaterialEmission[2][2][4] = {
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, not lighted
							     },
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, lighted
							     }
							   };
		
		GLfloat    unitMaterialShininess[2][2] = {
							   {
							     10.0,   // not seen, not lighted
							     10.0    // seen, not lighted
							   },
							   {
							     10.0,   // not seen, lighted
							     10.0    // seen, lighted
							   }
							 };
							 
		void SetParticleCoordSpace(float x, float y, float z, float scale)
		{
			float unit_x, unit_y, unit_z;
			Utilities::Vector3D up_vector, normal, rotate_axis;
			unit_x = x;
			unit_z = y;

			// Translate to the position of the terrain at the position of the unit
			unit_y = GetTerrainHeight(unit_x, unit_z) + z;
			glTranslatef(unit_x * 0.125f - terrainOffsetX, unit_y, unit_z * 0.125f - terrainOffsetY);
			glScalef(0.0625f*scale, 0.0625f*scale, 0.0625f*scale);
		}

		// sets the coordinate system so you can render a billboard at the current position
		// assumes that SetUnitCoordSpace has been called already
		void SetBillBoardCoordSystem(Unit *unit)
		{
			Utilities::Vector3D near_plane, far_plane, up, right, look, up_vector, rotate_axis, normal, window_coord;
			float degrees_to_rotate;
			GLfloat matrix[16];

			glRotatef(-unit->rotation, 0.0f, 1.0f, 0.0f);
			
			up_vector.set(0.0f, 1.0f, 0.0f);
			normal = GetTerrainNormal(unit->pos.x, unit->pos.y);

			rotate_axis = up_vector;
			rotate_axis.cross(normal);

			degrees_to_rotate = acos(up_vector.dot(normal)) * (float) (180 / PI);

			glRotatef(-degrees_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

			WorldCoordToWindowCoord(Utilities::Vector3D(0, 0, 0), window_coord);

			WindowCoordToVector(window_coord.x, window_coord.y, near_plane, far_plane);

			look = near_plane;
			look.normalize();
			up.set(0.0, 1.0, 0.0);

			right = up;
			right.cross(look);

			right.normalize();

			up = look;
			up.cross(right);

			matrix[0] = right.x;
			matrix[1] = right.y;
			matrix[2] = right.z;
			matrix[3] = 0.0;

			matrix[4] = up.x;
			matrix[5] = up.y;
			matrix[6] = up.z;
			matrix[7] = 0.0;

			matrix[8] = look.x;
			matrix[9] = look.y;
			matrix[10] = look.z;
			matrix[11] = 0.0;

			matrix[12] = 0.0;
			matrix[13] = 0.0;
			matrix[14] = 0.0;
			matrix[15] = 1.0;

			glMultMatrixf((GLfloat*) &matrix);
		}

		// Set the coordinate space of a projectile, so you then can just render the projectile and it will be placed at the appropriate
		// position
		void SetProjectileCoordSpace(Projectile* proj)
		{
			ProjectileType* type = proj->type;
			float degrees_to_rotate;
			Utilities::Vector3D up_vector, rotate_axis;
			
			glTranslatef(proj->pos.x * 0.125f - terrainOffsetX, proj->pos.z, proj->pos.y * 0.125f - terrainOffsetY);

			// rotate so the unit will be placed correctly onto possibly leaning ground, by rotating by the difference between
			// the up vector and the terrain normal (get degrees to rotate by with dot product, get axis with cross product)
			up_vector.set(0.0f, 1.0f, 0.0f);

			rotate_axis = up_vector;
			rotate_axis.cross(proj->direction);

			degrees_to_rotate = acos(up_vector.dot(proj->direction)) * (float) (180 / PI);

			glRotatef(degrees_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

			// scale down
			glScalef(0.0625f*type->size, 0.0625f*type->size, 0.0625f*type->size);
				
		}

		// Check whether a click at (clickx, clicky) hit unit
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance)
		{
			UnitType* type = unit->type;
			Utilities::OgreMesh* model = type->mesh;
			Utilities::Vector3D near_plane, far_plane, tp1, tp2, tp3, hit_pos;
			int index, index_v;

			if (!model)
				return false;

			glPushMatrix();

				SetUnitCoordSpace(unit);

				WindowCoordToVector((GLdouble) clickx, (GLdouble) clicky, near_plane, far_plane);
				
				index = 0;

/*				for (int i = 0; i < model->tri_count; i++)
				{
					index_v = model->tris[index++] * 3;
					tp1.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					index_v = model->tris[index++] * 3;
					tp2.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					index_v = model->tris[index++] * 3;
					tp3.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					if (CheckLineIntersectTri(tp1, tp3, tp2, near_plane, far_plane, hit_pos))
					{
						glPopMatrix();
						distance = near_plane.distance(hit_pos);
						return true;
					}
				}*/

			glPopMatrix();

			return false;
		}
		
		// get the screen coord of the middle (0.0, 0.0, 0.0) of unit
		Utilities::Vector3D GetUnitWindowPos(Unit* unit)
		{
			Utilities::Vector3D win_vector;
			glPushMatrix();

				SetUnitCoordSpace(unit);

				WorldCoordToWindowCoord(Utilities::Vector3D(0.0f, 0.0f, 0.0f), win_vector);

			glPopMatrix();

			return win_vector;

		}

		// get what unit was clicked, if any
		Unit* GetUnitClicked(int clickx, int clicky, int map_x, int map_y)
		{
			Unit* unit;
			Unit* cur_unit = 0;
			float cur_dist = 1e10, dist;
			for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
			{
				unit = *it;
				if (UnitIsRendered(unit, currentPlayerView))
				{
					if (unit->curAssociatedSquare.x > map_x-3 && unit->curAssociatedSquare.x < map_x+3 && unit->curAssociatedSquare.y < map_y+3 && unit->curAssociatedSquare.y < map_y+3)
					{
						if (Dimension::DoesHitUnit(unit, clickx, clicky, dist))
						{
							if (dist < cur_dist)
							{
								cur_unit = unit;
								cur_dist = dist;
							}
						}
					}
				}
			}
			return cur_unit;
		}

		void RenderBuildOutline(UnitType* type, int pos_x, int pos_y)
		{
			Utilities::Vector3D ter[2][2];
			int end_x, end_y;
			bool outofbounds = false;
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, pos_x, pos_y, start_x, start_y);
			bool suitable = IsSuitableForBuilding(type, start_x, start_y);
			end_x = start_x + type->widthOnMap-1;
			end_y = start_y + type->heightOnMap-1;

			if (start_x < 0)
				start_x = 0, outofbounds = true;
			
			if (start_y < 0)
				start_y = 0, outofbounds = true;
			
			if (end_x >= pWorld->width-1)
				end_x = pWorld->width-2, outofbounds = true;

			if (end_y >= pWorld->height-1)
				end_y = pWorld->height-2, outofbounds = true;

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBegin(GL_QUADS);
				for (int y = start_y; y <= end_y; y++)
				{
					for (int x = start_x; x <= end_x; x++)
					{
						if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
						{
							ter[0][0] = Dimension::GetTerrainCoord((float)x, (float)y);
							ter[0][1] = Dimension::GetTerrainCoord((float)x+1, (float)y);
							ter[1][0] = Dimension::GetTerrainCoord((float)x, (float)y+1);
							ter[1][1] = Dimension::GetTerrainCoord((float)x+1, (float)y+1);

							if (outofbounds || !SquareIsWalkable(type, x, y, SIW_IGNORE_OWN_MOBILE_UNITS))
							{
								glColor4f(1.0f, 0.0f, 0.0f, 1.0);
							}
							else
							{
								if (suitable)
								{
									glColor4f(0.0, 1.0, 0.0, 1.0);
								}
								else
								{
									glColor4f(1.0, 1.0, 0.0, 1.0);
								}
							}
							glNormal3f(0.0f, 1.0f, 0.0f);
							glVertex3f(ter[0][0].x, ter[0][0].y, ter[0][0].z);
							glVertex3f(ter[1][0].x, ter[1][0].y, ter[1][0].z);
							glVertex3f(ter[1][1].x, ter[1][1].y, ter[1][1].z);
							glVertex3f(ter[0][1].x, ter[0][1].y, ter[0][1].z);
						}
					}
				}
			glEnd();

			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);
		}

		void RenderProjectile(Projectile* proj)
		{
			if (SquareIsVisible(currentPlayerView, (int) proj->pos.x, (int) proj->pos.y))
			{
				Model* model = proj->type->model;
				glPushMatrix();

				SetProjectileCoordSpace(proj);

				glBegin(GL_TRIANGLES);

					if (model && model->texCoords)
					{
						RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, model->normals, model->vertices, model->texCoords);
					}
					else
					{
						RenderTriangles(model->tri_count, model->tris, model->normals, model->vertices);
					}

				glEnd();

				glPopMatrix();
			}	
		}

		void CalculateMaterial(Unit* unit, GLfloat (&dest)[4], GLfloat source[4], GLfloat (&mod)[2][2][4])
		{
			GLfloat temp[4] = {0.0, 0.0, 0.0, 0.0};
			int num = 0;
			int start_x, start_y;
			int is_seen, is_lighted;
			Uint16** NumUnitsSeeingSquare = currentPlayerView->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_seen = NumUnitsSeeingSquare[y][x] > 0 ? 1 : 0;
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						for (int i = 0; i < 4; i++)
							temp[i] += source[i] * mod[is_lighted][is_seen][i];
						num++;
					}
				}
			}
			if (num)
				for (int i = 0; i < 4; i++)
					dest[i] = temp[i] / (float) num;
		}

		void CalculateMaterial(Unit* unit, GLfloat& dest, GLfloat source, GLfloat (&mod)[2][2])
		{
			GLfloat temp = 0.0;
			int num = 0;
			int start_x, start_y;
			bool is_seen, is_lighted;
			Uint16** NumUnitsSeeingSquare = currentPlayerView->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_seen = NumUnitsSeeingSquare[y][x] > 0 ? 1 : 0;
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						temp += source * mod[is_lighted][is_seen];
						num++;
					}
				}
			}
			if (num)
				dest = temp / (float) num;
		}

		void RenderMorphAnim(Unit* unit, MorphAnim* morphAnim, int& animNum)
		{
			UnitType* type;
			Model* model, *tempModel;
			Model *a_models[4];
			int a_frames[2][4];
			float pos_between_anim_frames[2];
			float fade_out = 0.0f;
			float mix = 0.0f;
			GLfloat mixcol[] = {0.66f, 0.66f, 0.66f, 1.0f};
			GLfloat ambient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			GLfloat diffuse[4] = {0.1f, 0.1f, 0.1f, 1.0f};
			GLfloat specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			GLfloat emission[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			GLfloat shininess = 10.0f;

			glEnable(GL_RESCALE_NORMAL);

			model = morphAnim->models[0];
			type = unit->type;

			glActiveTextureARB(GL_TEXTURE0_ARB);

			if(model->texture == 0)
			{
				glBindTexture(GL_TEXTURE_2D, genericTexture);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, model->texture);
			}
		
			CalculateMaterial(unit, ambient, model->Material_Ambient, unitMaterialAmbient);
			CalculateMaterial(unit, diffuse, model->Material_Diffuse, unitMaterialDiffuse);
			CalculateMaterial(unit, emission, model->Material_Emissive, unitMaterialEmission);
			CalculateMaterial(unit, specular, model->Material_Specular, unitMaterialSpecular);
			CalculateMaterial(unit, shininess, *model->Material_Shininess, unitMaterialShininess);

			if (unit->pMovementData->action.action == AI::ACTION_DIE)
			{
				fade_out = 1.0f - (float) (AI::currentFrame - unit->lastAttacked) / (float) AI::aiFps;
				diffuse[3] = fade_out;
				glDisable(GL_DEPTH_TEST);
			}
			
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mixcol);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE1_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR);

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, unit->owner->texture);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
			
			if (morphAnim->numKeyFrames != 1)
			{
				
				HandleAnim(a_frames, animNum, mix, unit, morphAnim, ANIM_MORPH, pos_between_anim_frames);

				tempModel = morphAnim->tempModel;

				for (int i = 0; i < 4; i++)
				{
					a_models[i] = morphAnim->models[a_frames[0][i]];
				}

				for (int i = 0; i < tempModel->pointCount * 3; i++)
				{
					tempModel->vertices[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->vertices[i],
											      a_models[1]->vertices[i],
											      a_models[2]->vertices[i],
											      a_models[3]->vertices[i],
											      pos_between_anim_frames[0]);

					tempModel->normals[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->normals[i],
											     a_models[1]->normals[i],
											     a_models[2]->normals[i],
											     a_models[3]->normals[i],
											     pos_between_anim_frames[0]);
				}

				for (int i = 0; i < tempModel->texPointCount * 2; i++)
				{
					tempModel->texCoords[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->texCoords[i],
											       a_models[1]->texCoords[i],
											       a_models[2]->texCoords[i],
											       a_models[3]->texCoords[i],
											       pos_between_anim_frames[0]);
				}
				
				model = tempModel;
		
				glBegin(GL_TRIANGLES);

					if (tempModel->texCoords == NULL)
					{
						RenderTriangles(model->tri_count, model->tris, model->normals, model->vertices);
					}
					else
					{
						RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, model->normals, model->vertices, model->texCoords);
					}

				glEnd();
			}
			else
			{

				tempModel = morphAnim->tempModel;
				if (Window::hasVBOs)
				{
					if (model->staticArrays.vertexArray == 0)
					{
						GLfloat *vertices = new GLfloat[model->tri_count * 9];
						GLfloat *normals = new GLfloat[model->tri_count * 9];
						GLfloat *texCoords = new GLfloat[model->tri_count * 6];
						glGenBuffersARB(1, &model->staticArrays.vertexArray);
						glGenBuffersARB(1, &model->staticArrays.normalArray);
						glGenBuffersARB(1, &model->staticArrays.texCoordArray);
						glGenBuffersARB(1, &model->dynamicArrays.normalArray);
						
						int index_v, index_t, index = 0;
						int write_v = 0, write_t = 0;
						for (int j = 0; j < model->tri_count; j++)
						{
							for (int k = 0; k < 3; k++)
							{
								index_v = model->tris[index] * 3;

								normals[write_v] = model->normals[index_v];
								normals[write_v+1] = model->normals[index_v+1];
								normals[write_v+2] = model->normals[index_v+2];

								vertices[write_v] = model->vertices[index_v];
								vertices[write_v+1] = model->vertices[index_v+1];
								vertices[write_v+2] = model->vertices[index_v+2];

								write_v += 3;

								if (model->texCoords)
								{
									index_t = model->tex_tris[index] * 2;
									texCoords[write_t] = model->texCoords[index_t];
									texCoords[write_t+1] = model->texCoords[index_t+1];
									write_t += 2;
								}

								index++;
							}
						}
						
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.vertexArray);
						glBufferDataARB(GL_ARRAY_BUFFER_ARB, model->tri_count * 9 * sizeof(GLfloat), vertices, GL_STATIC_DRAW_ARB);
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.normalArray);
						glBufferDataARB(GL_ARRAY_BUFFER_ARB, model->tri_count * 9 * sizeof(GLfloat), normals, GL_STATIC_DRAW_ARB);
						if (model->texCoords)
						{
							glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.texCoordArray);
							glBufferDataARB(GL_ARRAY_BUFFER_ARB, model->tri_count * 6 * sizeof(GLfloat), texCoords, GL_STATIC_DRAW_ARB);
						}

						delete[] vertices;
						delete[] normals;
						delete[] texCoords;
					}

					glEnableClientState(GL_NORMAL_ARRAY);

					glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.normalArray);
					glNormalPointer(GL_FLOAT, 0, NULL);

					glEnableClientState(GL_VERTEX_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.vertexArray);
					glVertexPointer(3, GL_FLOAT, 0, NULL);
					if (model->texCoords)
					{
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->staticArrays.texCoordArray);
						glTexCoordPointer(2, GL_FLOAT, 0, NULL);
						glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					}
					
					glDrawArrays(GL_TRIANGLES, 0, model->tri_count*3);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

					if (model->texCoords)
					{
						glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					}

					glDisableClientState(GL_VERTEX_ARRAY);
					glDisableClientState(GL_NORMAL_ARRAY);
				}
				else
				{
					glBegin(GL_TRIANGLES);

					if (model->texCoords == NULL)
					{
						RenderTriangles(model->tri_count, model->tris, model->normals, model->vertices);
					}
					else
					{
						RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, model->normals, model->vertices, model->texCoords);
					}

					glEnd();
				}
			}

			if (unit->pMovementData->action.action == AI::ACTION_DIE)
			{
				glEnable(GL_DEPTH_TEST);
			}
			
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

			glDisable(GL_RESCALE_NORMAL);

			animNum++;
		}

		void RenderUnits()
		{
			Unit* unit;
			UnitType* type;
			unsigned int uindex;
			unsigned int aindex;
			unsigned int size = pWorld->vUnits.size();
//			TransformAnim* animations[2];
			for (uindex = 0; uindex < size; uindex++)
			{
				unit = pWorld->vUnits.at(uindex);
				type = unit->type;
				int animNum = 0;

				if (UnitIsRendered(unit, currentPlayerView))
				{

					glPushMatrix();

						SetUnitCoordSpace(unit);

	//					unit->animData.anim[0] =
						if (type->animations[unit->pMovementData->action.action])
						{
							for (int i = 0; i < type->animations[unit->pMovementData->action.action]->num_parts; i++)
							{
		//						animations[0] = unit->animData.anim[0]->transAnim;
		//						animations[1] = unit->animData.anim[1]->transAnim;
								RenderTransAnim(unit, type->animations[unit->pMovementData->action.action]->transAnim[i], animNum);
							}
						}
				
					glPopMatrix();
						
				}

				for (unsigned int i = 0; i < unit->projectiles.size(); i++)
				{
					RenderProjectile(unit->projectiles.at(i));
				}
			}

		}

/*			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						
			size = Dimension::unitsDisplayQueue.size();
			Unit* master = NULL;
			for (uindex = 0; uindex < size; uindex++)
			{
				master = Dimension::unitsDisplayQueue.at(uindex);
				if (!master)
					continue;

				for (aindex = 0; aindex < master->actionQueue.size(); aindex ++)
				{
					ActionQueueItem* q = master->actionQueue.at(aindex);
					if (q->ghost)
					{
						unit = q->ghost;
						type = unit->type;
						int animNum = 0;

						glPushMatrix();

						SetUnitCoordSpace(unit);

						if (type->animations[unit->pMovementData->action.action])
						{
							for (int i = 0; i < type->animations[unit->pMovementData->action.action]->num_parts; i++)
							{
								RenderTransAnim(unit, type->animations[unit->pMovementData->action.action]->transAnim[i], animNum);
							}
						}
						glPopMatrix();
					}
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/

		void RenderHealthBars()
		{
			Unit* unit;
			UnitType* type;
			unsigned int uindex;

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);

			for (uindex = 0; uindex < unitsSelected.size(); uindex++)
			{
				unit = unitsSelected.at(uindex);
				type = unit->type;

				if (UnitIsRendered(unit, currentPlayerView))
				{
					glPushMatrix();

						SetUnitCoordSpace(unit, true);

						glTranslatef(0.0f, 0.625f * type->height, 0.0f);
						SetBillBoardCoordSystem(unit);
						
						float scale_const = 1.0f / pow((float) type->size, 0.75f);
						glScalef(scale_const, scale_const, scale_const);
						
						// s = start
						// e = end
						// i = inner
						float x_e = (float) unit->type->widthOnMap / 2.0f;
						float x_s = -x_e;
						float y_s = 0.0;
						float y_e = 0.2f;
						float progress = 0;

						glBegin(GL_QUADS);
						
							progress = (float) unit->health / (float) type->maxHealth * (x_e - x_s);

							glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
							glVertex3f(x_s, y_s, 0.0);
							glVertex3f(x_s + progress, y_s, 0.0);
							glVertex3f(x_s + progress, y_e, 0.0);
							glVertex3f(x_s, y_e, 0.0);

							glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
							glVertex3f(x_s + progress, y_s, 0.0);
							glVertex3f(x_e, y_s, 0.0);
							glVertex3f(x_e, y_e, 0.0);
							glVertex3f(x_s + progress, y_e, 0.0);

							if (unit->pMovementData->action.action == AI::ACTION_BUILD)
							{
								if (unit->pMovementData->action.goal.unit != NULL)
								{
									Unit* target = unit->pMovementData->action.goal.unit;
									if (!target->isCompleted)
									{
										y_s = -0.3f;
										y_e = -0.1f;
										progress = target->completeness / 100.0f * (x_e - x_s);

										glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
										glVertex3f(x_s, y_s, 0.0);
										glVertex3f(x_s + progress, y_s, 0.0);
										glVertex3f(x_s + progress, y_e, 0.0);
										glVertex3f(x_s, y_e, 0.0);

										glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
										glVertex3f(x_s + progress, y_s, 0.0);
										glVertex3f(x_e, y_s, 0.0);
										glVertex3f(x_e, y_e, 0.0);
										glVertex3f(x_s + progress, y_e, 0.0);
									}
								}
							}
							
							if (unit->pMovementData->action.action == AI::ACTION_RESEARCH)
							{
								y_s = -0.3f;
								y_e = -0.1f;
								progress = unit->action_completeness / 100.0f * (x_e - x_s);

								glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
								glVertex3f(x_s, y_s, 0.0);
								glVertex3f(x_s + progress, y_s, 0.0);
								glVertex3f(x_s + progress, y_e, 0.0);
								glVertex3f(x_s, y_e, 0.0);

								glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
								glVertex3f(x_s + progress, y_s, 0.0);
								glVertex3f(x_e, y_s, 0.0);
								glVertex3f(x_e, y_e, 0.0);
								glVertex3f(x_s + progress, y_e, 0.0);

							}

						glEnd();

					glPopMatrix();
				}

			}
			Utilities::RevertViewport();
			
		}

		void InitRenderUnits()
		{
			genericTexture = Utilities::LoadGLTexture((char*) "models/textures/generic.png");
		}

		UnitTransfNode::UnitTransfNode(Unit* unit)
		{
			this->unit = unit;
		}

		void UnitTransfNode::PreRender()
		{
			UnitType* type = unit->type;
			float unit_x, unit_y, unit_z, radians_to_rotate;
			Utilities::Vector3D up_vector, normal, rotate_axis;
			Matrix4x4& mVMatrix = matrices[MATRIXTYPE_MODELVIEW];

			mtxStack[MATRIXTYPE_MODELVIEW].push(mVMatrix);
		
			unit_x = unit->pos.x;
			unit_z = unit->pos.y;

			if ((type->widthOnMap >> 1) << 1 == type->widthOnMap)
			{
				unit_x -= 0.5f;
			}
			if ((type->heightOnMap >> 1) << 1 == type->heightOnMap)
			{
				unit_z -= 0.5f;
			}
			
			// Translate to the position of the terrain at the position of the unit
			unit_y = GetTerrainHeight(unit_x, unit_z);
			mVMatrix.Translate(unit_x * 0.125f - terrainOffsetX, unit_y, unit_z * 0.125f - terrainOffsetY);

			// rotate so the unit will be placed correctly onto possibly leaning ground, by rotating by the difference between
			// the up vector and the terrain normal (get degrees to rotate by with dot product, get axis with cross product)
			up_vector.set(0.0f, 1.0f, 0.0f);
			normal = GetTerrainNormal(unit->pos.x, unit->pos.y);

			rotate_axis = up_vector;
			rotate_axis.cross(normal);

			radians_to_rotate = acos(up_vector.dot(normal));

			mVMatrix.Rotate(radians_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

			// rotate the unit by how much it's supposed to be rotated
			mVMatrix.Rotate(unit->rotation, 0.0f, 1.0f, 0.0f);
			
			mVMatrix.Scale(0.0625f*type->size * unit->completeness / 100.0f, 0.0625f*type->size * unit->completeness / 100.0f, 0.0625f*type->size * unit->completeness / 100.0f);
			
			// translate upwards (in the direction of the terrain normal, because of the rotation before)
			mVMatrix.Translate(0.0f, 1.05f, 0.0f);
			
		}
		
		void UnitTransfNode::PostRender()
		{
			matrices[MATRIXTYPE_MODELVIEW] = mtxStack[MATRIXTYPE_MODELVIEW].top();
			mtxStack[MATRIXTYPE_MODELVIEW].pop();
		}

	}
}
