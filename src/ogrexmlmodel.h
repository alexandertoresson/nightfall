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
		ref_ptr<Scene::Render::VBO> positions;
		ref_ptr<Scene::Render::VBO> normals;
		ref_ptr<Scene::Render::VBO> tangents;
		ref_ptr<Scene::Render::VBO> binormals;
		ref_ptr<Scene::Render::VBO> colorDiffuse;
		ref_ptr<Scene::Render::VBO> colorSpecular;
		unsigned tangentDims;
		std::vector<ref_ptr<Scene::Render::VBO> > texCoords;
		std::vector<unsigned> texCoordDims;
		
		OgreVertexBuffer() : positions(NULL), normals(NULL), tangents(NULL), binormals(NULL), colorDiffuse(NULL), colorSpecular(NULL), tangentDims(0)
		{
			
		}
	};

	struct OgreSubMesh
	{
		ref_ptr<Scene::Render::VBO> faces;

		enum 
		{
			PRIMITIVETYPE_TRIANGLELIST,
			PRIMITIVETYPE_TRIANGLESTRIP,
			PRIMITIVETYPE_TRIANGLEFAN
		} primitiveType;

		std::vector<ref_ptr<OgreVertexBuffer> > vbs;
		unsigned numElems;
		ref_ptr<Material> material;
		
		bool CheckRayIntersect(const Vector3D& near, const Vector3D& far, float& distance);

	};

	struct OgreMesh
	{
		std::vector<ref_ptr<OgreSubMesh> > submeshes;
		std::vector<ref_ptr<OgreVertexBuffer> > shared;
		std::vector<ref_ptr<Scene::Render::MeshTransformation> > transforms;
		bool CheckRayIntersect(const Vector3D& near_plane, const Vector3D& far_plane, float& distance);
	};

	ref_ptr<OgreMesh> LoadSpecificOgreXMLModel(std::string filename);
}

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h-end"
#endif

#endif
