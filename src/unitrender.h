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

namespace Game
{
	namespace Dimension
	{
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance);
		Utilities::Vector3D GetUnitWindowPos(Unit* unit);

		class UnitTransfNode : public Scene::Graph::Node
		{
			protected:
				virtual void PreRender();
				virtual void PostRender();
				Unit* unit;
			public:
				UnitTransfNode(Unit* unit);
		};

		class UnitRenderNode : public Scene::Render::GeometryNode
		{
			protected:
				virtual void Render();
		};

		class ProjectileNode : public Scene::Render::GeometryNode
		{
			protected:
				virtual void PreRender();
				virtual void Render();
				virtual void PostRender();
				Projectile* proj;
			public:
				ProjectileNode(Projectile* proj);
		};

		class OutlineNode : public UnitTransfNode
		{
			protected:
				virtual void Render();
				UnitType* type;
				IntPosition pos;
			public:
				OutlineNode(UnitType* type, IntPosition pos);
		};

	}
}

#ifdef DEBUG_DEP
#warning "unitrender.h-end"
#endif

#endif

