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
#include "ogrexmlmodel.h"

#include "unit.h"
#include "minixml.h"
#include "render-pre.h"
#include "paths.h"
#include "materialxml.h"
#include "utilities.h"
#include <cassert>
#include <sstream>
#include <map>

namespace Utilities
{
	static gc_ptr<OgreMesh> mesh = NULL;
	static gc_ptr<OgreSubMesh> submesh = NULL;
	static gc_ptr<OgreVertexBuffer> vb = NULL;
	static gc_ptr<OgreVertexAnimation> vertexAnimation = NULL;
	static gc_ptr<OgrePose> pose = NULL;
	static gc_ptr<OgreAnimationTrack> animationTrack = NULL;
	static std::vector<gc_ptr<OgreVertexBuffer> > vbs;
	static unsigned numVertices;
	static unsigned face_index, vertex_index, texcoord_index;

	static Utilities::FEString x_str = "x", y_str = "y", z_str = "z";

	void ParsePosition(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute(x_str, "0")),
		                  yss(elem->GetAttribute(y_str, "0")),
		                  zss(elem->GetAttribute(z_str, "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->positions->data.floats[vertex_index*3] = x;
		vb->positions->data.floats[vertex_index*3+1] = z;
		vb->positions->data.floats[vertex_index*3+2] = -y;
	}

	void ParseNormal(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute(x_str, "0")),
		                  yss(elem->GetAttribute(y_str, "0")),
		                  zss(elem->GetAttribute(z_str, "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->normals->data.floats[vertex_index*3] = x;
		vb->normals->data.floats[vertex_index*3+1] = z;
		vb->normals->data.floats[vertex_index*3+2] = -y;
	}

	static Utilities::FEString w_str = "w";

	void ParseTangent(Utilities::XMLElement *elem)
	{
		if (vb->tangentDims == 3)
		{
			std::stringstream xss(elem->GetAttribute(x_str, "0")),
			                  yss(elem->GetAttribute(y_str, "0")),
			                  zss(elem->GetAttribute(z_str, "0"));
			GLfloat x, y, z;
			xss >> x; yss >> y; zss >> z;
			vb->tangents->data.floats[vertex_index*3] = x;
			vb->tangents->data.floats[vertex_index*3+1] = z;
			vb->tangents->data.floats[vertex_index*3+2] = -y;
		}
		else if (vb->tangentDims == 4)
		{
			std::stringstream xss(elem->GetAttribute(x_str, "0")),
			                  yss(elem->GetAttribute(y_str, "0")),
			                  zss(elem->GetAttribute(z_str, "0")),
			                  wss(elem->GetAttribute(w_str, "0"));
			GLfloat x, y, z, w;
			xss >> x; yss >> y; zss >> z; wss >> w;
			vb->tangents->data.floats[vertex_index*4] = x;
			vb->tangents->data.floats[vertex_index*4+1] = z;
			vb->tangents->data.floats[vertex_index*4+2] = -y;
			vb->tangents->data.floats[vertex_index*4+3] = w;
		}
	}

	void ParseBinormal(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute(x_str, "0")),
		                  yss(elem->GetAttribute(y_str, "0")),
		                  zss(elem->GetAttribute(z_str, "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->binormals->data.floats[vertex_index*3] = x;
		vb->binormals->data.floats[vertex_index*3+1] = z;
		vb->binormals->data.floats[vertex_index*3+2] = -y;
	}

	static Utilities::FEString u_str = "u", v_str = "v";

	void ParseTexcoord(Utilities::XMLElement *elem)
	{
		std::stringstream uss, vss, wss;
		GLfloat u, v, w;
		if (texcoord_index >= vb->texCoords.size())
		{
			return;
		}
		switch (vb->texCoordDims[texcoord_index])
		{
			case 1:
				uss << elem->GetAttribute(u_str, "0");
				uss >> u;
				vb->texCoords[texcoord_index]->data.floats[vertex_index] = u;
				break;
			case 2:
				uss << elem->GetAttribute(u_str, "0"); vss << elem->GetAttribute(v_str, "0");
				uss >> u; vss >> v;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*2] = u;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*2+1] = v;
				break;
			case 3:
				uss << elem->GetAttribute(u_str, "0"); vss << elem->GetAttribute(v_str, "0"); wss << elem->GetAttribute(w_str, "0");
				uss >> u; vss >> v; wss >> w;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*3] = u;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*3+1] = v;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*3+2] = w;
				break;
			default:
				break;
		}
		texcoord_index++;
	}

	static Utilities::FEString position_str = "position", normal_str = "normal", tangent_str = "tangent", binormal_str = "binormal", texcoord_str = "texcoord";

	void ParseVertex(Utilities::XMLElement *elem)
	{
		if (vertex_index >= vb->numVertices)
		{
			return;
		}
		elem->Iterate(position_str, ParsePosition);
		if (vb->normals)
			elem->Iterate(normal_str, ParseNormal);
		if (vb->tangents)
			elem->Iterate(tangent_str, ParseTangent);
		if (vb->binormals)
			elem->Iterate(binormal_str, ParseBinormal);
		texcoord_index = 0;
		elem->Iterate(texcoord_str, ParseTexcoord);
		vertex_index++;
	}

	void ParseVertexBuffer(Utilities::XMLElement *elem)
	{
		vb = new OgreVertexBuffer;
		vb->numVertices = numVertices;
		if (elem->GetAttribute("positions") == "true")
		{
			vb->positions = new Scene::Render::VBO;
			vb->positions->data.floats = new GLfloat[numVertices * 3];
			vb->positions->numVals = numVertices * 3;
			vb->positions->size = numVertices * 3 * sizeof(GLfloat);
		}
		if (elem->GetAttribute("normals") == "true")
		{
			vb->normals = new Scene::Render::VBO;
			vb->normals->data.floats = new GLfloat[numVertices * 3];
			vb->normals->numVals = numVertices * 3;
			vb->normals->size = numVertices * 3 * sizeof(GLfloat);
		}
		if (elem->GetAttribute("binormals") == "true")
		{
			vb->binormals = new Scene::Render::VBO;
			vb->binormals->data.floats = new GLfloat[numVertices * 3];
			vb->binormals->numVals = numVertices * 3;
			vb->binormals->size = numVertices * 3 * sizeof(GLfloat);
		}
		if (elem->GetAttribute("tangents") == "true")
		{
			std::stringstream dims_ss(elem->GetAttribute("tangent_dimensions", "3"));
			unsigned dims;
			dims_ss >> dims;

			vb->tangentDims = dims;
			vb->tangents = new Scene::Render::VBO;
			vb->tangents->data.floats = new GLfloat[numVertices * dims];
			vb->tangents->numVals = numVertices * dims;
			vb->tangents->size = numVertices * dims * sizeof(GLfloat);
		}

		std::stringstream tc_ss(elem->GetAttribute("texture_coords", "0"));
		unsigned numTexCoords;
		tc_ss >> numTexCoords;

		for (unsigned i = 0; i < numTexCoords; i++)
		{
			std::stringstream dims_ss(elem->GetAttribute("texture_coord_dimensions_" + i, "2"));
			unsigned dims;
			dims_ss >> dims;
			vb->texCoordDims.push_back(dims);
		}
		vertex_index = 0;
		elem->Iterate("vertex", ParseVertex);
		assert(vertex_index == numVertices);
		vbs.push_back(vb);
	}

	void ParseGeometry(Utilities::XMLElement *elem)
	{
		std::stringstream nv_ss(elem->GetAttribute("vertexcount", "0"));
		nv_ss >> numVertices;
		elem->Iterate("vertexbuffer", ParseVertexBuffer);
	}

	static Utilities::FEString v1_str = "v1", v2_str = "v2", v3_str = "v3";

	void ParseFace(Utilities::XMLElement *elem)
	{
		std::stringstream v1_ss(elem->GetAttribute(v1_str)),
		                  v2_ss(elem->GetAttribute(v2_str)),
		                  v3_ss(elem->GetAttribute(v3_str));
		v1_ss >> submesh->faces->data.uints[face_index++];
		if (elem->HasAttribute(v2_str))
			v2_ss >> submesh->faces->data.uints[face_index++];
		if (elem->HasAttribute(v3_str))
			v3_ss >> submesh->faces->data.uints[face_index++];
	}

	void ParseFaces(Utilities::XMLElement *elem)
	{
		submesh->faces = new Scene::Render::VBO;
		std::stringstream nv_ss(elem->GetAttribute("count"));
		nv_ss >> submesh->faces->numVals;
		submesh->faces->data.uints = new GLuint[submesh->faces->numVals*3];
		face_index = 0;
		elem->Iterate("face", ParseFace);
		submesh->numElems = face_index;
		submesh->faces->size = face_index * sizeof(GLuint);
		submesh->faces->isElemBuffer = true;
	}

	void ParseSubmesh(Utilities::XMLElement *elem)
	{
		submesh = new OgreSubMesh;
		std::string primitiveType = elem->GetAttribute("operationtype", "triangle_list");
		if (primitiveType == "triangle_strip")
		{
			submesh->primitiveType = OgreSubMesh::PRIMITIVETYPE_TRIANGLESTRIP;
		}
		else if (primitiveType == "triangle_fan")
		{
			submesh->primitiveType = OgreSubMesh::PRIMITIVETYPE_TRIANGLEFAN;
		}
		else
		{
			submesh->primitiveType = OgreSubMesh::PRIMITIVETYPE_TRIANGLELIST;
		}
		elem->Iterate("faces", ParseFaces);
		if (elem->GetAttribute("usesharedvertices", "true") == "true")
		{
			submesh->vbs = mesh->shared;
		}
		else
		{
			vbs.clear();
			elem->Iterate("geometry", ParseGeometry);
			submesh->vbs = vbs;
		}
		mesh->submeshes.push_back(submesh);
	}

	void ParseSubmeshes(Utilities::XMLElement *elem)
	{
		elem->Iterate("submesh", ParseSubmesh);
	}

	void ParseSharedGeometry(Utilities::XMLElement *elem)
	{
		std::stringstream nv_ss(elem->GetAttribute("vertexcount"));
		nv_ss >> numVertices;
		vbs.clear();
		elem->Iterate("vertexbuffer", ParseVertexBuffer);
		mesh->shared = vbs;
	}

	void ParsePoseOffset(Utilities::XMLElement *elem)
	{
		OgrePoseOffset poseOffset;
		poseOffset.index = elem->GetAttributeT<int>("index", 0);
		poseOffset.offset.x = elem->GetAttributeT<float>("x", 0.0f);
		poseOffset.offset.y = elem->GetAttributeT<float>("y", 0.0f);
		poseOffset.offset.z = elem->GetAttributeT<float>("z", 0.0f);
		pose->offsets.push_back(poseOffset);
	}

	void ParsePose(Utilities::XMLElement *elem)
	{
		pose = new OgrePose;
		pose->name = elem->GetAttribute("name");
		pose->targetsShared = elem->GetAttribute("target") == "mesh";
		pose->targetIndex = elem->GetAttributeT<int>("index", 0);
	}

	void ParsePoses(Utilities::XMLElement *elem)
	{
		elem->Iterate("pose", ParsePose);
	}

	void ParseMorphKeyframe(Utilities::XMLElement *elem)
	{
		OgreMorphAnimFrame animFrame;

		// TODO: Load positions

		animFrame.time = elem->GetAttributeT<float>("time", 0.0f);
	}

	void ParseMorphKeyframes(Utilities::XMLElement *elem)
	{
		elem->Iterate("keyframe", ParseMorphKeyframe);
	}

	void ParsePoseKeyframe(Utilities::XMLElement *elem)
	{
		OgrePoseAnimFrame animFrame;

		// TODO: Load poserefs

		animFrame.time = elem->GetAttributeT<float>("time", 0.0f);
	}

	void ParsePoseKeyframes(Utilities::XMLElement *elem)
	{
		elem->Iterate("keyframe", ParsePoseKeyframe);
	}

	void ParseTrack(Utilities::XMLElement *elem)
	{
		animationTrack = NULL;
		if (elem->GetAttribute("type") == "morph")
		{
			gc_ptr<OgreMorphAnimation> ogreMorphAnimation = new OgreMorphAnimation;
			animationTrack = ogreMorphAnimation;
		}
		else if (elem->GetAttribute("type") == "pose")
		{
			gc_ptr<OgrePoseAnimation> ogrePoseAnimation = new OgrePoseAnimation;
			animationTrack = ogrePoseAnimation;
		}
		if (animationTrack)
		{
			animationTrack->targetsShared = elem->GetAttribute("target") == "mesh";
			animationTrack->targetIndex = elem->GetAttributeT<int>("index", 0);
		}
	}

	void ParseTracks(Utilities::XMLElement *elem)
	{
		elem->Iterate("track", ParseTrack);
	}

	void ParseAnimation(Utilities::XMLElement *elem)
	{
		vertexAnimation = new OgreVertexAnimation;
		vertexAnimation->name = elem->GetAttribute("name");
		vertexAnimation->length = elem->GetAttributeT<float>("length", 0.0f);
		elem->Iterate("tracks", ParseTracks);
		mesh->vertexAnimations[vertexAnimation->name] = vertexAnimation;
	}

	void ParseAnimations(Utilities::XMLElement *elem)
	{
		elem->Iterate("animation", ParseAnimation);
	}

	void ParseMesh(Utilities::XMLElement *elem)
	{
		mesh = new OgreMesh;
		elem->Iterate("submeshes", ParseSubmeshes);
		elem->Iterate("sharedgeometry", ParseSharedGeometry);
		elem->Iterate("poses", ParsePoses);
		elem->Iterate("animations", ParseAnimations);
	}

	void ParseModTranslate(Utilities::XMLElement *elem)
	{
		float x = elem->GetAttributeT<float>("x", 0.0);
		float y = elem->GetAttributeT<float>("y", 0.0);
		float z = elem->GetAttributeT<float>("z", 0.0);
		mesh->transforms.push_back(new Scene::Render::MeshTranslation(x, y, z));
	}

	void ParseModRotate(Utilities::XMLElement *elem)
	{
		float radians = elem->GetAttributeT<float>("degrees", 0.0) * (PI / 180);
		float x = elem->GetAttributeT<float>("x", 0.0);
		float y = elem->GetAttributeT<float>("y", 0.0);
		float z = elem->GetAttributeT<float>("z", 0.0);
		mesh->transforms.push_back(new Scene::Render::MeshRotation(radians, x, y, z));
	}

	void ParseModScale(Utilities::XMLElement *elem)
	{
		float amount = elem->GetAttributeT<float>("amount", 0.0);
		float x = elem->GetAttributeT<float>("x", 0.0);
		float y = elem->GetAttributeT<float>("y", 0.0);
		float z = elem->GetAttributeT<float>("z", 0.0);
		if (elem->HasAttribute("amount"))
			mesh->transforms.push_back(new Scene::Render::MeshScaling(amount, amount, amount));
		else
			mesh->transforms.push_back(new Scene::Render::MeshScaling(x, y, z));
	}

	void ParseModTransforms(Utilities::XMLElement *elem)
	{
		TagFuncMap tfmap;
		tfmap["translate"] = ParseModTranslate;
		tfmap["rotate"] = ParseModRotate;
		tfmap["scale"] = ParseModScale;
		elem->Iterate(tfmap, NULL);
	}

	void ParseModifiers(Utilities::XMLElement *elem)
	{
		elem->Iterate("transforms", ParseModTransforms);
	}

	static std::map<std::string, gc_ptr<OgreMesh> > filenameToMesh;

	gc_ptr<OgreMesh> LoadSpecificOgreXMLModel(std::string name)
	{
		Utilities::XMLReader xmlReader;
		
		std::string filename = Utilities::GetDataFile(name + ".mesh.xml");

		mesh = filenameToMesh[filename];

		if (!mesh)
		{
			if (filename.length() && xmlReader.Read(filename))
			{
				xmlReader.root->Iterate("mesh", ParseMesh);

				std::string modFilename = Utilities::GetDataFile(name + ".mod.xml");

				Utilities::XMLReader modXmlReader;

				if (modFilename.length() && modXmlReader.Read(modFilename))
				{
					modXmlReader.root->Iterate("modifiers", ParseModifiers);
				}
			}
			else
			{
				std::cout << "Warning: Mesh " << name << " not found!" << std::endl;
			}
		}

		filenameToMesh[filename] = mesh;

		xmlReader.Deallocate();

		return mesh;

	}

	bool OgreSubMesh::CheckRayIntersect(const Vector3D& near_plane, const Vector3D& far_plane, float& distance)
	{
		const gc_ptr<OgreVertexBuffer>& vb = vbs[0];
		Utilities::Vector3D tp1, tp2, tp3, hit_pos;
		int index_v;

		for (unsigned i = 0; i < faces->numVals*3; )
		{
			index_v = faces->data.uints[i++] * 3;
			tp1.set(vb->positions->data.floats[index_v], vb->positions->data.floats[index_v+1], vb->positions->data.floats[index_v+2]);
			index_v = faces->data.uints[i++] * 3;
			tp2.set(vb->positions->data.floats[index_v], vb->positions->data.floats[index_v+1], vb->positions->data.floats[index_v+2]);
			index_v = faces->data.uints[i++] * 3;
			tp3.set(vb->positions->data.floats[index_v], vb->positions->data.floats[index_v+1], vb->positions->data.floats[index_v+2]);
			if (CheckLineIntersectTri(tp1, tp3, tp2, near_plane, far_plane, hit_pos))
			{
				distance = near_plane.distance(hit_pos);
				return true;
			}
		}
		return false;
	}

	bool OgreMesh::CheckRayIntersect(const Vector3D& near_plane, const Vector3D& far_plane, float& distance)
	{
		for (std::vector<gc_ptr<OgreSubMesh> >::iterator it = submeshes.begin(); it != submeshes.end(); it++)
		{
			if ((*it)->CheckRayIntersect(near_plane, far_plane, distance))
				return true;
		}
		return false;
	}

}
