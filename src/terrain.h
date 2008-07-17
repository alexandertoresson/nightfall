#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#ifdef DEBUG_DEP
#warning "terrain.h"
#endif

#include "terrain-pre.h"

#include "vector3d.h"
#include "unit-pre.h"
#include "dimension.h"
#include "scenegraph.h"
#include "render-pre.h"

namespace Game
{
	namespace Dimension
	{
		extern float      waterLevel;
		extern float      waterHeight;

		struct XYZCoord : public gc_null_shader<XYZCoord>
		{
			float x;
			float y;
			float z;
		};

		struct TerrainBSVBOs : gc_null_shader<TerrainBSVBOs>
		{
			Scene::Render::VBO positions;
			Scene::Render::VBO normals;
		};

		struct HeightMap
		{
			gc_array<float, 2> heights;
			gc_array<XYZCoord, 2> normals;
			gc_array<Uint16, 2> steepness;
			gc_array<float, 3> water;
			gc_array<bool, 2> squareHasWater;
			gc_array<XYZCoord, 2> waterNormals;
			gc_array<bool, 2> bigSquareHasWater;

			gc_array<TerrainBSVBOs, 2> bsvbos;
			Scene::Render::VBO index;
			Scene::Render::VBO light;
			Scene::Render::VBO waterBack;
			Scene::Render::VBO waterFront;

			void shade()
			{
				heights.shade();
				normals.shade();
				steepness.shade();
				water.shade();
				squareHasWater.shade();
				waterNormals.shade();
				bigSquareHasWater.shade();
				bsvbos.shade();
			}
		};
		
		extern gc_root_ptr<HeightMap>::type heightMap;

		Utilities::Vector3D GetTerrainNormal(float x, float y);
		
		Utilities::Vector3D GetTerrainCoord(float x, float y);
		Dimension::Position GetPosition(Utilities::Vector3D* v);

		class TerrainNode : public Scene::Render::GLStateNode
		{
			public:
				TerrainNode();
			protected:
				virtual void Render();
		};

		class WaterNode : public Scene::Render::GLStateNode
		{
			public:
				WaterNode();
			protected:
				virtual void Render();
		};

	}
}

#ifdef DEBUG_DEP
#warning "terrain.h-end"
#endif

#endif
