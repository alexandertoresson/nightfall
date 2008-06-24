#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#ifdef DEBUG_DEP
#warning "utilities.h"
#endif

#include "utilities-pre.h"

#include "vector3d.h"

#include <fstream>
#include <vector>

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

}

#ifdef DEBUG_DEP
#warning "utilities.h-end"
#endif

#define __UTILITIES_H_END__

#endif
