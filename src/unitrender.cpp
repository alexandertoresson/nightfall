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
#include "matrix4x4.h"
#include "camera.h"
#include <iostream>
#include <cassert>

using namespace std;

namespace Game
{
	namespace Dimension
	{
		
		GLfloat    unitMaterialModifiers[2][2] = {
		                                           {
		                                             0.0f,   // not seen, not lighted
		                                             0.5f    // seen, not lighted
		                                           },
		                                           {
		                                             0.0f,   // not seen, lighted
		                                             1.0f    // seen, lighted
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

		// Check whether a click at (clickx, clicky) hit unit
		bool DoesHitUnit(const gc_ptr<Unit>& unit, int clickx, int clicky, float& distance)
		{
			gc_ptr<UnitType> type = unit->type;
			const gc_ptr<Utilities::OgreMesh>& mesh = type->mesh;
			Utilities::Vector3D near_plane, far_plane;
			bool result = false;

			if (!mesh)
				return false;

			glPushMatrix();

				const gc_ptr<UnitNode>& unitNode = UnitMainNode::GetInstance()->GetUnitNode(unit);

				if (unitNode)
				{
					unitNode->GetMatrix(Scene::Graph::Node::MATRIXTYPE_MODELVIEW).Apply();
					
					WindowCoordToVector((GLdouble) clickx, (GLdouble) clicky, near_plane, far_plane);

					result = unit->type->mesh->CheckRayIntersect(near_plane, far_plane, distance);
				}
				
			glPopMatrix();

			return result;
		}
		
		// get the screen coord of the middle (0.0, 0.0, 0.0) of unit
		Utilities::Vector3D GetUnitWindowPos(const gc_ptr<Unit>& unit)
		{
			Utilities::Vector3D win_vector(-1000, -1000, -1000);
			glPushMatrix();

				const gc_ptr<UnitNode>& unitNode = UnitMainNode::GetInstance()->GetUnitNode(unit);

				if (unitNode)
				{
					unitNode->GetMatrix(Scene::Graph::Node::MATRIXTYPE_MODELVIEW).Apply();

					WorldCoordToWindowCoord(Utilities::Vector3D(0.0f, 0.0f, 0.0f), win_vector);
				}

			glPopMatrix();

			return win_vector;

		}

		// get what unit was clicked, if any
		gc_ptr<Unit> GetUnitClicked(int clickx, int clicky, int map_x, int map_y)
		{
			gc_ptr<Unit> cur_unit;
			float cur_dist = 1e10, dist;
			const std::set<gc_ptr<Dimension::Unit> >& units = UnitMainNode::GetInstance()->GetUnits();
			for (set<gc_ptr<Dimension::Unit> >::iterator it = units.begin(); it != units.end(); it++)
			{
				const gc_ptr<Unit>& unit = *it;
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
/*

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						
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
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		*/

		void UnitMainNode::ScheduleUnitNodeAddition(const gc_ptr<Unit>& unit)
		{
			SDL_LockMutex(listsMutex);
			unitChanges.produce(make_add_item(unit));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleUnitNodeDeletion(const gc_ptr<Unit>& unit)
		{
			SDL_LockMutex(listsMutex);
			unitChanges.produce(make_del_item(unit));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleSelection(const gc_ptr<Unit>& unit)
		{
			SDL_LockMutex(listsMutex);
			std::cout << "select " << std::endl;
			unitSelectionChanges.produce(make_add_item(unit));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleDeselection(const gc_ptr<Unit>& unit)
		{
			SDL_LockMutex(listsMutex);
			unitSelectionChanges.produce(make_del_item(unit));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleProjectileAddition(const gc_ptr<Projectile>& proj)
		{
			SDL_LockMutex(listsMutex);
			projChanges.produce(make_add_item(proj));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleProjectileDeletion(const gc_ptr<Projectile>& proj)
		{
			SDL_LockMutex(listsMutex);
			projChanges.produce(make_del_item(proj));
			SDL_UnlockMutex(listsMutex);
		}

		void UnitMainNode::ScheduleBuildOutlineAddition(const gc_ptr<UnitType>& type, int x, int y)
		{
			buildOutlineType = type;
			buildOutlinePosition.x = x;
			buildOutlinePosition.y = y;
		}

		void UnitMainNode::ScheduleBuildOutlineDeletion()
		{
			buildOutlineType.reset();
		}

		void UnitMainNode::PreRender()
		{
//			SDL_LockMutex(listsMutex);
			
			// Removals
			/////////////////////////////////////////////////////////////////////////


			while (AddDelItem<gc_ptr<Unit> > item = unitChanges.consume())
			{
				const gc_ptr<Unit>& unit = item.GetValue();
				if (item.IsAdd())
				{
					const gc_root_ptr<UnitNode>::type unitNode = new UnitNode(unit);
	//				std::cout << "add " << unit->GetHandle() << " (" << unit << ")" << std::endl;
					AddChild(unitNode);
					unitToUnitNode[unit] = unitNode;
					units.insert(unit);
				}
				else
				{
					const gc_ptr<UnitNode>& unitNode = unitToUnitNode[unit];
	//				std::cout << "delete " << unit->GetHandle() << " (" << unit << ")" << std::endl;
					if (unitNode)
					{
						unitNode->DeleteTree();
						unitToUnitNode.erase(unit);
						units.erase(unit);
					}
				}
			}

			while (AddDelItem<gc_ptr<Unit> > item = unitSelectionChanges.consume())
			{
				const gc_ptr<Unit>& unit = item.GetValue();
				if (item.IsAdd())
				{
					gc_ptr<UnitNode>& unitNode = unitToUnitNode[unit];
					if (unitNode && !unitToSelectNode[unit])
					{
						gc_root_ptr<UnitSelectionNode>::type selectNode = new UnitSelectionNode(unit);
						unitNode->AddChild(selectNode);
						unitToSelectNode[unit] = selectNode;
					}
				}
				else
				{
					const gc_ptr<UnitSelectionNode>& selectNode = unitToSelectNode[unit];
					if (selectNode)
					{
						selectNode->DeleteTree();
						unitToSelectNode.erase(unit);
					}
				}
			}
			
			while (AddDelItem<gc_ptr<Projectile> > item = projChanges.consume())
			{
				const gc_ptr<Projectile>& proj = item.GetValue();
				if (item.IsAdd())
				{
					gc_root_ptr<ProjectileNode>::type projNode = new ProjectileNode(proj);
					AddChild(projNode);
					projToProjNode[proj] = projNode;
				}
				else
				{
					gc_ptr<ProjectileNode>& projNode = projToProjNode[proj];
					if (projNode)
					{
						projNode->DeleteTree();
						projToProjNode.erase(proj);
					}
				}
			}
			
			BuildOutlineNode::instance.Set(buildOutlineType, buildOutlinePosition);

//			SDL_UnlockMutex(listsMutex);
		}

		const gc_ptr<UnitNode>& UnitMainNode::GetUnitNode(const gc_ptr<Unit>& unit)
		{
			return unitToUnitNode[unit];
		}

		const std::set<gc_ptr<Unit> >& UnitMainNode::GetUnits()
		{
			return units;
		}

		void UnitMainNode::Reset()
		{
			instance = new UnitMainNode;
		}

		gc_root_ptr<UnitMainNode>::type& UnitMainNode::GetInstance()
		{
			return instance;
		}

		gc_root_ptr<UnitMainNode>::type UnitMainNode::instance;

		void UnitNode::ApplyMatrix()
		{
			const gc_ptr<UnitType> type = unit->type;
			float unit_x, unit_y, unit_z, radians_to_rotate;
			Utilities::Vector3D up_vector, normal, rotate_axis;

			PushMatrix(MATRIXTYPE_MODELVIEW);

			Utilities::Matrix4x4& mVMatrix = matrices[MATRIXTYPE_MODELVIEW];

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

			PushMatrix(MATRIXTYPE_MODELVIEW);

			mVMatrix.Scale(0.0625f);

			// rotate so the unit will be placed correctly onto possibly leaning ground, by rotating by the difference between
			// the up vector and the terrain normal (get degrees to rotate by with dot product, get axis with cross product)
			up_vector.set(0.0f, 1.0f, 0.0f);
			normal = GetTerrainNormal(unit->pos.x, unit->pos.y);

			rotate_axis = up_vector;
			rotate_axis.cross(normal);

			radians_to_rotate = acos(up_vector.dot(normal));

			mVMatrix.Rotate(radians_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

			// rotate the unit by how much it's supposed to be rotated
			mVMatrix.Rotate(unit->rotation * (PI / 180), 0.0f, 1.0f, 0.0f);
			
			if (unit->type->mesh->transforms.size())
			{
				for (std::vector<gc_ptr<Scene::Render::MeshTransformation> >::iterator it = unit->type->mesh->transforms.begin(); it != unit->type->mesh->transforms.end(); it++)
				{
					(*it)->Apply(mVMatrix);
				}
			}

			mVMatrix.Scale(unit->completeness / 100.0f);
			
		}
		
		void UnitNode::PostRender()
		{
			PopMatrix(MATRIXTYPE_MODELVIEW);
			PopMatrix(MATRIXTYPE_MODELVIEW);
		}

		void UnitNode::Traverse()
		{
			if (UnitIsRendered(unit, currentPlayerView))
			{
				PreRender();
				TraverseAllChildren();
				PostRender();
			}
		}

		UnitSubMeshRenderNode::UnitSubMeshRenderNode(const gc_ptr<Unit>& unit, const gc_ptr<Utilities::OgreSubMesh>& submesh) : OgreSubMeshNode(submesh), unit(unit)
		{
			int i = 0;
			assert(unit);
			for (std::vector<Utilities::Colour>::iterator it = unit->owner->colours.begin(); it != unit->owner->colours.end(); it++, i++)
			{
				std::stringstream ss;
				ss << "playerColour" << i;
				this->uniforms.push_back(new Utilities::Uniform4f(ss.str(), *it));
			}
		}

		UnitSubMeshRenderNode::~UnitSubMeshRenderNode()
		{
			while (uniforms.size())
			{
				std::vector<Utilities::Uniform*>::iterator it = uniforms.begin();
				delete *it;
				uniforms.erase(it);
			}
		}

		GLfloat UnitSubMeshRenderNode::CalculateMaterialModifier(const gc_ptr<Unit>& unit, GLfloat (&mod)[2][2])
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
						temp += mod[is_lighted][is_seen];
						num++;
					}
				}
			}
			if (num)
				return temp / (float) num;
			return 0.0f;
		}

		void UnitSubMeshRenderNode::PreRender()
		{
			OgreSubMeshNode::PreRender();
			if (myGLState->material && myGLState->material->program)
			{
				for (std::vector<Utilities::Uniform*>::iterator it = uniforms.begin(); it != uniforms.end(); it++)
				{
					GLint id = glGetUniformLocationARB(myGLState->material->program, (*it)->name.c_str());
					if (id != -1)
					{
						(*it)->Set(id);
					}
					else
					{
	//						std::cout << "Uniform " << it->name << " not found!" << std::endl;
					}
				}
			}
			
			GLint id = glGetUniformLocationARB(myGLState->material->program, "materialModifier");

			Utilities::UniformFloat("materialModifier", CalculateMaterialModifier(unit, unitMaterialModifiers)).Set(id);

		}

		void UnitSubMeshRenderNode::Render()
		{
			OgreSubMeshNode::Render();
		}

		UnitNode::UnitNode(const gc_ptr<Unit>& unit)
		{
			const gc_ptr<Utilities::OgreMesh>& mesh = unit->type->mesh;
			this->unit = unit;
			for (std::vector<gc_ptr<Utilities::OgreSubMesh> >::iterator it = mesh->submeshes.begin(); it != mesh->submeshes.end(); it++)
			{
				this->AddChild(new UnitSubMeshRenderNode(unit, *it));
			}
		}

		UnitSelectionNode::UnitSelectionNode(const gc_ptr<Unit>& unit) : unit(unit)
		{
			
		}

		void UnitSelectionNode::Traverse()
		{
			if (UnitIsRendered(unit, currentPlayerView))
			{
				PreRender();
				Render();
				PostRender();
			}
		}

		void UnitSelectionNode::ApplyMatrix()
		{
			Utilities::Vector3D near_plane, far_plane, up, right, look, up_vector, rotate_axis, normal, window_coord;
			Utilities::Matrix4x4 matrix;
			
			PopMatrix(MATRIXTYPE_MODELVIEW);
			PushMatrix(MATRIXTYPE_MODELVIEW);

			Utilities::Matrix4x4& mVMatrix = matrices[MATRIXTYPE_MODELVIEW];

			mVMatrix.Translate(0.0f, unit->type->height, 0.0f);

			matrices[MATRIXTYPE_MODELVIEW].Apply();

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

			matrix.matrix[0][0] = right.x;
			matrix.matrix[0][1] = right.y;
			matrix.matrix[0][2] = right.z;
			matrix.matrix[0][3] = 0.0;

			matrix.matrix[1][0] = up.x;
			matrix.matrix[1][1] = up.y;
			matrix.matrix[1][2] = up.z;
			matrix.matrix[1][3] = 0.0;

			matrix.matrix[2][0] = look.x;
			matrix.matrix[2][1] = look.y;
			matrix.matrix[2][2] = look.z;
			matrix.matrix[2][3] = 0.0;

			matrix.matrix[3][0] = 0.0;
			matrix.matrix[3][1] = 0.0;
			matrix.matrix[3][2] = 0.0;
			matrix.matrix[3][3] = 1.0;

			mVMatrix.MulBy(matrix);

			mVMatrix.Scale(0.1);
					
		}

		void UnitSelectionNode::PostRender()
		{
		}

		void UnitSelectionNode::Render()
		{
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glUseProgramObjectARB(0);

			matrices[MATRIXTYPE_MODELVIEW].Apply();

			// s = start
			// e = end
			// i = inner
			float x_e = (float) unit->type->widthOnMap / 2.0f;
			float x_s = -x_e;
			float y_s = 0.0;
			float y_e = 0.2f;
			float progress = 0;

			glBegin(GL_QUADS);
			
				progress = (float) unit->health / (float) unit->type->maxHealth * (x_e - x_s);

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
					if (unit->pMovementData->action.goal.unit)
					{
						const gc_ptr<Unit>& target = unit->pMovementData->action.goal.unit;
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
			
			glEnable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);

		}

		ProjectileNode::ProjectileNode(const gc_ptr<Projectile>& proj) : OgreMeshNode(proj->type->mesh)
		{
			this->proj = proj;
		}

		void ProjectileNode::ApplyMatrix()
		{
			const gc_ptr<ProjectileType>& type = proj->type;
			float radians_to_rotate;
			Utilities::Vector3D up_vector, rotate_axis;
			
			PushMatrix(MATRIXTYPE_MODELVIEW);

			Utilities::Matrix4x4& mVMatrix = matrices[MATRIXTYPE_MODELVIEW];
			
			mVMatrix.Translate(proj->pos.x * 0.125f - terrainOffsetX, proj->pos.z, proj->pos.y * 0.125f - terrainOffsetY);

			// scale down
			glScalef(0.0625f*type->size, 0.0625f*type->size, 0.0625f*type->size);
				
			if (proj->type->mesh->transforms.size())
			{
				for (std::vector<gc_ptr<Scene::Render::MeshTransformation> >::iterator it = proj->type->mesh->transforms.begin(); it != proj->type->mesh->transforms.end(); it++)
				{
					(*it)->Apply(mVMatrix);
				}
			}

			up_vector.set(0.0f, 1.0f, 0.0f);

			rotate_axis = up_vector;
			rotate_axis.cross(proj->direction);

			radians_to_rotate = acos(up_vector.dot(proj->direction));

			glRotatef(radians_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

		}

		void ProjectileNode::PostRender()
		{
			PopMatrix(MATRIXTYPE_MODELVIEW);
		}
				
		void ProjectileNode::Traverse()
		{
			if (SquareIsVisible(currentPlayerView, (int) proj->pos.x, (int) proj->pos.y))
			{
				PreRender();
				TraverseAllChildren();
				PostRender();
			}
		}

		void BuildOutlineNode::Render()
		{
			if (type)
			{
				Utilities::Vector3D ter[2][2];
				int end_x, end_y;
				bool outofbounds = false;
				int start_x, start_y;
				GetTypeUpperLeftCorner(type, pos.x, pos.y, start_x, start_y);
				bool suitable = IsSuitableForBuilding(type, start_x, start_y);
				end_x = start_x + type->widthOnMap-1;
				end_y = start_y + type->heightOnMap-1;

				matrices[MATRIXTYPE_MODELVIEW].Apply();

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
		}
		
		void BuildOutlineNode::Set(const gc_ptr<UnitType>& type, IntPosition pos)
		{
			this->type = type;
			this->pos = pos;
		}
				
		BuildOutlineNode BuildOutlineNode::instance;
	}
}
