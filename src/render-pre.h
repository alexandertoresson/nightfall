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
#ifndef __RENDER_H_PRE__
#define __RENDER_H_PRE__

#ifdef DEBUG_DEP
#warning "render.h-pre"
#endif

#include "sdlheader.h"

namespace Scene
{
	namespace Render
	{
		class GLStateNode;
		class GeometryNode;
		class OgreMeshNode;

		struct VBO
		{
			union
			{
				GLfloat* floats;
				GLuint* uints;
				GLushort* ushorts;
				void* pnt;
			} data;
			unsigned numVals;
			unsigned size;
			unsigned valsPerItem;
			GLuint buffer;
			bool isElemBuffer;
			GLenum mode;
			bool changed;

			VBO(bool isElemBuffer = false, GLenum mode = GL_STATIC_DRAW_ARB) : numVals(0), size(0), valsPerItem(0), buffer(0), isElemBuffer(isElemBuffer), mode(mode), changed(true)
			{
				data.pnt = NULL;
			}

			~VBO()
			{
				if (data.floats)
				{
					delete[] data.floats;
				}
			}

			void Lock();
			void Unlock();

			void SetChanged()
			{
				changed = true;
			}

			void shade() const
			{
				
			}

		};

		class MeshTransformation;
		class MeshTranslation;
		class MeshRotation;
		class MeshScaling;
	}
}

#endif
