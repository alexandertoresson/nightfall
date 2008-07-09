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

			VBO() : numVals(0), size(0), valsPerItem(0), buffer(0), isElemBuffer(false)
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

			void shade()
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
