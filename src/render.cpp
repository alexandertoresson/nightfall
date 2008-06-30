#include "render.h"

#include "matrix4x4.h"
#include "ogrexmlmodel.h"
#include "utilities.h"
#include "terrain.h"
#include "unitrender.h"
#include "camera.h"
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
				if (isElemBuffer)
				{
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer);
					glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, data.pnt, GL_STATIC_DRAW_ARB);
				}
				else
				{
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data.pnt, GL_STATIC_DRAW_ARB);
				}
			}
		}

		void VBO::Unlock()
		{
			
		}

		void GLStateNode::SetState(GLState::GLStateBool state, bool b)
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

		GLStateNode::GLStateNode(ref_ptr<GLStateNode::GLState> state)
		{
			myGLState = state;
		}
		
		void GLStateNode::PreRender()
		{
			Node::PreRender();
			int i;
			if (curSetState != myGLState)
			{
				if (curSetState && curSetState->material)
				{
					for (std::vector<ref_ptr<Utilities::TextureImageData> >::iterator it = curSetState->material->textures.begin(); it != curSetState->material->textures.end(); it++)
						(*it)->Unlock();
				}

				for (GLStateBoolList::iterator it = myGLState->stateBools.begin(); it != myGLState->stateBools.end(); it++)
				{
					GLState::GLStateBool state = it->first;
					bool b = it->second;
					if ((curStateBools[state] != STATEBOOLSTATE_ENABLED && b) || (curStateBools[state] != STATEBOOLSTATE_DISABLED && !b))
					{
						SetState(state, b);
					}
				}

				if (myGLState->material)
				{
					glUseProgramObjectARB(myGLState->material->program);

					for (std::vector<ref_ptr<Utilities::Uniform> >::iterator it = myGLState->material->uniforms.begin(); it != myGLState->material->uniforms.end(); it++)
					{
						GLint id = glGetUniformLocationARB(myGLState->material->program, (*it)->name.c_str());
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
					for (std::vector<ref_ptr<Utilities::TextureImageData> >::iterator it = myGLState->material->textures.begin(); it != myGLState->material->textures.end(); it++, i++)
					{
						(*it)->Lock();

						if (myGLState->material->program)
						{
							GLint sampler = glGetUniformLocationARB(myGLState->material->program, (*it)->name.c_str());
							glUniform1iARB(sampler, i);
						}

						glActiveTexture(GL_TEXTURE0_ARB + i);
						glBindTexture(GL_TEXTURE_2D, (*it)->buffer);

						i++;
					}

					glActiveTexture(GL_TEXTURE0_ARB);

					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, myGLState->material->ambient.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, myGLState->material->diffuse.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, myGLState->material->specular.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, myGLState->material->emission.val);
					glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &myGLState->material->shininess);
				}
				else
				{
					glUseProgramObjectARB(0);
				}
				
			}

			curSetState = myGLState;

		}

		ref_ptr<GLStateNode::GLState> GLStateNode::curSetState = NULL;

		GLStateNode::StateBoolState GLStateNode::curStateBools[GeometryNode::GLState::GLSTATE_NUM] =
			{
				GLStateNode::STATEBOOLSTATE_UNKNOWN,
				GLStateNode::STATEBOOLSTATE_UNKNOWN,
				GLStateNode::STATEBOOLSTATE_UNKNOWN
			};

		GeometryNode::GeometryNode(ref_ptr<GeometryNode::GeomState> geomState, ref_ptr<GLStateNode::GLState> glState) : GLStateNode(glState)
		{
			myGeomState = geomState;
		}
		
		void GeometryNode::PreRender()
		{
			GLStateNode::PreRender();
			int i;
			if (curSetState != myGeomState)
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
					for (std::vector<ref_ptr<VBO> >::iterator it = curSetState->texCoordArrays.begin(); it != curSetState->texCoordArrays.end(); it++, i++)
						(*it)->Unlock();
				}

				if (myGeomState->vertexArray)
				{
					myGeomState->vertexArray->Lock();
					glEnableClientState(GL_VERTEX_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, myGeomState->vertexArray->buffer);
					glVertexPointer(3, GL_FLOAT, 0, NULL);
				}
	
				if (myGeomState->normalArray)
				{
					myGeomState->normalArray->Lock();
					glEnableClientState(GL_NORMAL_ARRAY);
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, myGeomState->normalArray->buffer);
					glNormalPointer(GL_FLOAT, 0, NULL);
				}
				else
				{
					glDisableClientState(GL_NORMAL_ARRAY);
				}

				i = 0;
				for (std::vector<GeomState::AttribArray>::iterator it = myGeomState->attribArrays.begin(); it != myGeomState->attribArrays.end(); it++, i++)
				{
					it->array->Lock();
					glBindAttribLocationARB(myGLState->material->program, i, it->name.c_str());
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, it->array->buffer);
					glVertexAttribPointerARB(i, it->array->size, it->type, GL_FALSE, 0, NULL);
				}

				if (myGeomState->texCoordArrays.size())
				{
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				}
				else
				{
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				}

				i = 0;
				for (std::vector<ref_ptr<VBO> >::iterator it = myGeomState->texCoordArrays.begin(); it != myGeomState->texCoordArrays.end(); it++, i++)
				{
					const ref_ptr<VBO>& vbo = *it;
					
					vbo->Lock();
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->buffer);
					glTexCoordPointer(2, GL_FLOAT, 0, NULL);

					i++;
				}

				if (myGeomState->vertexArray)
				{
					if (myGeomState->indicesArray)
					{
						myGeomState->indicesArray->Lock();
						glEnableClientState(GL_INDEX_ARRAY);
						glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, myGeomState->indicesArray->buffer);
						glIndexPointer(myGeomState->indicesAre32Bit ? GL_INT : GL_SHORT, 0, NULL);
					}
					else
					{
						glDisableClientState(GL_INDEX_ARRAY);
					}
				}

			}

			curSetState = myGeomState;

		}

		void GeometryNode::Render()
		{
			matrices[MATRIXTYPE_MODELVIEW].Apply();

			if (myGeomState->indicesArray)
			{
				glDrawElements(myGeomState->primitive, myGeomState->numElems, myGeomState->indicesAre32Bit ?  GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, NULL);
			}
			else
			{
				glDrawArrays(myGeomState->primitive, 0, myGeomState->numElems);
			}

		}
		
		ref_ptr<GeometryNode::GeomState> GeometryNode::curSetState = NULL;
		unsigned GeometryNode::curNumAttribArrays = 0;

		OgreSubMeshNode::OgreSubMeshNode(const ref_ptr<Utilities::OgreSubMesh>& submesh) : GeometryNode(new GeomState, new GLState)
		{
			const ref_ptr<Utilities::OgreVertexBuffer>& vb = submesh->vbs[0];
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
			for (std::vector<ref_ptr<VBO> >::iterator it = vb->texCoords.begin(); it != vb->texCoords.end(); it++)
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

				for (std::vector<ref_ptr<MeshTransformation> >::iterator it = mesh->transforms.begin(); it != mesh->transforms.end(); it++)
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

		OgreMeshNode::OgreMeshNode(const ref_ptr<Utilities::OgreMesh> mesh)
		{
			for (std::vector<ref_ptr<Utilities::OgreSubMesh> >::iterator it = mesh->submeshes.begin(); it != mesh->submeshes.end(); it++)
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

