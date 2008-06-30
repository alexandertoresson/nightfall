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
		class Node
		{
			public:
				enum MatrixType
				{
					MATRIXTYPE_VIEWPORT = 0,
					MATRIXTYPE_MODELVIEW,
					MATRIXTYPE_NUM
				};

			protected:
		
				virtual void ApplyMatrix();
				virtual void PreRender();
				virtual void Render();
				virtual void PostRender();

				std::map<int, ref_ptr<Node> > children;
				Node* parent;
				static Utilities::Matrix4x4 matrices[MATRIXTYPE_NUM];
				static std::stack<Utilities::Matrix4x4> mtxStack[MATRIXTYPE_NUM];
				bool enabled;

				void PushMatrix(MatrixType matType);
				void PopMatrix(MatrixType matType);

				virtual void TraverseAllChildren();
				virtual bool SendEventToAllChildren(SDL_Event* event);

				int lastPlacement;

			public:
				Node();
				virtual ~Node();

				virtual void AddChild(ref_ptr<Node> node, int placement);
				virtual void AddChild(ref_ptr<Node> node);
				virtual void RemoveChild(ref_ptr<Node> node);

				virtual void DeleteTree();
				virtual void Traverse();

				void SetEnabled(bool enabled);

				Utilities::Matrix4x4 GetMatrix(MatrixType type, ref_ptr<Node> baseNode = NULL);
				
				void BuildMatrices(ref_ptr<Node> baseNode);
				static void ResetMatrices();

				virtual bool HandleEvent(SDL_Event* event);

		};

		class SwitchNode : public Node
		{
			private:
				int active;
			protected:
				virtual void Render();
			public:
				SwitchNode();
				SwitchNode(int a);

				void SetActive(int a);
		};

		class MatrixNode : public Node
		{
			private:
				Utilities::Matrix4x4 matrix;
				MatrixType matType;
			protected:
				virtual void ApplyMatrix();
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
