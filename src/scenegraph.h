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
		class Node : public gc_ptr_from_this<Node>
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

				std::map<int, gc_ptr<Node> > children;
				gc_ptr<Node> parent;
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

				virtual void AddChild(gc_ptr<Node> node, int placement);
				virtual void AddChild(gc_ptr<Node> node);
				virtual void RemoveChild(gc_ptr<Node> node);

				virtual void DeleteTree();
				virtual void Traverse();

				void SetEnabled(bool enabled);

				Utilities::Matrix4x4 GetMatrix(MatrixType type, gc_ptr<Node> baseNode = NULL);
				
				void BuildMatrices(gc_ptr<Node> baseNode);
				static void ResetMatrices();

				virtual bool HandleEvent(SDL_Event* event);

				virtual void shade()
				{
					gc_shade_map(children);
				}

				static void TraverseFullTree();

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

		extern gc_root_ptr<Node>::type rootNode;
	}
}

#ifdef DEBUG_DEP
#warning "scenegraph.h-end"
#endif

#endif
