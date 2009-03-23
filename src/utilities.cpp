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
#include "vector3d.h"

#include "utilities.h"

#include "vector3d.h"
#include "sdlheader.h"
#include "networking.h"
#include "vfs.h"
#include "errors.h"
#include "game.h"
#include "console.h"
#include "window.h"
#include "unit.h"
#include <cmath>
#include <cassert>
#include <iostream>

using namespace Window;
using namespace std;

namespace Utilities
{
	bool IsOGLExtensionSupported(const char *extension)
	{
		const GLubyte *extensions = NULL;
		const GLubyte *start;
		GLubyte *where, *terminator;

		/* Extension names should not have spaces. */
		where = (GLubyte *) strchr(extension, ' ');
		if (where || *extension == '\0')
			return false;
		
		extensions = glGetString(GL_EXTENSIONS);
		/* It takes a bit of care to be fool-proof about parsing the
		OpenGL extensions string. Don't be fooled by sub-strings,
		etc. */
		start = extensions;
		for (;;) {
			where = (GLubyte *) strstr((const char *) start, extension);
			if (!where)
				break;
			terminator = where + strlen(extension);
			if (where == start || *(where - 1) == ' ')
				if (*terminator == ' ' || *terminator == '\0')
					return true;
			start = terminator;
		}
		return false;
	}

	// get the world coordinate of a click at (x, y)
	// takes depth buffer value at (x, y) into account
	Vector3D GetOGLPos(int x, int y)
	{
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX = 0, winY = 0, winZ = 0;
		GLdouble posX = 0, posY = 0, posZ = 0;

		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport );

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

		gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		return Utilities::Vector3D((float) posX, (float) posY, (float) posZ);
	}		
		
	// Convert a coord in the current modelview coordinate space to a 2d position on screen
	void WorldCoordToWindowCoord(Vector3D world_coord, Vector3D &windows_coord)
	{
		GLdouble mvmatrix[16];
		GLdouble projmatrix[16];
		GLint viewport[4];
		double wx, wy, wz;

		glGetIntegerv(GL_VIEWPORT, viewport);	
		glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
		
		gluProject (world_coord.x, world_coord.y, world_coord.z, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
		windows_coord.set((float) wx, float(Window::windowHeight - wy), (float) wz);
	}

	// check whether a line {lp1, lp2} intersects with a triangle, and if so, where it hits the triangle.
	bool CheckLineIntersectTri(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos)
	{
		Vector3D normal, intersect_pos, view_vector;
		Vector3D test;

		// Find plane normal
		normal = tp2 - tp1;
		normal.cross(tp3 - tp1);
		normal.normalize(); // not really needed

		// Find distance from lp1 and lp2 to the plane
		float dist1 = (lp1-tp1).dot(normal);
		float dist2 = (lp2-tp1).dot(normal);
		float angle, mix;

		mix = -dist1/(dist2-dist1);

		if (mix < 0.0f) return false;  // line doesn't cross the plane.
		if (mix > 1.0f) return false;  // line doesn't cross the plane.
		if (dist1 == dist2) return false;  // line and plane are parallel

		view_vector = lp2-lp1;
		view_vector.normalize();
		
		// get angle between the normal of the triangle and view vector, to be able to see
		// from what direction the view vector is hitting the triangle
		
		angle = acos(view_vector.dot(normal));

		if (angle >= PI/2)
		{
			return false;  // line intersects with the plane from the reverse/wrong direction
		}

		// Find point on the line that intersects with the plane
		intersect_pos = lp1 + (lp2-lp1) * mix;

		// Find whether the interesection point lies inside the triangle by testing it against all edges
		test = normal;
		test.cross(tp2-tp1);
		if (test.dot(intersect_pos-tp1) < 0.0f) return false;

		test = normal;
		test.cross(tp3-tp2);
		if (test.dot(intersect_pos-tp2) < 0.0f) return false;

		test = normal;
		test.cross(tp1-tp3);
		if (test.dot(intersect_pos-tp1) < 0.0f) return false;

		hit_pos = intersect_pos;
		
		return true;
	}

	void WindowCoordToVector(GLdouble clickx, GLdouble clicky, Vector3D &pos_vector_near, Vector3D &pos_vector_far)
	{
		// This function will find 2 points in world space that are on the line into the screen defined by screen-space( ie. window-space ) point (x,y)
		GLdouble mvmatrix[16];
		GLdouble projmatrix[16];
		GLint viewport[4];
		GLdouble dX, dY, dZ;
		GLdouble clicky_fixed;
		Vector3D v1, v2, v3;
			
		glGetIntegerv(GL_VIEWPORT, viewport);	
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
		clicky_fixed = double (Window::windowHeight - clicky); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top
			
		gluUnProject (clickx, clicky_fixed, 0.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
		v1.set((float) dX, (float) dY, (float) dZ);
		gluUnProject (clickx, clicky_fixed, 1.0, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
		v2.set((float) dX, (float) dY, (float) dZ);
		pos_vector_near = v1;
		pos_vector_far = v2;
	}

	// check whether a line {lp1, lp2} intersects with a plane, defined by the triangle {tp1, tp2, tp3}, and if so, where it hits the plane.
	bool CheckLineIntersect(Vector3D tp1, Vector3D tp2, Vector3D tp3, Vector3D lp1, Vector3D lp2, Vector3D &hit_pos)
	{
		Vector3D normal, intersect_pos, view_vector;

		// Find plane normal
		normal = tp2 - tp1;
		normal.cross(tp3 - tp1);
		normal.normalize(); // not really needed

		// Find distance from lp1 and lp2 to the plane defined by the triangle
		float dist1 = (lp1-tp1).dot(normal);
		float dist2 = (lp2-tp1).dot(normal);
		float angle, mix;

		mix = -dist1/(dist2-dist1);

		if (mix < 0.0f) return false;  // line doesn't cross the plane.
		if (mix > 1.0f) return false;  // line doesn't cross the plane.
		if (dist1 == dist2) return false;  // line and plane are parallel

		view_vector = lp2-lp1;
		view_vector.normalize();
		
		angle = acos(view_vector.dot(normal));

		if (angle >= PI/2)
		{
			return false;  // line intersects with the plane from the reverse/wrong direction
		}

		// Find point on the line that intersects with the plane
		intersect_pos = lp1 + (lp2-lp1) * mix;
		hit_pos = intersect_pos;
		return true;
	}

	int SwitchTo2DViewport(float w, float h)
	{
		glPushMatrix(); // push the modelview matrix

		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

		glEnable(GL_BLEND);

		glMatrixMode (GL_PROJECTION); 
			
		glPushMatrix(); // push the projection matrix

		glLoadIdentity(); 
		glOrtho(0, w, h, 0, -1, 1); 

		glMatrixMode( GL_MODELVIEW );

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		return SUCCESS;
	}

	int RevertViewport()
	{
		glMatrixMode (GL_PROJECTION); 
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);

//			Need blending for the water rendering, remember? :-)
//			glDisable(GL_ALPHA);
//			glDisable(GL_BLEND);
			
		glPopMatrix(); // restore the projection matrix
	
		glMatrixMode( GL_MODELVIEW ); // go back into modelview matrix mode

		glPopMatrix(); // restore the modelview matrix
		return SUCCESS;
	}
		
	float InterpolateCatmullRom(float v0, float v1, float v2, float v3, float x)
	{
		float x2 = x * x;
		float x3 = x2 * x;
		return 0.5f * ((2*v1) + (-v0 + v2) * x + (2*v0 - 5*v1 + 4*v2 - v3) * x2 + (-v0 + 3*v1 - 3*v2 + v3) * x3);
	}

	Vector3D InterpolateCatmullRom(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x)
	{
		float x2 = x * x;
		float x3 = x2 * x;
		return ((v1*2) + (-v0 + v2) * x + (v0*2 - v1*5 + v2*4 - v3) * x2 + (-v0 + v1*3 - v2*3 + v3) * x3) * 0.5;
	}

	float BoundCheck(float val, float limit1, float limit2)
	{
		float temp;
		if (limit1 > limit2)
		{
			temp = limit1;
			limit1 = limit2;
			limit2 = temp;
		}
		if (val > limit2)
			return limit2;
		if (val < limit1)
			return limit1;
		return val;
	}

	Vector3D BoundCheck(Vector3D val, Vector3D limit1, Vector3D limit2)
	{
		val.x = BoundCheck(val.x, limit1.x, limit2.x);
		val.y = BoundCheck(val.y, limit1.y, limit2.y);
		val.z = BoundCheck(val.z, limit1.z, limit2.z);
		return val;
	}
	
	float InterpolateCatmullRomBounded(float v0, float v1, float v2, float v3, float x)
	{
		float x2 = x * x;
		float x3 = x2 * x;
		float m1 = (v2 - v0) * 0.5f;
		float m2 = (v3 - v1) * 0.5f;
		m1 = BoundCheck(m1, 0.0, v2 - v1);
		m1 = BoundCheck(m1, 0.0, v0 - v1);
		m2 = BoundCheck(m2, 0.0, v1 - v2);
		m2 = BoundCheck(m2, 0.0, v3 - v1);
		return v1 * (2 * x3 - 3 * x2 + 1) + m1 * (x3 - 2 * x2 + x) + v2 * (-2 * x3 + 3 * x2) + m2 * (x3 - x2);
	}

	Vector3D InterpolateCatmullRomBounded(Vector3D v0, Vector3D v1, Vector3D v2, Vector3D v3, float x)
	{
		float x2 = x * x;
		float x3 = x2 * x;
		Vector3D m1 = (v2 - v0) * 0.5;
		Vector3D m2 = (v3 - v1) * 0.5;
		Vector3D zero_vector(0, 0, 0);
		m1 = BoundCheck(m1, zero_vector, v2 - v1);
		m1 = BoundCheck(m1, zero_vector, v0 - v1);
		m2 = BoundCheck(m2, zero_vector, v1 - v2);
		m2 = BoundCheck(m2, zero_vector, v3 - v1);
		return v1 * (2 * x3 - 3 * x2 + 1) + m1 * (x3 - 2 * x2 + x) + v2 * (-2 * x3 + 3 * x2) + m2 * (x3 - x2);
	}

	// Trims both start and end in a string ' ' and '\t' is removed.
	void StringTrim(std::string target, std::string& result)
	{	
		int begin = 0;
		int end = 0;
		int length = target.length();
		
		//Find begin
		for(int i = 0; i < length; i++)
		{
			char current = target.at(i);
				if(current != '\t' && current != ' ')
				{
					begin = i;
					break;
				}				
		}
		
		//Find end
		for(int i = length - 1; i != 0; i--)
		{
			char current = target.at(i);
			if(current != '\t' && current != ' ')
			{
				end = i;
				break;
			}				
		}
		
		result = target.substr(begin, end - begin + 1);
	}
	
	void ReadLineFromFile(ifstream& file, std::string& buffer)
	{
		buffer.clear();

		while (file.peek() != '\n' && file.peek() != '\r' && !file.eof())
			buffer.push_back((char) file.get());

		while ((file.peek() == '\n' || file.peek() == '\r') && !file.eof())
			file.get();
	}
	
	int power_of_two(int input)
	{
	    int value = 1;

	    while ( value < input ) 
	    {
		        value <<= 1;
	    }
	    return value;
	}
	
	GLuint CreateGLTexture(SDL_Surface *image)
	{
		if (Game::Rules::noGraphics)
			return 0;
		if(!image) 
		{
			console << Console::err << "Failed to create GL texture. Received a NULL pointer." << Console::nl;
			return 0;
		}
		else
		{
			console << "Texture: W: " << image->w << " H: " << image->h << Console::nl;
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// use bilinear scaling for scaling down and trilinear scaling for scaling up
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate mipmap using GLU
			if(image->format->BytesPerPixel == 3)
			{
				gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image->w, image->h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
			}
			else if(image->format->BytesPerPixel == 4)
			{
				gluBuild2DMipmaps(GL_TEXTURE_2D, 4, image->w, image->h, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
			}
			
			return texture;
		}
	}
	
	SDL_Surface *LoadImage(std::string path)
	{
		std::string filename = VFS::ResolveReadable(path);

		if (!filename.length())
		{
			cout << "Failed to load texture '" << path << "' -- Not found!" << endl;
			return NULL;
		}

		SDL_Surface *image = IMG_Load(filename.c_str());

		if(!image) 
		{
			cout << "Failed to load texture '" << path << "' SDL Reports: " << IMG_GetError() << endl;
			return NULL;
		}
		return image;
	}

	//This function handles malformed textures with a size that is not a power of 2
	GLuint LoadTexture(std::string path)
	{
		if (Game::Rules::noGraphics)
			return 0;
		
		std::string filename = VFS::ResolveReadable("/data/textures/" + path);

		if (!filename.length())
		{
			cout << "Failed to load texture '" << path << "' -- Not found!" << endl;
			return 0;
		}

		SDL_Surface *image = IMG_Load(filename.c_str());

		if(!image) 
		{
			cout << "Failed to load texture '" << filename << "' SDL Reports: " << IMG_GetError() << endl;
			return 0;
		}
		else
		{
			console << "Texture: " << filename << Console::nl;
			GLuint texture = CreateGLTexture(image);
			SDL_FreeSurface(image);
			
			return texture;
		}
	}

	float RandomDegree()
	{
		return ((float) rand() / RAND_MAX) * 360;
	}
	
	void PrintGLError()
	{
		bool is_first_error = true;
		std::cout << "OpenGL errors:" << std::endl;
		while (1)
		{
			int error = glGetError();
			switch(error)
			{
				case GL_NO_ERROR:
				{
					if (is_first_error)
					{
						std::cout << "No errors" << std::endl;
					}
					return;
				}
				case GL_INVALID_OPERATION:
				{
					std::cout << "Invalid Operation" << std::endl;
					break;
				}
				case GL_STACK_OVERFLOW:
				{
					std::cout << "Stack overflow" << std::endl;
					break;
				}
				case GL_STACK_UNDERFLOW:
				{
					std::cout << "Stack underflow" << std::endl;
					break;
				}
				case GL_INVALID_ENUM:
				{
					std::cout << "Invalid Enum" << std::endl;
					break;
				}
				case GL_INVALID_VALUE:
				{
					std::cout << "Invalid Value" << std::endl;
					break;
				}
				default:
				{	
					std::cout << "Unknown error..." <<  std::endl;
					break;
				}

			}
			is_first_error = false;
		}
	}

	#ifdef _MSC_VER
		#define VSNPRINTF _vsnprintf
	#else
		#define VSNPRINTF vsnprintf
	#endif

	// vsprintf for std::string
	std::string stdvsprintf(std::string format, va_list v)
	{
		char *buf = (char*) malloc(1); // malloc(0) is undefined behaviour, and also the implementation MAY write one \0.
		int size = 1;
		int length = VSNPRINTF(buf, 0, format.c_str(), v);
		if (length < 0)
		{
			// Microsoft implementation detected; returns a negative integer if output didn't fit.
			// Loop until it doesn't return a negative integer.
			while (length < 0)
			{
				size <<= 1;
				buf = (char*) realloc(buf, size+1); // Allow implementation to either write size or
				                                    // size+1 characters
				length = VSNPRINTF(buf, size, format.c_str(), v);
			}
		}
		else if (length <= 1)
		{
			// Old implementation detected; returns number of characters actually written.
			// If it wrote one \0 character, allow that too...
			// Loop until written characters is less than buffer size.
			// Note that if the format string is "" or if it evaluates to "",
			// this branch will be entered on the Windows platform, but it will do
			// no harm.
			do
			{
				size <<= 1;
				buf = (char*) realloc(buf, size+1);
				length = VSNPRINTF(buf, size, format.c_str(), v);
			}
			while (length >= size);
		}
		else
		{
			// Appears we have a conforming vsnprintf implementation; returns number of characters needed.
			size = length+1;
			buf = (char*) realloc(buf, size+1);
			VSNPRINTF(buf, size, format.c_str(), v);
		}
		buf[size] = '\0'; // Make sure it *really* ends with \0 (if it doesn't, we're in trouble)
		                  // At least Windows MAY not write an ending \0.

		std::string str = std::string(buf);
			
		free(buf);

		return str;
	}

	// sprintf for std::string
	std::string stdsprintf(std::string format, ...)
	{
		va_list v;
		va_start(v, format);
		std::string str = stdvsprintf(format, v);
		va_end(v);
		return str;
	}
}

