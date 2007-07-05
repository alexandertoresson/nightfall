#ifndef __TERRAIN_H_PRE__
#define __TERRAIN_H_PRE__

#define TERRAIN_HIGH_QUALITY

#ifdef DEBUG_DEP
#warning "terrain.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		struct XYZCoord;

		struct UVCoord;

		struct UVWCoord;

		struct RGBA;

		struct SphereNormal;

		struct HeightMap;
		
		extern GLuint terraintexture;
		
		extern float terrainOffsetX, terrainOffsetY;
		extern float terrainHeight;
		extern float waterLevel;
		extern HeightMap HeightMipmaps[2][32];
		extern int water_cur_front, water_cur_back;

		// overall quality of terrain; increase to increase terrain detail
		extern float quality;
		// water mipmap level, decrease to increase quality level
		extern int waterQuality;
		
		float GetTerrainHeight(float x, float y);
		float GetTerrainHeightHighestLevel(float x, float y);
		float GetTerrainHeightHighestLevel(int x, int y);
		
		int DrawTerrain();
		void DrawWater();
		void CalculateWater();
		float GetWaterDepth(int x, int y);
		bool BigSquareIsRendered(int x, int y);
		bool BigSquaresAreRendered(int x1, int y1, int x2, int y2);

		void InitFog();

		// Ladda v‰rlden ur bin‰r/textfil
		int LoadWorld(const char* file);

		// St‰nger ned v‰rlden
		void UnloadWorld(void);
		void UnloadTerrain();
		
	}
}

#define __TERRAIN_H_PRE_END__

#include "vector3d.h"
#include "unit.h"
#include "utilities.h"
#include "dimension.h"

#endif

#ifdef __UTILITIES_H_END__
#ifdef __VECTOR3D_H_END__
#ifdef __UNIT_H_PRE_END__
#ifdef __DIMENSION_H_END__

#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#ifdef DEBUG_DEP
#warning "terrain.h"
#endif

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

	}
}

#ifdef DEBUG_DEP
#warning "terrain.h-end"
#endif

#define __TERRAIN_H_END__

#endif
#endif
#endif
#endif
#endif
