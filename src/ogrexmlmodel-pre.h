#ifndef __OGREXMLMODEL_H_PRE__
#define __OGREXMLMODEL_H_PRE__

#ifdef DEBUG_DEP
#warning "model.h-pre"
#endif

#include "sdlheader.h"
#include <vector>

namespace Utilities
{
	struct OgreSubMesh;

	struct OgreVertexBuffer;

	struct OgreMesh
	{
		std::vector<OgreSubMesh*> submeshes;
		std::vector<OgreVertexBuffer*> shared;
	};
}

#endif
