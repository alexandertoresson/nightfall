#ifndef __UNITRENDER_H__
#define __UNITRENDER_H__

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
				std::map<gc_root_ptr<Unit>::type, gc_ptr<UnitNode> > unitToUnitNode;
				std::map<gc_ptr<Unit>, gc_ptr<UnitSelectionNode> > unitToSelectNode;
				std::map<gc_root_ptr<Projectile>::type, gc_ptr<ProjectileNode> > projToProjNode;

				std::set<gc_root_ptr<Unit>::type > unitScheduledForAddition;
				std::vector<gc_root_ptr<Unit>::type > unitScheduledForDeletion;

				std::set<gc_ptr<Unit> > unitScheduledForSelection;
				std::vector<gc_ptr<Unit> > unitScheduledForDeselection;
				
				std::set<gc_root_ptr<Projectile>::type > projScheduledForAddition;
				std::vector<gc_ptr<Projectile> > projScheduledForDeletion;

				gc_ptr<UnitType> buildOutlineType;
				IntPosition buildOutlinePosition;
			protected:
				virtual void PreRender();
			public:
				void ScheduleUnitNodeAddition(const gc_root_ptr<Unit>::type& unit);
				void ScheduleUnitNodeDeletion(const gc_root_ptr<Unit>::type& unit);

				void ScheduleSelection(const gc_ptr<Unit>& unit);
				void ScheduleDeselection(const gc_ptr<Unit>& unit);

				void ScheduleProjectileAddition(const gc_root_ptr<Projectile>::type& proj);
				void ScheduleProjectileDeletion(const gc_ptr<Projectile>& proj);

				void ScheduleBuildOutlineAddition(const gc_ptr<UnitType>& type, int x, int y);
				void ScheduleBuildOutlineDeletion();

				const gc_ptr<UnitNode>& GetUnitNode(const gc_ptr<Unit>& unit);

				static UnitMainNode instance;

				virtual void shade()
				{
					Scene::Graph::Node::shade();
					gc_shade_map_key_value(unitToUnitNode);
					gc_shade_map_key_value(unitToSelectNode);
					gc_shade_map(projToProjNode);
					gc_shade_container(unitScheduledForAddition);
					gc_shade_container(unitScheduledForDeletion);
					gc_shade_container(unitScheduledForSelection);
					gc_shade_container(unitScheduledForDeselection);
					gc_shade_container(unitScheduledForDeselection);
					buildOutlineType.shade();
				}
		};

		class UnitSubMeshRenderNode : public Scene::Render::OgreSubMeshNode
		{
			private:
				GLfloat CalculateMaterialModifier(const gc_ptr<Unit>& unit, GLfloat (&mod)[2][2]);
				std::vector<Utilities::Uniform*> uniforms;
			protected:
				gc_root_ptr<Unit>::type unit;
				virtual void PreRender();
				virtual void Render();
			public:
				UnitSubMeshRenderNode(const gc_root_ptr<Unit>::type& unit, const gc_ptr<Utilities::OgreSubMesh>& submesh);
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
				gc_root_ptr<Unit>::type unit;
				virtual void ApplyMatrix();
				virtual void PostRender();
				virtual void Traverse();
			public:
				UnitNode(const gc_root_ptr<Unit>::type& unit);
				
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
				gc_root_ptr<Unit>::type unit;
			public:
				UnitSelectionNode(const gc_root_ptr<Unit>::type& unit);
				
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

