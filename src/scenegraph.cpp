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
#include "scenegraph.h"
#include "render.h"

namespace Scene
{
	namespace Graph
	{
		Utilities::Matrix4x4 Node::matrices[MATRIXTYPE_NUM];
		std::stack<Utilities::Matrix4x4> Node::mtxStack[MATRIXTYPE_NUM];

		Node::Node() : parent(NULL), enabled(true)
		{
		}

		Node::~Node()
		{
		}

		void Node::SetEnabled(bool enabled)
		{
			this->enabled = enabled;
		}

		void Node::ApplyMatrix()
		{
			
		}

		void Node::PreRender()
		{
			ApplyMatrix();
		}

		void Node::Render()
		{
			TraverseAllChildren();
		}

		void Node::PostRender()
		{
			
		}

		void Node::Traverse()

		{
			if (enabled)
			{
				PreRender();
				Render();
				PostRender();
			}
		}

		void Node::DeleteTree()
		{
			if (parent)
			{
				parent->RemoveChild(GetRef());
			}
		}

		bool Node::HandleEvent(SDL_Event* event)
		{
			return SendEventToAllChildren(event);
		}

		void Node::AddChild(gc_ptr<Node> node)
		{
			if (node->parent)
			{
				node->parent->RemoveChild(node);
			}
			node->parent = GetRef();
			children.push_back(node);
			node->pos_it = --children.end();
		}

		void Node::RemoveChild(gc_ptr<Node> node)
		{
			if (node->parent)
			{
				children.erase(node->pos_it);
				node->parent = NULL;
			}
		}

		void Node::Clear()
		{
			while (children.size())
				RemoveChild(children.back());
		}

		void Node::TraverseAllChildren()
		{
			for (std::list<gc_ptr<Node> >::iterator it = children.begin(); it != children.end(); it++)
			{
				(*it)->Traverse();
			}
		}

		bool Node::SendEventToAllChildren(SDL_Event* event)
		{
			for (std::list<gc_ptr<Node> >::reverse_iterator it = children.rbegin(); it != children.rend(); it++)
			{
				if ((*it)->HandleEvent(event))
				{
					return true;
				}
			}
			return false;
		}

		void Node::ResetMatrices()
		{
			for (int i = 0; i < MATRIXTYPE_NUM; i++)
			{
				matrices[i].Identity();
				while (!mtxStack[i].empty())
				{
					mtxStack[i].pop();
				}
			}
		}

		void Node::BuildMatrices(gc_ptr<Node> baseNode)
		{
			if (parent && baseNode != parent)
			{
				parent->BuildMatrices(baseNode);
			}
			else
			{
				ResetMatrices();
			}
			ApplyMatrix();
		}

		Utilities::Matrix4x4 Node::GetMatrix(MatrixType type, gc_ptr<Node> baseNode)
		{
			Utilities::Matrix4x4 mat;
			BuildMatrices(baseNode);
			if (type >= 0 && type <= MATRIXTYPE_NUM)
			{
				mat = matrices[type];
			}
			ResetMatrices();
			return mat;
		}

		void Node::PushMatrix(MatrixType matType)
		{
			Utilities::Matrix4x4& matrix = matrices[matType];
			mtxStack[matType].push(matrix);
		}

		void Node::PopMatrix(MatrixType matType)
		{
			matrices[matType] = mtxStack[matType].top();
			mtxStack[matType].pop();
		}

		void Node::TraverseFullTree()
		{
			Render::GLState::ResetState();
			Render::GeomState::ResetState();
			rootNode->Traverse();
		}

		SwitchNode::SwitchNode() : Node(), active(NULL)
		{
			
		}

		SwitchNode::SwitchNode(unsigned a) : Node()
		{
			SetActive(a);
		}

		void SwitchNode::SetActive(unsigned a)
		{
			std::list<gc_ptr<Node> >::iterator it = children.begin();
			for (unsigned i = 0; i < a; i++)
				it++;
			active = *it;
		}

		void SwitchNode::Render()
		{
			if (active)
			{
				active->Traverse();
			}
		}

		MatrixNode::MatrixNode(Utilities::Matrix4x4 mat, MatrixType matType)
		{
			matrix.Set(mat.matrix);
			this->matType = matType;
		}

		void MatrixNode::ApplyMatrix()
		{
			mtxStack[matType].push(matrices[matType]);
			matrices[matType] = matrices[matType] * matrix;
		}

		void MatrixNode::PostRender()
		{
			matrices[matType] = mtxStack[matType].top();
			mtxStack[matType].pop();
		}

		gc_root_ptr<Node>::type rootNode = NULL;
	}
}
