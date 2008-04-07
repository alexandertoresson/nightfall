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

		GeometryNode::GeometryNode(GeometryNode::GFXState *state)
		{
			myState = state;
		}
		
		void GeometryNode::SetState(GFXState::GFXStateBool state, bool b)
		{
			switch (state)
			{
				case GFXState::GFXSTATE_BLEND:
					if (b)
						glEnable(GL_BLEND);
					else
						glDisable(GL_BLEND);
					break;
				case GFXState::GFXSTATE_CULL_FACE:
					if (b)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);
					break;
				case GFXState::GFXSTATE_DEPTH_TEST:
					if (b)
						glEnable(GL_DEPTH_TEST);
					else
						glDisable(GL_DEPTH_TEST);
					break;
				case GFXState::GFXSTATE_FOG:
					if (b)
						glEnable(GL_FOG);
					else
						glDisable(GL_FOG);
					break;
				case GFXState::GFXSTATE_LIGHTING:
					if (b)
						glEnable(GL_LIGHTING);
					else
						glDisable(GL_LIGHTING);
					break;
				case GFXState::GFXSTATE_RESCALE_NORMAL:
					if (b)
						glEnable(GL_RESCALE_NORMAL);
					else
						glDisable(GL_RESCALE_NORMAL);
					break;
				case GFXState::GFXSTATE_TEXTURE_2D:
					if (b)
						glEnable(GL_TEXTURE_2D);
					else
						glDisable(GL_TEXTURE_2D);
					break;
				default:
					break;
			}
		}

		void GeometryNode::PreRender()
		{
			int i;
			if (curSetState != myState)
			{
				for (GFXStateBoolList::iterator it = myState->stateBools.begin(); it != myState->stateBools.end(); it++)
				{
					GFXState::GFXStateBool state = it->first;
					bool b = it->second;
					if (curStateBools[state] != STATEBOOLSTATE_ENABLED && b || curStateBools[state] != STATEBOOLSTATE_DISABLED && !b)
					{
						SetState(state, b);
					}
				}

				glUseProgramObjectARB(myState->program);

				glEnableClientState(GL_VERTEX_ARRAY);
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, myState->vertexArray);
				glVertexPointer(3, GL_FLOAT, 0, NULL);
	
				if (myState->normalArray)
				{
					glEnableClientState(GL_NORMAL_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, myState->normalArray);
					glNormalPointer(GL_FLOAT, 0, NULL);
				}
				else
				{
					glDisableClientState(GL_NORMAL_ARRAY);
				}

				i = 0;
				for (std::vector<GFXState::AttribArray>::iterator it = myState->attribArrays.begin(); it != myState->attribArrays.end(); it++, i++)
				{
					glBindAttribLocationARB(myState->program, i, it->name.c_str());
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->array);
					glVertexAttribPointerARB(i, it->size, it->type, GL_FALSE, 0, NULL);
				}

				for (std::vector<GFXState::Uniform4f>::iterator it = myState->uniform4fs.begin(); it != myState->uniform4fs.end(); it++)
				{
						GLuint id = glGetUniformLocationARB(myState->program, it->name.c_str());
						glUniform4fvARB(id, 4, (GLfloat*) &it->val);
				}

				for (std::vector<GFXState::UniformInt>::iterator it = myState->uniformInts.begin(); it != myState->uniformInts.end(); it++)
				{
						GLuint id = glGetUniformLocationARB(myState->program, it->name.c_str());
						glUniform1iARB(id, it->val);
				}

				if (myState->textures.size())
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				i = 0;
				for (std::vector<GFXState::Texture>::iterator it = myState->textures.begin(); it != myState->textures.end(); it++, i++)
				{
					if (myState->program)
					{
						GLuint sampler = glGetUniformLocationARB(myState->program, it->name.c_str());
						glUniform1iARB(sampler, i);
					}

					glActiveTexture(GL_TEXTURE0_ARB + i);
					glBindTexture(GL_TEXTURE_2D, it->texture);
					
					if (it->texCoords)
					{
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->texCoords);
						glTexCoordPointer(2, GL_FLOAT, 0, NULL);
					}
				}

				if (myState->indicesArray)
				{
					glEnableClientState(GL_INDEX_ARRAY);
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, myState->indicesArray);
					glIndexPointer(GL_INT, 0, NULL);
				}
				else
				{
					glDisableClientState(GL_INDEX_ARRAY);
				}

			}
			curSetState = myState;
		}

		void GeometryNode::Render()
		{
			if (myState->indicesArray)
			{
				glDrawElements(myState->primitive, myState->numVertices, GL_UNSIGNED_INT, NULL);
			}
			else
			{
				glDrawArrays(myState->primitive, 0, myState->numVertices);
			}
		}
		
		void GeometryNode::PostRender()
		{
		}
		
		GeometryNode::StateBoolState GeometryNode::curStateBools[GeometryNode::GFXState::GFXSTATE_NUM] =
			{
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN,
				GeometryNode::STATEBOOLSTATE_UNKNOWN
			};
		GeometryNode::GFXState *GeometryNode::curSetState = NULL;
		unsigned GeometryNode::curNumAttribArrays = 0;

		Node rootNode;
	}
}
