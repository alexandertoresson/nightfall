#ifndef __OGREXMLMODEL_H__
#define __OGREXMLMODEL_H__

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h"
#endif

#include "unit-pre.h"
#include "ogrexmlmodel-pre.h"
#include "render.h"

namespace Utilities
{
	struct OgreVertexBuffer
	{
		unsigned numVertices;
		Scene::Render::VBO* positions;
		Scene::Render::VBO* normals;
		Scene::Render::VBO* tangents;
		Scene::Render::VBO* binormals;
		Scene::Render::VBO* colorDiffuse;
		Scene::Render::VBO* colorSpecular;
		unsigned tangentDims;
		std::vector<Scene::Render::VBO*> texCoords;
		std::vector<unsigned> texCoordDims;
		
		OgreVertexBuffer() : positions(NULL), normals(NULL), tangents(NULL), binormals(NULL), colorDiffuse(NULL), colorSpecular(NULL), tangentDims(0)
		{
			
		}
	};

	struct OgreSubMesh
	{
		Scene::Render::VBO faces;

		enum 
		{
			PRIMITIVETYPE_TRIANGLELIST,
			PRIMITIVETYPE_TRIANGLESTRIP,
			PRIMITIVETYPE_TRIANGLEFAN
		} primitiveType;

		std::vector<OgreVertexBuffer*> vbs;
	};

	OgreMesh* LoadOgreXMLModel(std::string filename);
}

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h-end"
#endif

#endif
