#include "scenegraph.h"

namespace Scene
{
	namespace Graph
	{
		Utilities::Matrix4x4 Node::matrices[MATRIXTYPE_NUM];
		std::stack<Utilities::Matrix4x4> Node::mtxStack[MATRIXTYPE_NUM];

		Node::Node() : parent(NULL), enabled(true), lastPlacement(0)
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

		void Node::DeleteTree(bool removeFromParent)
		{
			for (std::map<int, Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				// removeFromParent = false, or the child will attempt to remove itself from this
				it->second->DeleteTree(false);
			}
			children.clear();
			if (parent && removeFromParent)
			{
				parent->RemoveChild(this);
			}
			delete this;
		}

		bool Node::HandleEvent(SDL_Event* event)
		{
			return SendEventToAllChildren(event);
		}

		void Node::AddChild(Node* node, int placement)
		{
			children[placement] = node;
			if (node->parent)
			{
				node->parent->RemoveChild(node);
			}
			node->parent = this;
			if (placement > lastPlacement)
			{
				lastPlacement = placement;
			}
		}

		void Node::AddChild(Node* node)
		{
			AddChild(node, lastPlacement + 1);
		}

		void Node::RemoveChild(Node* node)
		{
			for (std::map<int, Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				if (it->second == node)
				{
					children.erase(it);
					break;
				}
			}
			node->parent = NULL;
		}

		void Node::TraverseAllChildren()
		{
			for (std::map<int, Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				it->second->Traverse();
			}
		}

		bool Node::SendEventToAllChildren(SDL_Event* event)
		{
			for (std::map<int, Node*>::reverse_iterator it = children.rbegin(); it != children.rend(); it++)
			{
				if (it->second->HandleEvent(event))
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

		void Node::BuildMatrices(Node* baseNode)
		{
			if (parent && parent != baseNode)
			{
				parent->BuildMatrices(baseNode);
			}
			else
			{
				ResetMatrices();
			}
			ApplyMatrix();
		}

		Utilities::Matrix4x4 Node::GetMatrix(MatrixType type, Node* baseNode)
		{
			Utilities::Matrix4x4 mat;
			BuildMatrices(baseNode);
			if (type >= 0 && type <= MATRIXTYPE_NUM)
			{
				mat = matrices[type];
			}
			else
			{
				mat = Utilities::Matrix4x4();
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

		SwitchNode::SwitchNode() : Node(), active(0)
		{
			
		}

		SwitchNode::SwitchNode(int a) : Node(), active(a)
		{
			
		}

		void SwitchNode::SetActive(int a)
		{
			active = a;
		}

		void SwitchNode::Render()
		{
			for (std::map<int, Node*>::iterator it = children.begin(); it != children.end(); it++)
			{
				if (it->first == active)
				{
					it->second->Traverse();
				}
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

		Node rootNode;
	}
}
