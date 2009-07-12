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
#ifndef UNITRENDER_H
#define UNITRENDER_H

#ifdef DEBUG_DEP
#warning "unitrender.h"
#endif

#include "unitrender-pre.h"

#include "unit-pre.h"
#include "unittype-pre.h"
#include "sdlheader.h"
#include "vector3d.h"
#include "scenegraph.h"
#include "render.h"
#include "dimension.h"
#include "materialxml-pre.h"
#include "lockfreequeue.h"
#include <map>
#include <vector>
#include <set>

namespace Game
{
	namespace Dimension
	{
		gc_ptr<Unit> GetUnitClicked(int clickx, int clicky, int map_x, int map_y);

		bool DoesHitUnit(const gc_ptr<Unit>& unit, int clickx, int clicky, float& distance);
		Utilities::Vector3D GetUnitWindowPos(const gc_ptr<Unit>& unit);

		class UnitMainNode : public Scene::Graph::Node
		{
			private:

				UnitMainNode() : listsMutex(SDL_CreateMutex())
				{
					
				}

				SDL_mutex* listsMutex;
				std::map<gc_ptr<Unit>, gc_ptr<UnitNode> > unitToUnitNode;
				std::map<gc_ptr<Unit>, gc_ptr<UnitSelectionNode> > unitToSelectNode;
				std::map<gc_ptr<Projectile>, gc_ptr<ProjectileNode> > projToProjNode;
				std::set<gc_ptr<Unit> > units;

				template <typename T>
				class AddDelItem
				{
					public:
						enum Type
						{
							ADD,
							DEL
						};
					private:
						Type type;
						T val;
						bool valid;

					public:
						AddDelItem() : valid(false) {}
						AddDelItem(Type type, T val) : type(type), val(val), valid(true) {}

						operator bool() const
						{
							return valid;
						}

						bool IsAdd()
						{
							return type == ADD;
						}

						bool IsDel()
						{
							return !IsAdd();
						}

						const T& GetValue()
						{
							return val;
						}

						void shade() const
						{
							val.shade();
						}
				};

				template <typename T>
				AddDelItem<T> make_add_item(T val)
				{
					return AddDelItem<T>(AddDelItem<T>::ADD, val);
				}

				template <typename T>
				AddDelItem<T> make_del_item(T val)
				{
					return AddDelItem<T>(AddDelItem<T>::DEL, val);
				}

				lockfreequeue<AddDelItem<gc_ptr<Unit> > > unitChanges;
				lockfreequeue<AddDelItem<gc_ptr<Unit> > > unitSelectionChanges;
				lockfreequeue<AddDelItem<gc_ptr<Projectile> > > projChanges;

				gc_ptr<UnitType> buildOutlineType;
				IntPosition buildOutlinePosition;
				
				static gc_root_ptr<UnitMainNode>::type instance;

			protected:
				virtual void PreRender();
			public:
				void ScheduleUnitNodeAddition(const gc_ptr<Unit>& unit);
				void ScheduleUnitNodeDeletion(const gc_ptr<Unit>& unit);

				void ScheduleSelection(const gc_ptr<Unit>& unit);
				void ScheduleDeselection(const gc_ptr<Unit>& unit);

				void ScheduleProjectileAddition(const gc_ptr<Projectile>& proj);
				void ScheduleProjectileDeletion(const gc_ptr<Projectile>& proj);

				void ScheduleBuildOutlineAddition(const gc_ptr<UnitType>& type, int x, int y);
				void ScheduleBuildOutlineDeletion();

				const gc_ptr<UnitNode>& GetUnitNode(const gc_ptr<Unit>& unit);

				const std::set<gc_ptr<Unit> >& GetUnits();

				static void Reset();
				static gc_root_ptr<UnitMainNode>::type& GetInstance();

				virtual void shade()
				{
					Scene::Graph::Node::shade();
					gc_shade_map_key_value(unitToUnitNode);
					gc_shade_map_key_value(unitToSelectNode);
					gc_shade_map_key_value(projToProjNode);
					gc_shade_container(unitChanges);
					gc_shade_container(unitSelectionChanges);
					gc_shade_container(projChanges);
					buildOutlineType.shade();
				}
		};

		class UnitSubMeshRenderNode : public Scene::Render::OgreSubMeshNode
		{
			private:
				GLfloat CalculateMaterialModifier(const gc_ptr<Unit>& unit, GLfloat (&mod)[2][2]);
				std::vector<Utilities::Uniform*> uniforms;
			protected:
				gc_ptr<Unit> unit;
				virtual void PreRender();
				virtual void Render();
			public:
				UnitSubMeshRenderNode(const gc_ptr<Unit>& unit, const gc_ptr<Utilities::OgreSubMesh>& submesh);
				~UnitSubMeshRenderNode();
				int i;

				virtual void shade()
				{
					Scene::Render::OgreSubMeshNode::shade();
					unit.shade();
				}
		};

		class UnitNode : public Scene::Graph::Node
		{
			protected:
				gc_ptr<Unit> unit;
				virtual void ApplyMatrix();
				virtual void PostRender();
				virtual void Traverse();
			public:
				UnitNode(const gc_ptr<Unit>& unit);
				
				virtual void shade()
				{
					Scene::Graph::Node::shade();
					unit.shade();
				}
		};

		class UnitSelectionNode : public Scene::Graph::Node
		{
			protected:
				virtual void ApplyMatrix();
				virtual void Render();
				virtual void PostRender();
				virtual void Traverse();
				gc_ptr<Unit> unit;
			public:
				UnitSelectionNode(const gc_ptr<Unit>& unit);
				
				virtual void shade()
				{
					Scene::Graph::Node::shade();
					unit.shade();
				}
		};

		class ProjectileNode : public Scene::Render::OgreMeshNode
		{
			protected:
				virtual void ApplyMatrix();
				virtual void PostRender();
				virtual void Traverse();
				gc_ptr<Projectile> proj;
			public:
				ProjectileNode(const gc_ptr<Projectile>& proj);
		};

		class BuildOutlineNode : public Scene::Graph::Node
		{
			protected:
				virtual void Render();
				gc_ptr<UnitType> type;
				IntPosition pos;

				BuildOutlineNode()
				{
					
				}
			public:
				void Set(const gc_ptr<UnitType>& type, IntPosition pos);
				static BuildOutlineNode instance;
				
				virtual void shade()
				{
					Scene::Graph::Node::shade();
					type.shade();
				}
		};

	}
}

#ifdef DEBUG_DEP
#warning "unitrender.h-end"
#endif

#endif

