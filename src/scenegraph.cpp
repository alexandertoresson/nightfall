#include "scenegraph.h"

namespace Scene
{
	namespace Graph
	{
		Utilities::Matrix4x4 Node::matrices[MATRIXTYPE_NUM];
		std::stack<Utilities::Matrix4x4> Node::mtxStack[MATRIXTYPE_NUM];

		Node::Node() : parent(NULL)
		{
			
		}

		Node::~Node()
		{
			
		}

		void Node::PreRender()
		{
			
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
			PreRender();
			Render();
			PostRender();
		}

		void Node::DeleteTree()
		{
			for (std::vector<Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				(*it)->DeleteTree();
			}
			delete this;
		}

		void Node::HandleEvent(Utilities::Vector3D v)
		{
			SendEventToAllChildren(v);
		}

		void Node::AddChild(Node* node)
		{
			children.push_back(node);
			if (node->parent)
			{
				node->parent->RemoveChild(node);
			}
			node->parent = this;
		}

		void Node::RemoveChild(Node* node)
		{
			for (std::vector<Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				if (*it == node)
				{
					children.erase(it);
					break;
				}
			}
			node->parent = NULL;
		}

		void Node::TraverseAllChildren()
		{
			for (std::vector<Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				(*it)->Traverse();
			}
		}

		void Node::SendEventToAllChildren(Utilities::Vector3D v)
		{
			for (std::vector<Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				(*it)->HandleEvent(v);
			}
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

		void Node::BuildMatrices()
		{
			if (parent)
			{
				parent->BuildMatrices();
			}
			else
			{
				ResetMatrices();
			}
			PreRender();
		}

		Utilities::Matrix4x4 Node::GetMatrix(MatrixType type)
		{
			BuildMatrices();
			if (type >= 0 && type <= MATRIXTYPE_NUM)
			{
				return matrices[type];
			}
			else
			{
				return Utilities::Matrix4x4();
			}
		}

		SwitchNode::SwitchNode() : Node(), active(0)
		{
			
		}

		SwitchNode::SwitchNode(unsigned a) : Node(), active(a)
		{
			
		}

		void SwitchNode::SetActive(unsigned a)
		{
			active = a;
		}

		void SwitchNode::Render()
		{
			if (active < children.size())
			{
				children[active]->Traverse();
			}
		}

		MatrixNode::MatrixNode(Utilities::Matrix4x4 mat, MatrixType matType)
		{
			matrix.Set(mat.matrix);
			this->matType = matType;
		}

		void MatrixNode::PreRender()
		{
			mtxStack[matType].push(matrices[matType]);
		}

		void MatrixNode::PostRender()
		{
			mtxStack[matType].pop();
		}

	}
}
