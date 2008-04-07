#ifndef __SCENEGRAPH_H__
#define __SCENEGRAPH_H__

#ifdef DEBUG_DEP
#warning "scenegraph.h"
#endif

#include "matrix4x4.h"
#include "vector3d.h"
#include "sdlheader.h"
#include <vector>
#include <stack>
#include <map>

namespace Scene
{
	namespace Graph
	{
		enum MatrixType
		{
			MATRIXTYPE_VIEWPORT = 0,
			MATRIXTYPE_MODELVIEW,
			MATRIXTYPE_NUM
		};

		class Node
		{
			private:
				virtual void PreRender();
				virtual void Render();
				virtual void PostRender();

			protected:
				std::vector<Node*> children;
				Node* parent;
				static Utilities::Matrix4x4 matrices[MATRIXTYPE_NUM];
				static std::stack<Utilities::Matrix4x4> mtxStack[MATRIXTYPE_NUM];

				void TraverseAllChildren();
				void SendEventToAllChildren(Utilities::Vector3D v);

			public:
				Node();
				virtual ~Node();

				void AddChild(Node* node);
				void RemoveChild(Node* node);

				void DeleteTree();		
				void Traverse();

				Utilities::Matrix4x4 GetMatrix(MatrixType type);
				
				void BuildMatrices();
				static void ResetMatrices();

				virtual void HandleEvent(Utilities::Vector3D v);

		};

		class SwitchNode : Node
		{
			private:
				unsigned active;

				virtual void Render();
			public:
				SwitchNode();
				SwitchNode(unsigned a);

				void SetActive(unsigned a);
		};

		class MatrixNode : Node
		{
			private:
				Utilities::Matrix4x4 matrix;
				MatrixType matType;
				virtual void PreRender();
				virtual void PostRender();
			public:
				MatrixNode(Utilities::Matrix4x4 mat, MatrixType matType);
				void SetMatrix(Utilities::Matrix4x4 mat, MatrixType matType);
		};

		class GeometryNode : Node
		{
			private:
				virtual void PreRender();
				virtual void Render();
				virtual void PostRender();
			public:
				struct GFXState
				{
					// Edit curStateBools below when adding or removing states
					enum GFXStateBool
					{
						GFXSTATE_BLEND,
						GFXSTATE_CULL_FACE,
						GFXSTATE_DEPTH_TEST,
						GFXSTATE_FOG,
						GFXSTATE_LIGHTING,
						GFXSTATE_RESCALE_NORMAL,
						GFXSTATE_TEXTURE_2D,
						GFXSTATE_NUM
					};

					struct AttribArray
					{
						std::string name;
						GLuint array;
						GLint size;
						GLenum type;
					};

					struct Uniform4f
					{
						std::string name;
						float val[4];
					};

					struct UniformInt
					{
						std::string name;
						int val;
					};

					struct Texture
					{
						std::string name;
						GLuint texture;
						GLuint texCoords;
					};

					std::map<GFXStateBool, bool> stateBools;
					GLuint program;
					GLuint vertexArray;
					GLuint normalArray;
					GLuint indicesArray;
					std::vector<Texture> textures;
					std::vector<AttribArray> attribArrays;
					std::vector<Uniform4f> uniform4fs;
					std::vector<UniformInt> uniformInts;
					GLenum primitive;
					unsigned numVertices;

					GFXState();
				};

				typedef std::map<GFXState::GFXStateBool, bool> GFXStateBoolList;

			private:
				enum StateBoolState
				{
					STATEBOOLSTATE_ENABLED,
					STATEBOOLSTATE_DISABLED,
					STATEBOOLSTATE_UNKNOWN
				};

				static StateBoolState curStateBools[GFXState::GFXSTATE_NUM];
				static GFXState *curSetState;
				static unsigned curNumAttribArrays;
				GFXState *myState;
				void SetState(GFXState::GFXStateBool state, bool b);
			public:
				GeometryNode(GFXState *state);
		};

		extern Node rootNode;
	}
}

#ifdef DEBUG_DEP
#warning "scenegraph.h-end"
#endif

#endif
