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
#ifndef TERRAIN_H_PRE
#define TERRAIN_H_PRE

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
		bool LoadTerrainXML(std::string name);

		void UnloadTerrain();
		
	}
}

#endif

