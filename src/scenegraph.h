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
			protected:
				virtual void PreRender();
				virtual void Render();
				virtual void PostRender();

				std::vector<Node*> children;
				Node* parent;
				static Utilities::Matrix4x4 matrices[MATRIXTYPE_NUM];
				static std::stack<Utilities::Matrix4x4> mtxStack[MATRIXTYPE_NUM];

				virtual void TraverseAllChildren();
				virtual void SendEventToAllChildren(Utilities::Vector3D v);

			public:
				Node();
				virtual ~Node();

				virtual void AddChild(Node* node);
				virtual void RemoveChild(Node* node);

				virtual void DeleteTree();		
				virtual void Traverse();

				Utilities::Matrix4x4 GetMatrix(MatrixType type);
				
				void BuildMatrices();
				static void ResetMatrices();

				virtual void HandleEvent(Utilities::Vector3D v);

		};

		class SwitchNode : public Node
		{
			private:
				unsigned active;
			protected:
				virtual void Render();
			public:
				SwitchNode();
				SwitchNode(unsigned a);

				void SetActive(unsigned a);
		};

		class MatrixNode : public Node
		{
			private:
				Utilities::Matrix4x4 matrix;
				MatrixType matType;
			protected:
				virtual void PreRender();
				virtual void PostRender();
			public:
				MatrixNode(Utilities::Matrix4x4 mat, MatrixType matType);
				void SetMatrix(Utilities::Matrix4x4 mat, MatrixType matType);
		};

		extern Node rootNode;
	}
}

#ifdef DEBUG_DEP
#warning "scenegraph.h-end"
#endif

#endif
