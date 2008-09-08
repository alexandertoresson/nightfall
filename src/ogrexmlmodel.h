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
	struct OgreBone
	{
		std::string name;
		gc_ptr<OgreBone> parent;

		bool doScale;

		Utilities::Vector3D position;
		float angle;
		Utilities::Vector3D axis;
		Utilities::Vector3D scale;
		
		void shade()
		{
			parent.shade();
		}
	};

	struct OgreBoneAnimKeyFrame
	{
		float time;
		
		bool doTranslate;
		bool doRotate;
		bool doScale;

		Utilities::Vector3D translate;
		float angle;
		Utilities::Vector3D axis;
		Utilities::Vector3D scale;
	};

	struct OgreBoneAnimTrack
	{
		gc_ptr<OgreBone> bone;
		std::vector<OgreBoneAnimKeyFrame> keyframes;
		
		void shade() const
		{
			bone.shade();
		}
	};

	struct OgreBoneAnimation
	{
		std::string name;
		float length;

		std::vector<OgreBoneAnimTrack> tracks;
		
		void shade() const
		{
			gc_shade_container(tracks);
		}
	};

	struct OgreVertexBoneAssignment
	{
		gc_ptr<OgreBone> bone;
		float weight;

		void shade() const
		{
			bone.shade();
		}
	};

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

	struct OgreAnimationTrack
	{
		virtual void shade() = 0;
	};

	struct OgreVertexAnimation
	{
		std::string name;
		float length;
		std::vector<OgreAnimationTrack> tracks;
	};

	struct OgreMorphAnimFrame
	{
		Scene::Render::VBO positions;
		float time;

		void shade() const
		{
			positions.shade();
		}
	};

	struct OgreMorphAnimation : public OgreAnimationTrack
	{
		std::vector<OgreMorphAnimFrame> frames;
		
		void shade() const
		{
			gc_shade_container(frames);
		}
	};

	struct OgrePoseOffset
	{
		int index;
		Utilities::Vector3D offset;
	};

	struct OgrePose
	{
		std::string name;
		std::vector<OgrePoseOffset> offsets;
	};

	struct OgrePoseAnimRef
	{
		gc_ptr<OgrePose> pose;
		float influence;

		void shade() const
		{
			pose.shade();
		}
	};

	struct OgrePoseAnimFrame
	{
		std::vector<OgrePoseAnimRef> poseRefs;
		float time;
		
		void shade() const
		{
			gc_shade_container(poseRefs);
		}
	};

	struct OgrePoseAnimation : public OgreAnimationTrack
	{
		std::vector<OgrePoseAnimFrame> frames;
		
		void shade() const
		{
			gc_shade_container(frames);
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

		std::map<std::string, gc_ptr<OgreVertexAnimation> > vertexAnimations;
		std::vector<OgreVertexBoneAssignment> vertexBoneAssignments;
		
		bool CheckRayIntersect(const Vector3D& near, const Vector3D& far, float& distance);

		void shade()
		{
			faces.shade();
			gc_shade_container(vbs);
			material.shade();
			gc_shade_map(vertexAnimations);
			gc_shade_container(vertexBoneAssignments);
		}

	};

	struct OgreMesh
	{
		std::vector<gc_ptr<OgreSubMesh> > submeshes;
		std::vector<gc_ptr<OgreVertexBuffer> > shared;
		std::vector<gc_ptr<Scene::Render::MeshTransformation> > transforms;

		std::vector<gc_ptr<OgrePose> > poses;
		std::vector<gc_ptr<OgreBone> > bones;
		std::map<std::string, OgreBoneAnimation> boneAnimations;

		bool CheckRayIntersect(const Vector3D& near_plane, const Vector3D& far_plane, float& distance);

		void shade()
		{
			gc_shade_container(submeshes);
			gc_shade_container(shared);
			gc_shade_container(transforms);
			gc_shade_container(poses);
			gc_shade_map(boneAnimations);
		}
	};

	gc_ptr<OgreMesh> LoadSpecificOgreXMLModel(std::string filename);
}

#ifdef DEBUG_DEP
#warning "ogrexmlmodel.h-end"
#endif

#endif
