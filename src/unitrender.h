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
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance);
		Utilities::Vector3D GetUnitWindowPos(Unit* unit);

		class UnitMainNode : public Scene::Graph::Node
		{
			private:

				UnitMainNode() : listsMutex(SDL_CreateMutex())
				{
					
				}

				SDL_mutex* listsMutex;
				std::map<Unit*, UnitNode*> unitToUnitNode;
				std::map<Unit*, UnitSelectionNode*> unitToSelectNode;
				std::map<Projectile*, ProjectileNode*> projToProjNode;

				std::set<Unit*> unitScheduledForAddition;
				std::vector<Unit*> unitScheduledForDeletion;

				std::set<Unit*> unitScheduledForSelection;
				std::vector<Unit*> unitScheduledForDeselection;
				
				std::set<Projectile*> projScheduledForAddition;
				std::vector<Projectile*> projScheduledForDeletion;

				UnitType* buildOutlineType;
				IntPosition buildOutlinePosition;
			protected:
				virtual void PreRender();
			public:
				void ScheduleUnitNodeAddition(Unit* unit);
				void ScheduleUnitNodeDeletion(Unit* unit);

				void ScheduleSelection(Unit* unit);
				void ScheduleDeselection(Unit* unit);

				void ScheduleProjectileAddition(Projectile* proj);
				void ScheduleProjectileDeletion(Projectile* proj);

				void ScheduleBuildOutlineAddition(UnitType* type, int x, int y);
				void ScheduleBuildOutlineDeletion();

				UnitNode* GetUnitNode(Unit* unit);

				static UnitMainNode instance;
		};

		class UnitSubMeshRenderNode : public Scene::Render::OgreSubMeshNode
		{
			private:
				GLfloat CalculateMaterialModifier(Unit* unit, GLfloat (&mod)[2][2]);
				std::vector<Utilities::Uniform*> uniforms;
			protected:
				Unit* unit;
				virtual void PreRender();
				virtual void Render();
			public:
				UnitSubMeshRenderNode(Unit* unit, Utilities::OgreSubMesh* submesh);
				~UnitSubMeshRenderNode();
				int i;
		};

		class UnitNode : public Scene::Graph::Node
		{
			protected:
				Unit* unit;
				virtual void ApplyMatrix();
				virtual void PostRender();
				virtual void Traverse();
			public:
				UnitNode(Unit* unit);
		};

		class UnitSelectionNode : public Scene::Graph::Node
		{
			protected:
				virtual void ApplyMatrix();
				virtual void Render();
				virtual void PostRender();
				virtual void Traverse();
				Unit* unit;
			public:
				UnitSelectionNode(Unit* unit);
		};

		class ProjectileNode : public Scene::Render::OgreMeshNode
		{
			protected:
				virtual void ApplyMatrix();
				virtual void PostRender();
				virtual void Traverse();
				Projectile* proj;
			public:
				ProjectileNode(Projectile* proj);
		};

		class BuildOutlineNode : public Scene::Graph::Node
		{
			protected:
				virtual void Render();
				UnitType* type;
				IntPosition pos;

				BuildOutlineNode() : type(NULL)
				{
					
				}
			public:
				void Set(UnitType* type, IntPosition pos);
				static BuildOutlineNode instance;
		};

	}
}

#ifdef DEBUG_DEP
#warning "unitrender.h-end"
#endif

#endif

