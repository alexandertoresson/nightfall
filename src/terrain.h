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

		struct HeightMap
		{
			gc_array<float, 2> heights;
			gc_array<XYZCoord, 2> normals;
			gc_array<Uint16, 2> steepness;
			gc_array<float, 3> water;
			gc_array<bool, 2> squareHasWater;
			gc_array<XYZCoord, 2> waterNormals;
			gc_array<bool, 2> bigSquareHasWater;

			void shade()
			{
				heights.shade();
				normals.shade();
				steepness.shade();
				water.shade();
				squareHasWater.shade();
				waterNormals.shade();
				bigSquareHasWater.shade();
			}
		};
		
		extern gc_root_ptr<HeightMap>::type heightMap;

		Utilities::Vector3D GetTerrainNormal(float x, float y);
		
		Utilities::Vector3D GetTerrainCoord(float x, float y);
		Dimension::Position GetPosition(Utilities::Vector3D* v);

		class TerrainNode : public Scene::Graph::Node
		{
			private:
				TerrainNode();
			protected:
				virtual void Render();
			public:
				static TerrainNode instance;
		};

	}
}

#ifdef DEBUG_DEP
#warning "terrain.h-end"
#endif

#endif
