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
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#ifdef DEBUG_DEP
#warning "utilities.h"
#endif

#include "utilities-pre.h"

#include "vector3d.h"

#include <fstream>
#include <vector>
#include <sstream>

namespace Utilities
{
	// Loads A Texture and converts it into OpenGL texture, it supports all formats that SDL_image can support with or without alpha.
	// Gives a 32 bit OpenGL texture
	GLuint LoadTexture(std::string path);
	SDL_Surface *LoadImage(std::string path);

	GLuint CreateGLTexture(SDL_Surface*);
		
	void WorldCoordToWindowCoord(Vector3D world_coord, Vector3D &windows_coord);
	void WindowCoordToVector(GLdouble clickx, GLdouble clicky, Vector3D &pos_vector_near, Vector3D &pos_vector_far);
	
	bool CheckLineIntersectTri(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos);
	bool CheckLineIntersect(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos);
	
	int SwitchTo2DViewport(float w, float h);
	int RevertViewport();
	
	Vector3D GetOGLPos(int x, int y);
	
	float InterpolateCatmullRom(float v0, float v1, float v2, float v3, float x);
	Vector3D InterpolateCatmullRom(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x);
	float InterpolateCatmullRomBounded(float v0, float v1, float v2, float v3, float x);
	Vector3D InterpolateCatmullRomBounded(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x);
		
	void PrintGLError();

	template <class T>
	inline std::string ToString(const T& t)
	{
		std::stringstream ss;
		ss << t;
		return ss.str();
	}
}

#ifdef DEBUG_DEP
#warning "utilities.h-end"
#endif

#define __UTILITIES_H_END__

#endif
