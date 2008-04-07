#ifndef __UNITRENDER_H_PRE__
#define __UNITRENDER_H_PRE__

#ifdef DEBUG_DEP
#warning "unitrender.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		struct VBOArrays
		{
			GLuint normalArray;
			GLuint vertexArray;
			GLuint texCoordArray;
			VBOArrays()
			{
				normalArray = 0;
				vertexArray = 0;
				texCoordArray = 0;
			}
		};

		void SetParticleCoordSpace(float x, float y, float z, float scale = 1.0f);
		
		void RenderUnits();
		void RenderHealthBars();
		
		void InitRenderUnits();

		class UnitTransfNode;
	}
}

#endif
