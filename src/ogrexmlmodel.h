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
		gc_ptr<Scene::Render::VBO> positions;
		gc_ptr<Scene::Render::VBO> normals;
		gc_ptr<Scene::Render::VBO> tangents;
		gc_ptr<Scene::Render::VBO> binormals;
		gc_ptr<Scene::Render::VBO> colorDiffuse;
		gc_ptr<Scene::Render::VBO> colorSpecular;
		unsigned tangentDims;
		std::vector<gc_ptr<Scene::Render::VBO> > texCoords;
		std::vector<unsigned> texCoordDims;
		
		OgreVertexBuffer() : positions(NULL), normals(NULL), tangents(NULL), binormals(NULL), colorDiffuse(NULL), colorSpecular(NULL), tangentDims(0)
		{
			
		}

		void shade()
		{
			positions.shade();
			normals.shade();
			tangents.shade();
			binormals.shade();
			colorDiffuse.shade();
			colorSpecular.shade();
			gc_shade_container(texCoords);
		}
	};

	struct OgreSubMesh
	{
		gc_ptr<Scene::Render::VBO> faces;

		enum 
		{
			PRIMITIVETYPE_TRIANGLELIST,
			PRIMITIVETYPE_TRIANGLESTRIP,
			PRIMITIVETYPE_TRIANGLEFAN
		} primitiveType;

		std::vector<gc_ptr<OgreVertexBuffer> > vbs;
		unsigned numElems;
		gc_ptr<Material> material;
		
		bool CheckRayIntersect(const Vector3D& near, const Vector3D& far, float& distance);

		void shade()
		{
			faces.shade();
			gc_shade_container(vbs);
			material.shade();
		}

	};

	struct OgreMesh
	{
		std::vector<gc_ptr<OgreSubMesh> > submeshes;
		std::vector<gc_ptr<OgreVertexBuffer> > shared;
		std::vector<gc_ptr<Scene::Render::MeshTransformation> > transforms;
		bool CheckRayIntersect(const Vector3D& near_plane, const Vector3D& far_plane, float& distance);

		void shade()
		{
			gc_shade_container(submeshes);
			gc_shade_container(shared);
			gc_shade_container(transforms);
		}
	};

	gc_ptr<OgreMesh> LoadSpecificOgreXMLModel(std::string filename);
}

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h-end"
#endif

#endif
