#ifndef __TERRAIN_H_PRE__
#define __TERRAIN_H_PRE__

#define TERRAIN_HIGH_QUALITY

#ifdef DEBUG_DEP
#warning "terrain.h-pre"
#endif

#include "sdlheader.h"
#include <string>

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
		
		extern float terrainOffsetX, terrainOffsetY;
		extern float terrainHeight;
		extern float waterLevel;
		extern int water_cur_front, water_cur_back;

		// overall quality of terrain; increase to increase terrain detail
		extern float quality;
		// water mipmap level, decrease to increase quality level
		extern int waterQuality;
		
		float GetTerrainHeight(float x, float y);
		float GetTerrainHeight(int x, int y);
		
		int DrawTerrain();
		void DrawWater();
		void CalculateWater();
		float GetWaterDepth(int x, int y);
		bool BigSquareIsRendered(int x, int y);
		bool BigSquaresAreRendered(int x1, int y1, int x2, int y2);

		void InitFog();

		// Ladda världen ur bin'r/textfil
		int LoadWorld(std::string filename);

		void UnloadTerrain();
		
	}
}

#endif

