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

namespace Game
{
	namespace Dimension
	{
		extern GLfloat    terrainMaterialAmbientDiffuse[2][2][4];
		extern GLfloat    terrainMaterialSpecular[4];
		extern GLfloat    terrainMaterialEmission[4];
		extern GLfloat    terrainMaterialShininess;
		
		extern float      waterLevel;
		extern float      waterHeight;
		extern GLfloat    waterMaterialAmbientDiffuse[2][2][4];
		extern GLfloat    waterMaterialSpecular[4];
		extern GLfloat    waterMaterialEmission[4];
		extern GLfloat    waterMaterialShininess;
		extern GLfloat    waterColor[3];

		struct XYZCoord
		{
			float x;
			float y;
			float z;
		};

		struct UVCoord
		{
			float u;
			float v;
		};

		struct SphereNormal
		{
			float phi;
			float theta;
		};

		struct UVWCoord
		{
			float u;
			float v;
			float w;
		};
		
		struct RGBA
		{
			float r;
			float g;
			float b;
			float a;
		};

		struct HeightMap
		{
			float** ppHeights;
			UVWCoord*** ppTexCoords;
#ifdef TERRAIN_HIGH_QUALITY
			SphereNormal*** ppNormals;
#else
			XYZCoord*** ppNormals;
#endif
			bool** ppSquareHasWater;
		};
		
		Utilities::Vector3D GetTerrainNormal(float x, float y);
		
		Utilities::Vector3D GetTerrainCoord(float x, float y);
		Utilities::Vector3D GetTerrainCoordHighestLevel(float x, float y);
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
