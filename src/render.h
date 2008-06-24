#ifndef __RENDER_H__
#define __RENDER_H__

#ifdef DEBUG_DEP
#warning "render.h"
#endif

#include "ogrexmlmodel-pre.h"
#include "scenegraph.h"
#include "render-pre.h"
#include "materialxml-pre.h"
#include "ref_ptr.h"

namespace Scene
{
	namespace Render
	{
		class GLStateNode : public Graph::Node
		{
			protected:
				virtual void PreRender();
			public:
				struct GLState
				{
					// Edit curStateBools below when adding or removing states
					enum GLStateBool
					{
						GLSTATE_BLEND,
						GLSTATE_CULL_FACE,
						GLSTATE_DEPTH_TEST,
						GLSTATE_NUM
					};

					std::map<GLStateBool, bool> stateBools;
					Utilities::Material *material;

					GLState() : material(NULL)
					{
						
					}
				};

				typedef std::map<GLState::GLStateBool, bool> GLStateBoolList;

			protected:
				enum StateBoolState
				{
					STATEBOOLSTATE_ENABLED,
					STATEBOOLSTATE_DISABLED,
					STATEBOOLSTATE_UNKNOWN
				};

			private:
				static StateBoolState curStateBools[GLState::GLSTATE_NUM];
				static ref_ptr<GLState> curSetState;
			protected:
				ref_ptr<GLState> myGLState;
				void SetState(GLState::GLStateBool state, bool b);
			public:
				GLStateNode(ref_ptr<GLState> state);
		};

		class GeometryNode : public GLStateNode
		{
			protected:
				virtual void PreRender();
				virtual void Render();
			public:
				struct GeomState
				{

					struct AttribArray
					{
						std::string name;
						VBO *array;
						GLenum type;
					};

					VBO* vertexArray;
					VBO* normalArray;
					VBO* indicesArray;
					bool indicesAre32Bit;
					std::vector<VBO*> texCoordArrays;
					std::vector<AttribArray> attribArrays;
					GLenum primitive;
					unsigned numElems;

					GeomState() : vertexArray(NULL), normalArray(NULL), indicesArray(NULL), indicesAre32Bit(false), primitive(GL_TRIANGLES), numElems(0)
					{
						
					}
				};

			private:
				static unsigned curNumAttribArrays;
				static ref_ptr<GeomState> curSetState;
			protected:
				ref_ptr<GeomState> myGeomState;
			public:
				GeometryNode(ref_ptr<GeomState> geomState, ref_ptr<GLState> glState);
		};

		class OgreSubMeshNode : public GeometryNode
		{
			protected:
				virtual void Render();
			public:
				OgreSubMeshNode(Utilities::OgreSubMesh *mesh);
				virtual ~OgreSubMeshNode();
		};
		
		class OgreMeshNode : public Scene::Graph::Node
		{
			private:
				Utilities::OgreMesh* mesh;
			protected:
				virtual void ApplyMatrix();
				virtual void PostRender();
			public:
				OgreMeshNode(Utilities::OgreMesh *mesh);
		};


		
		class MeshTransformation
		{
			public:
				virtual void Apply(Utilities::Matrix4x4& matrix) = 0;
		};

		class MeshTranslation : public MeshTransformation
		{
			private:
				float x, y, z;
			public:
				MeshTranslation(float x, float y, float z) : x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
		};

		class MeshRotation : public MeshTransformation
		{
			private:
				float radians, x, y, z;
			public:
				MeshRotation(float radians, float x, float y, float z) : radians(radians), x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
		};

		class MeshScaling : public MeshTransformation
		{
			private:
				float x, y, z;
			public:
				MeshScaling(float x, float y, float z) : x(x), y(y), z(z)
				{
					
				}
				void Apply(Utilities::Matrix4x4& matrix);
		};

	}
}

#ifdef DEBUG_DEP
#warning "render.h-end"
#endif

#endif
