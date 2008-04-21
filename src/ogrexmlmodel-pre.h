#ifndef __OGREXMLMODEL_H_PRE__
#define __OGREXMLMODEL_H_PRE__

#ifdef DEBUG_DEP
#warning "model.h-pre"
#endif

#include "sdlheader.h"
#include <vector>

namespace Utilities
{
	struct VBO
	{
		union
		{
			GLfloat* floats;
			GLuint* uints;
			GLushort* ushorts;
		} data;
		unsigned numVals;
		unsigned size;
		GLuint buffer;
		bool isElemBuffer;

		VBO() : numVals(0), size(0), buffer(0), isElemBuffer(false)
		{
			data.floats = NULL;
		}

	};

	struct OgreVertexBuffer
	{
		unsigned numVertices;
		VBO* positions;
		VBO* normals;
		VBO* tangents;
		VBO* binormals;
		VBO* colorDiffuse;
		VBO* colorSpecular;
		unsigned tangentDims;
		std::vector<VBO*> texCoords;
		std::vector<unsigned> texCoordDims;
		
		OgreVertexBuffer() : positions(NULL), normals(NULL), tangents(NULL), binormals(NULL), colorDiffuse(NULL), colorSpecular(NULL), tangentDims(0)
		{
			
		}
	};

	struct OgreSubMesh
	{
		VBO faces;

		enum 
		{
			PRIMITIVETYPE_TRIANGLELIST,
			PRIMITIVETYPE_TRIANGLESTRIP,
			PRIMITIVETYPE_TRIANGLEFAN
		} primitiveType;

		std::vector<OgreVertexBuffer*> vbs;
	};

	struct OgreMesh
	{
		std::vector<OgreSubMesh*> submeshes;
		std::vector<OgreVertexBuffer*> shared;
	};
}

#endif
