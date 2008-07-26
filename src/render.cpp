#include "render.h"

#include "matrix4x4.h"
#include "ogrexmlmodel.h"
#include "utilities.h"
#include "terrain.h"
#include "unitrender.h"
#include "camera.h"
#include "unit.h"
#include <iostream>

namespace Scene
{
	namespace Render
	{
		
		void VBO::Lock()
		{
			if (!buffer)
			{
				glGenBuffersARB(1, &buffer);
			}
			if (changed)
			{
				if (isElemBuffer)
				{
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
					glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, data.pnt, mode);
				}
				else
				{
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data.pnt, mode);
				}
				changed = false;
			}
		}

		void VBO::Unlock()
		{
			
		}

		void GLState::SetState(GLStateBool state, bool b)
		{
			switch (state)
			{
				case GLState::GLSTATE_BLEND:
					if (b)
						glEnable(GL_BLEND);
					else
						glDisable(GL_BLEND);
					break;
				case GLState::GLSTATE_CULL_FACE:
					if (b)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);
					break;
				case GLState::GLSTATE_DEPTH_TEST:
					if (b)
						glEnable(GL_DEPTH_TEST);
					else
						glDisable(GL_DEPTH_TEST);
					break;
				default:
					break;
			}
		}

		void GLState::Apply()
		{
			int i;
			if (curSetState != GetRef())
			{
				if (curSetState && curSetState->material)
				{
					for (std::map<std::string, gc_ptr<Utilities::TextureImageData> >::iterator it = curSetState->material->textures.begin(); it != curSetState->material->textures.end(); it++)
						it->second->Unlock();
				}

				for (GLStateBoolList::iterator it = stateBools.begin(); it != stateBools.end(); it++)
				{
					GLStateBool state = it->first;
					bool b = it->second;
					if ((curStateBools[state] != STATEBOOLSTATE_ENABLED && b) || (curStateBools[state] != STATEBOOLSTATE_DISABLED && !b))
					{
						SetState(state, b);
					}
				}

				if (material)
				{
					glUseProgramObjectARB(material->program);

					for (std::vector<gc_ptr<Utilities::Uniform> >::iterator it = material->uniforms.begin(); it != material->uniforms.end(); it++)
					{
						GLint id = glGetUniformLocationARB(material->program, (*it)->name.c_str());
						if (id != -1)
						{
							(*it)->Set(id);
						}
						else
						{
	//						std::cout << "Uniform " << it->name << " not found!" << std::endl;
						}
					}

					i = 0;
					for (std::map<std::string, gc_ptr<Utilities::TextureImageData> >::iterator it = material->textures.begin(); it != material->textures.end(); it++, i++)
					{
						it->second->Lock();

						if (material->program)
						{
							GLint sampler = glGetUniformLocationARB(material->program, it->first.c_str());
							glUniform1iARB(sampler, i);
						}

						glActiveTexture(GL_TEXTURE0_ARB + i);
						glBindTexture(GL_TEXTURE_2D, it->second->buffer);

						i++;
					}

					glActiveTexture(GL_TEXTURE0_ARB);

					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material->emission.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &material->shininess);
				}
				else
				{
					glUseProgramObjectARB(0);
				}
				
			}

			curSetState = GetRef();

		}

		void GLState::ResetState()
		{
			curSetState = NULL;
		}

		gc_ptr<GLState> GLState::curSetState = NULL;

		GLState::StateBoolState GLState::curStateBools[GLState::GLSTATE_NUM] =
			{
				GLState::STATEBOOLSTATE_UNKNOWN,
				GLState::STATEBOOLSTATE_UNKNOWN,
				GLState::STATEBOOLSTATE_UNKNOWN
			};

		GLStateNode::GLStateNode(gc_ptr<GLState> state)
		{
			myGLState = state;
		}
		
		void GLStateNode::PreRender()
		{
			Node::PreRender();
			myGLState->Apply();
		}

		void GeomState::Apply(GLhandleARB program)
		{
			int i;
			if (curSetState != GetRef())
			{
				if (curSetState)
				{
					if (curSetState->indicesArray)
						curSetState->indicesArray->Unlock();
					if (curSetState->normalArray)
						curSetState->normalArray->Unlock();
					if (curSetState->vertexArray)
						curSetState->vertexArray->Unlock();
					for (std::vector<GeomState::AttribArray>::iterator it = curSetState->attribArrays.begin(); it != curSetState->attribArrays.end(); it++)
						it->array->Unlock();
					for (std::vector<gc_ptr<VBO> >::iterator it = curSetState->texCoordArrays.begin(); it != curSetState->texCoordArrays.end(); it++, i++)
						(*it)->Unlock();
				}

				if (vertexArray)
				{
					vertexArray->Lock();
					glEnableClientState(GL_VERTEX_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexArray->buffer);
					glVertexPointer(3, GL_FLOAT, 0, NULL);
				}
	
				if (normalArray)
				{
					normalArray->Lock();
					glEnableClientState(GL_NORMAL_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalArray->buffer);
					glNormalPointer(GL_FLOAT, 0, NULL);
				}
				else
				{
					glDisableClientState(GL_NORMAL_ARRAY);
				}

				i = 0;
				for (std::vector<AttribArray>::iterator it = attribArrays.begin(); it != attribArrays.end(); it++, i++)
				{
					it->array->Lock();
					glBindAttribLocationARB(program, i, it->name.c_str());
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->array->buffer);
					glVertexAttribPointerARB(i, it->array->size, it->type, GL_FALSE, 0, NULL);
				}

				if (texCoordArrays.size())
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				i = 0;
				for (std::vector<gc_ptr<VBO> >::iterator it = texCoordArrays.begin(); it != texCoordArrays.end(); it++, i++)
				{
					const gc_ptr<VBO>& vbo = *it;
					
					vbo->Lock();
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->buffer);
					glTexCoordPointer(2, GL_FLOAT, 0, NULL);

					i++;
				}

				if (vertexArray)
				{
					if (indicesArray)
					{
						indicesArray->Lock();
						glEnableClientState(GL_INDEX_ARRAY);
						glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicesArray->buffer);
						glIndexPointer(indicesAre32Bit ? GL_INT : GL_SHORT, 0, NULL);
					}
					else
					{
						glDisableClientState(GL_INDEX_ARRAY);
					}
				}

			}

			curSetState = GetRef();

		}

		void GeomState::Draw()
		{
			if (indicesArray)
			{
				glDrawElements(primitive, numElems, indicesAre32Bit ?  GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, NULL);
			}
			else
			{
				glDrawArrays(primitive, 0, numElems);
			}

		}

		void GeomState::ResetState()
		{
			curSetState = NULL;
		}

		gc_ptr<GeomState> GeomState::curSetState = NULL;

		GeometryNode::GeometryNode(gc_ptr<GeomState> geomState, gc_ptr<GLState> glState) : GLStateNode(glState)
		{
			myGeomState = geomState;
		}
		
		void GeometryNode::PreRender()
		{
			GLStateNode::PreRender();
			myGeomState->Apply(myGLState->material->program);
		}

		void GeometryNode::Render()
		{
			matrices[MATRIXTYPE_MODELVIEW].Apply();

			myGeomState->Draw();
		}
		
		OgreSubMeshNode::OgreSubMeshNode(const gc_ptr<Utilities::OgreSubMesh>& submesh) : GeometryNode(new GeomState, new GLState)
		{
			const gc_ptr<Utilities::OgreVertexBuffer>& vb = submesh->vbs[0];
			myGeomState->indicesArray = submesh->faces;
			myGeomState->vertexArray = vb->positions;
			myGeomState->normalArray = vb->normals;
			switch (submesh->primitiveType)
			{
				case Utilities::OgreSubMesh::PRIMITIVETYPE_TRIANGLELIST:
					myGeomState->primitive = GL_TRIANGLES;
					break;
				case Utilities::OgreSubMesh::PRIMITIVETYPE_TRIANGLESTRIP:
					myGeomState->primitive = GL_TRIANGLE_STRIP;
					break;
				case Utilities::OgreSubMesh::PRIMITIVETYPE_TRIANGLEFAN:
					myGeomState->primitive = GL_TRIANGLE_FAN;
					break;
			}
			myGeomState->indicesAre32Bit = true;
			myGeomState->numElems = submesh->numElems;
			for (std::vector<gc_ptr<VBO> >::iterator it = vb->texCoords.begin(); it != vb->texCoords.end(); it++)
			{
				myGeomState->texCoordArrays.push_back(*it);
			}
			myGLState->material = submesh->material;
		}

		OgreSubMeshNode::~OgreSubMeshNode()
		{
		}

		void OgreSubMeshNode::Render()
		{
			GeometryNode::Render();
		}

		void OgreMeshNode::ApplyMatrix()
		{
			if (mesh->transforms.size())
			{
				PushMatrix(MATRIXTYPE_MODELVIEW);

				Utilities::Matrix4x4& mVMatrix = matrices[MATRIXTYPE_MODELVIEW];

				for (std::vector<gc_ptr<MeshTransformation> >::iterator it = mesh->transforms.begin(); it != mesh->transforms.end(); it++)
				{
					(*it)->Apply(mVMatrix);
				}
			}
		}

		void OgreMeshNode::PostRender()
		{
			if (mesh->transforms.size())
				PopMatrix(MATRIXTYPE_MODELVIEW);
		}

		OgreMeshNode::OgreMeshNode(const gc_ptr<Utilities::OgreMesh> mesh)
		{
			for (std::vector<gc_ptr<Utilities::OgreSubMesh> >::iterator it = mesh->submeshes.begin(); it != mesh->submeshes.end(); it++)
			{
				this->AddChild(new OgreSubMeshNode(*it));
			}
		}
				
		void MeshTranslation::Apply(Utilities::Matrix4x4& matrix)
		{
			matrix.Translate(x, y, z);
		}
		
		void MeshRotation::Apply(Utilities::Matrix4x4& matrix)
		{
			matrix.Rotate(radians, x, y, z);
		}
		
		void MeshScaling::Apply(Utilities::Matrix4x4& matrix)
		{
			matrix.Scale(x, y, z);
		}
	}
}

