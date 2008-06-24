#include "ogrexmlmodel.h"

#include "unit.h"
#include "minixml.h"
#include "render-pre.h"
#include <sstream>

namespace Utilities
{
	OgreMesh* mesh = NULL;
	OgreSubMesh* submesh = NULL;
	OgreVertexBuffer* vb = NULL;
	std::vector<OgreVertexBuffer*> vbs;
	Scene::Render::VBO* texCoordVBO = NULL;
	unsigned numVertices;
	unsigned face_index, vertex_index, texcoord_index;
	
	void ParsePosition(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute("x", "0")),
		                  yss(elem->GetAttribute("y", "0")),
		                  zss(elem->GetAttribute("z", "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->positions->data.floats[vertex_index*3] = x;
		vb->positions->data.floats[vertex_index*3+1] = y;
		vb->positions->data.floats[vertex_index*3+2] = z;
	}

	void ParseNormal(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute("x", "0")),
		                  yss(elem->GetAttribute("y", "0")),
		                  zss(elem->GetAttribute("z", "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->normals->data.floats[vertex_index*3] = x;
		vb->normals->data.floats[vertex_index*3+1] = y;
		vb->normals->data.floats[vertex_index*3+2] = z;
	}

	void ParseTangent(Utilities::XMLElement *elem)
	{
		if (vb->tangentDims == 3)
		{
			std::stringstream xss(elem->GetAttribute("x", "0")),
			                  yss(elem->GetAttribute("y", "0")),
			                  zss(elem->GetAttribute("z", "0"));
			GLfloat x, y, z;
			xss >> x; yss >> y; zss >> z;
			vb->tangents->data.floats[vertex_index*3] = x;
			vb->tangents->data.floats[vertex_index*3+1] = y;
			vb->tangents->data.floats[vertex_index*3+2] = z;
		}
		else if (vb->tangentDims == 4)
		{
			std::stringstream xss(elem->GetAttribute("x", "0")),
			                  yss(elem->GetAttribute("y", "0")),
			                  zss(elem->GetAttribute("z", "0")),
			                  wss(elem->GetAttribute("w", "0"));
			GLfloat x, y, z, w;
			xss >> x; yss >> y; zss >> z; wss >> w;
			vb->tangents->data.floats[vertex_index*4] = x;
			vb->tangents->data.floats[vertex_index*4+1] = y;
			vb->tangents->data.floats[vertex_index*4+2] = z;
			vb->tangents->data.floats[vertex_index*4+3] = w;
		}
	}

	void ParseBinormal(Utilities::XMLElement *elem)
	{
		std::stringstream xss(elem->GetAttribute("x", "0")),
		                  yss(elem->GetAttribute("y", "0")),
		                  zss(elem->GetAttribute("z", "0"));
		GLfloat x, y, z;
		xss >> x; yss >> y; zss >> z;
		vb->binormals->data.floats[vertex_index*3] = x;
		vb->binormals->data.floats[vertex_index*3+1] = y;
		vb->binormals->data.floats[vertex_index*3+2] = z;
	}

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
				uss << elem->GetAttribute("u", "0");
				uss >> u;
				vb->texCoords[texcoord_index]->data.floats[vertex_index] = u;
				break;
			case 2:
				uss << elem->GetAttribute("u", "0"); vss << elem->GetAttribute("v", "0");
				uss >> u; vss >> v;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*2] = u;
				vb->texCoords[texcoord_index]->data.floats[vertex_index*2+1] = v;
				break;
			case 3:
				uss << elem->GetAttribute("u", "0"); vss << elem->GetAttribute("v", "0"); wss << elem->GetAttribute("w", "0");
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

	void ParseVertex(Utilities::XMLElement *elem)
	{
		if (vertex_index >= vb->numVertices)
		{
			return;
		}
		elem->Iterate("position", ParsePosition);
		if (vb->normals)
			elem->Iterate("normal", ParseNormal);
		if (vb->tangents)
			elem->Iterate("tangent", ParseTangent);
		if (vb->binormals)
			elem->Iterate("binormal", ParseBinormal);
		texcoord_index = 0;
		elem->Iterate("texcoord", ParseTexcoord);
		vertex_index++;
	}

	void ParseVertexBuffer(Utilities::XMLElement *elem)
	{
		vb = new OgreVertexBuffer;
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
		vbs.push_back(vb);
	}

	void ParseGeometry(Utilities::XMLElement *elem)
	{
		std::stringstream nv_ss(elem->GetAttribute("vertexcount", "0"));
		nv_ss >> numVertices;
		elem->Iterate("vertexbuffer", ParseVertexBuffer);
	}

	void ParseFace(Utilities::XMLElement *elem)
	{
		std::stringstream v1_ss(elem->GetAttribute("v1")),
		                  v2_ss(elem->GetAttribute("v2")),
		                  v3_ss(elem->GetAttribute("v3"));
		v1_ss >> submesh->faces.data.uints[face_index++];
		if (elem->HasAttribute("v2"))
			v2_ss >> submesh->faces.data.uints[face_index++];
		if (elem->HasAttribute("v3"))
			v3_ss >> submesh->faces.data.uints[face_index++];
	}

	void ParseFaces(Utilities::XMLElement *elem)
	{
		std::stringstream nv_ss(elem->GetAttribute("count"));
		nv_ss >> submesh->faces.numVals;
		submesh->faces.data.uints = new GLuint[submesh->faces.numVals*3];
		face_index = 0;
		elem->Iterate("face", ParseFace);
		submesh->faces.size = face_index * sizeof(GLfloat);
		submesh->faces.isElemBuffer = true;
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
			submesh->primitiveType = OgreSubMesh::PRIMITIVETYPE_TRIANGLESTRIP;
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

	void ParseMesh(Utilities::XMLElement *elem)
	{
		mesh = new OgreMesh;
		elem->Iterate("submeshes", ParseSubmeshes);
		elem->Iterate("sharedgeometry", ParseSharedGeometry);
	}

	OgreMesh* LoadOgreXMLModel(std::string filename)
	{
		Utilities::XMLReader xmlReader;
		xmlReader.Read(filename);

		xmlReader.root->Iterate("mesh", ParseMesh);

		xmlReader.Deallocate();
		return mesh;
	}
}
