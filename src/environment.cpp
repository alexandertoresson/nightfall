#include "environment.h"

#include "dimension.h"
#include "console.h"
#include "paths.h"

namespace Game
{
	namespace Dimension
	{
		namespace Environment
		{
/*
			Vector4D::Vector4D(GLfloat i1 / * = 0.000000 * /, GLfloat i2 / * = 0.000000 * /, GLfloat i3 / * = 0.000000 * /, GLfloat i4 / * = 1.000000 * /)
			{
				m1 = i1;
				m2 = i2;
				m3 = i3;
				m4 = i4;

				arr = NULL;
			}

			Vector4D::~Vector4D(void)
			{
				if (arr != NULL)
					DeleteGLArray();
			}

			void Vector4D::BuildFromGLArray(GLfloat* arr)
			{
				m1 = arr[0];
				m2 = arr[1];
				m3 = arr[2];
				m4 = arr[3];
			}

			void Vector4D::BuildFromVAList(...)
			{
				va_list ap;
				int args = 4;
				va_start(ap, args);

				m1 = va_arg(ap, GLfloat);
				m2 = va_arg(ap, GLfloat);
				m3 = va_arg(ap, GLfloat);
				m4 = va_arg(ap, GLfloat);

				va_end(ap);
			}

			GLfloat* Vector4D::ToGLArray(void)
			{
				if (arr != NULL)
				{
					if (arr[0] == m1 && arr[1] == m2 && arr[2] == m3 && arr[3] == m4)
						return arr;
					else
						DeleteGLArray();
				}
				
				GLfloat* arr = new GLfloat[4];
				
				arr[0] = m1;
				arr[1] = m2;
				arr[2] = m3;
				arr[3] = m4;

				return arr;
			}

			const char* Vector4D::ToString(void)
			{
				std::ostringstream ss;
				ss << "{ ";
				ss << m1;
				ss << ", ";
				ss << m2;
				ss << ", ";
				ss << m3;
				ss << ", ";
				ss << m4;
				ss << " }";

				return ss.str().c_str();
			}

			void Vector4D::DeleteGLArray(void)
			{
				if (arr != NULL)
					delete [] arr;

				arr = NULL;
			}
*/

			FourthDimension* FourthDimension::m_pInstance = NULL;

			FourthDimension* FourthDimension::Instance(void)
			{
				if (m_pInstance == NULL)
					m_pInstance = new FourthDimension;

				return m_pInstance;
			}

			void FourthDimension::Destroy(void)
			{
				if (!m_pInstance)
					return;

				delete m_pInstance;
				m_pInstance = NULL;
			}

			FourthDimension* FourthDimension::ReallocInstance(void)
			{
				if (m_pInstance)
					FourthDimension::Destroy();

				return FourthDimension::Instance();
			}

			FourthDimension::~FourthDimension(void)
			{
				EnvironmentalCondition* cond = m_envFirst->_next;
				EnvironmentalCondition* temp = NULL;
				while(cond != m_envFirst)
				{
					temp = &*cond;
					cond = cond->_next;
					delete temp;
				}
				delete m_envFirst;

				if (m_skyboxesCount > 0)
				{
					for (int i = 0; i < m_skyboxCurIndex; i++)
					{
						SkyBox* box = m_skyboxes[i];
						glDeleteTextures(1, &box->texture);
						delete box;
						box = NULL;
						m_skyboxes[i] = NULL;
					}

					delete [] m_skyboxes;

					m_skyboxes = NULL;
					m_skyboxCurIndex = 0;
					m_skyboxesCount = 0;
					
					m_skyboxesGLContainer.clear();
				}
			}

			void FourthDimension::SetHourLength(const float val)
			{
				m_hourLength = val;
			}

			float FourthDimension::GetHourLength(void) const
			{
				return m_hourLength;
			}

			void FourthDimension::SetDayLength(const int val)
			{
				m_dayLength = val;
			}

			int FourthDimension::GetDayLength(void) const
			{
				return m_dayLength;
			}

			void FourthDimension::SetCurrentHour(const float val)
			{
				m_currentHour = val;
				if (CheckEnvironmentalConditions())
					ApplyEnvironmentalConditions();
			}

			float FourthDimension::GetCurrentHour(void) const
			{
				return m_currentHour;
			}
			
			bool FourthDimension::CheckEnvironmentalConditions(void)
			{
				if (!m_prepared)
					return false;

				return true;
			}

			void FourthDimension::CalculateChange(void)
			{
				EnvironmentalCondition *firstChecked = m_curEnvCond;
				while (1)
				{
					if (m_curEnvCond->hourBegin < m_curEnvCond->hourEnd)
					{
						if (m_currentHour >= m_curEnvCond->hourBegin && m_currentHour < m_curEnvCond->hourEnd)
						{
							break;
						}
					}
					else
					{
						if ((m_currentHour >= m_curEnvCond->hourBegin && m_currentHour <= m_dayLength) ||
						    (m_currentHour >= 0 && m_currentHour <= m_curEnvCond->hourEnd))
						{
							break;
						}
					}

					m_curEnvCond = m_curEnvCond->_next;
					if (m_curEnvCond == firstChecked)
					{
						console << "Invalid new time set!" << Console::nl;
						return;
					}
				}

				m_nextEnvCond = m_curEnvCond->_next;

				float length = CalculateLength(m_curEnvCond);

				console << "Length: " << length << ", Seconds: " <<
					(m_hourLength * length) << ", current time: " << m_currentHour << "\n";

				int i = 0;
				for ( ; i < 4; i++)
					m_curAmbient[i] = m_curEnvCond->ambient[i];

				for (i = 0; i < 4; i++)
					m_curDiffuse[i] = m_curEnvCond->diffuse[i];

				for (i = 0; i < 4; i++)
					m_curSunPos[i] = m_curEnvCond->sunPos[i];

				for (i = 0; i < 4; i++)
					m_curFogColor[i] = m_curEnvCond->fogColor[i];

				m_curFogIntensity = m_curEnvCond->fogIntensity;
				m_curFogBegin = m_curEnvCond->fogBegin;
				m_curFogEnd = m_curEnvCond->fogEnd;
				m_curAlpha = 0.0f;

				if (m_curEnvCond->terrainMaterialAmbientDiffuse[0][0][0] >= 0.0f)
				{
					for (i = 0; i < 2; i++)
					{
						for (int j = 0; j < 2; j++)
						{
							for (int k = 0; k < 4; k++)
								Dimension::terrainMaterialAmbientDiffuse[i][j][k] = m_curEnvCond->terrainMaterialAmbientDiffuse[i][j][k];
						}
					}
				}

				if (m_curEnvCond->waterMaterialAmbientDiffuse[0][0][0] >= 0.0f)
				{
					for (i = 0; i < 2; i++)
					{
						for (int j = 0; j < 2; j++)
						{
							for (int k = 0; k < 4; k++)
								Dimension::waterMaterialAmbientDiffuse[i][j][k] = m_curEnvCond->waterMaterialAmbientDiffuse[i][j][k];
						}
					}
				}

			}

			bool FourthDimension::ApplyEnvironmentalConditions(EnvironmentalCondition* const cond /* = NULL */)
			{
				if (!m_prepared)
					return false;
					
				if (cond != NULL)
					m_curEnvCond = cond;

				CalculateChange();
				
				return true;
			}

			void FourthDimension::SetDefaultSun(void) const
			{
				GLfloat defSunPos[4]       = { 1024.0f, 1024.0f, 1024.0f, 1.0f };
				GLfloat defLightAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
				GLfloat defLightDiffuse[4] = { 1.5f, 1.5f, 1.5f, 1.0f };

				// Enable ambient light for Light 0
				glLightfv(GL_LIGHT0, GL_AMBIENT,  defLightAmbient);

				// Enable diffuse light for Light 0
				glLightfv(GL_LIGHT0, GL_DIFFUSE,  defLightDiffuse);

				// Set the position of Light 0
				glLightfv(GL_LIGHT0, GL_POSITION, defSunPos);

				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

				glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
				glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);
				glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);

				// Enable Light 0
				glEnable(GL_LIGHT0);

				// Enable lighting
				glEnable(GL_LIGHTING);
			}

			void FourthDimension::RotateWorld(float lastPass)
			{
				if (!m_prepared)
					return;

				m_currentHour += lastPass / m_hourLength;
				if (m_curEnvCond->hourBegin > m_curEnvCond->hourEnd)
				{
					m_progress = m_currentHour - m_curEnvCond->hourBegin;

					if (m_progress < 0)
						m_progress += m_dayLength;

					m_progress /= m_curEnvCond->hourEnd - m_curEnvCond->hourBegin + m_dayLength;
				}
				else
				{
					m_progress = (m_currentHour - m_curEnvCond->hourBegin) / (m_curEnvCond->hourEnd - m_curEnvCond->hourBegin);
				}

				if (m_currentHour >= m_dayLength)
					m_currentHour -= m_dayLength;
				
				bool isNext = false;
				if (m_currentHour >= m_curEnvCond->_next->hourBegin)
				{
					if (m_curEnvCond->hourBegin > m_curEnvCond->hourEnd)
					{
						if (m_currentHour <= m_curEnvCond->hourBegin && m_currentHour >= m_curEnvCond->hourEnd)
						{
							isNext = true;
						}
					}
					else
					{
						isNext = true;
					}
				}
				if (isNext)
				{
					if (m_curEnvCond->fptrOnEnd)
						m_curEnvCond->fptrOnEnd(m_curEnvCond);

					m_curEnvCond = m_curEnvCond->_next;
					
					if (m_curEnvCond->fptrOnBegin)
						m_curEnvCond->fptrOnBegin(m_curEnvCond);

					if (CheckEnvironmentalConditions())
					{
						ApplyEnvironmentalConditions();
					}
				}

				EnvironmentalCondition *interpCond_1, *interpCond_2;
				float interpProgress;

				if (m_progress >= 0.5)
				{
					interpCond_1 = m_curEnvCond;
					interpCond_2 = m_curEnvCond->_next;
					interpProgress = m_progress - 0.5;
				}
				else
				{
					interpCond_1 = m_curEnvCond->_last;
					interpCond_2 = m_curEnvCond;
					interpProgress = m_progress + 0.5;
				}

				int i = 0;
				for (i = 0; i < 4; i++)
				{
					m_curAmbient[i] = interpCond_1->ambient[i] * (1 - interpProgress) + interpCond_2->ambient[i] * interpProgress;
					m_curDiffuse[i] = interpCond_1->diffuse[i] * (1 - interpProgress) + interpCond_2->diffuse[i] * interpProgress;
					m_curSunPos[i] = interpCond_1->sunPos[i] * (1 - interpProgress) + interpCond_2->sunPos[i] * interpProgress;
				}

				if (interpCond_1->waterMaterialAmbientDiffuse[0][0][0] >= 0.0f)
				{
					for (i = 0; i < 2; i++)
					{
						for (int j = 0; j < 2; j++)
						{
							for (int k = 0; k < 4; k++)
							{
								Dimension::waterMaterialAmbientDiffuse[i][j][k] = interpCond_1->waterMaterialAmbientDiffuse[i][j][k] * (1 - interpProgress) + interpCond_2->waterMaterialAmbientDiffuse[i][j][k] * interpProgress;
							}
						}
					}
				}

				if (interpCond_1->terrainMaterialAmbientDiffuse[0][0][0] >= 0.0f)
				{
					for (i = 0; i < 2; i++)
					{
						for (int j = 0; j < 2; j++)
						{
							for (int k = 0; k < 4; k++)
							{
								Dimension::terrainMaterialAmbientDiffuse[i][j][k] = interpCond_1->terrainMaterialAmbientDiffuse[i][j][k] * (1 - interpProgress) + interpCond_2->terrainMaterialAmbientDiffuse[i][j][k] * interpProgress;
							}
						}
					}
				}

				if (m_curEnvCond->fogColor[0] > 0.0f)
				{
					for	(i = 0; i < 4; i++)
						m_curFogColor[i] = interpCond_1->fogColor[i] * (1 - interpProgress) + interpCond_2->fogColor[i] * interpProgress;

					m_curFogIntensity += interpCond_1->fogIntensity * (1 - interpProgress) + interpCond_2->fogIntensity * interpProgress;
					m_curFogBegin += interpCond_1->fogBegin * (1 - interpProgress) + interpCond_2->fogBegin * interpProgress;
					m_curFogEnd += interpCond_1->fogEnd * (1 - interpProgress) + interpCond_2->fogEnd * interpProgress;
				}
				
				SetFog(false);
				
				m_skyboxRotation += 120.0 * (lastPass / m_hourLength / (float) m_dayLength);

				while (m_skyboxRotation > 360.0f)
					m_skyboxRotation -= (int) (m_skyboxRotation / 360.0);
			}

			void FourthDimension::ApplyLight(void)
			{
				glLightfv(GL_LIGHT0, GL_AMBIENT,  m_curAmbient);

				// Enable diffuse light for Light 1
				glLightfv(GL_LIGHT0, GL_DIFFUSE,  m_curDiffuse);

				// Set the position of Light 1
				glLightfv(GL_LIGHT0, GL_POSITION, m_curSunPos);
			}

			void FourthDimension::AllocSkyboxContainer(const int length)
			{
				if (m_skyboxes != NULL)
					return;

				m_skyboxesCount = length;
				m_skyboxes = new SkyBox*[length];
				m_skyboxCurIndex = 0;
			}

			void FourthDimension::InitSkyBox(int detail, int hdetail)
			{
				LoadSkyboxes();
			
				float* coords = new float[hdetail * detail * 3];
				float* texcoords = new float[hdetail * detail * 2];

				float u, v, ut, vt;
				int u_p, v_p;
				
				float uIncr = PI / (float)(hdetail - 1);
				float vIncr = 2.0 * PI / (float)(detail - 1);
				float utIncr = 1.0f / (hdetail - 1);
				float vtIncr = 1.0f / (detail - 1);

				float* tmpcoords = coords;
				float* tmptexcoords = texcoords;

				//Sphere rendering
				for(u = 0.0f, u_p = 0, ut = 0.0f; u_p < hdetail; u += uIncr, ut += utIncr, u_p++)
				{
					for(v = 0.0f, v_p = 0, vt = 0.0f; v_p < detail; v += vIncr, vt += vtIncr, v_p++)
					{
						*tmpcoords = sin(u) * cos(v); //x
						tmpcoords++;
						
						*tmpcoords = sin(u) * sin(v); //y
						tmpcoords++;

						*tmpcoords = cos(u); //z
						tmpcoords++;

						*tmptexcoords = vt;
						tmptexcoords++;

						*tmptexcoords = ut;
						tmptexcoords++;
					}
				}

				//Face Creation
				int *faces = new int[(detail - 1) * (hdetail -1)  * 4];
				int *tmpfaces = faces;
				int facecount = 0;
				for(u_p = 0; u_p < hdetail - 1; u_p++)
				{
					//Circular positioning
					for(v_p = 0; v_p < detail - 1; v_p++)
					{
						*tmpfaces = u_p * detail + v_p; //v0
						tmpfaces++;

						*tmpfaces = u_p * detail + (v_p + 1); //v1
						tmpfaces++;

						*tmpfaces = (u_p + 1) * detail + (v_p + 1); //v2;
						tmpfaces++;

						*tmpfaces = (u_p + 1) * detail + v_p; //v3;
						tmpfaces++;

						facecount += 4;
					}
				}
				
				console << "[ENV] Sphere Face Points: " << facecount << Console::nl;
				this->sphere_coords = coords;
				this->sphere_texcoords = texcoords;
				this->sphere_faces = faces;
				this->sphere_face_count = (detail - 1) * (hdetail - 1);
			}
			
			bool FourthDimension::LoadSkyboxes(std::string config_file)
			{
				std::string filename = Utilities::GetConfigFile(config_file);
				std::ifstream     file;
				std::string       buffer;
				
				if (!filename.length())
				{
					return false;
				}

				file.open(filename.c_str());

				if (!file.is_open())
					return false;
				
				int c = -1;
				while (!file.eof())
				{
					buffer.clear();
					
					Utilities::ReadLineFromFile(file, buffer);
					if (c == -1)
					{
						c = atoi(buffer.c_str());
						
						if (c <= 0)
						{
							console << Console::err << "[SKYBOX] Invalid amount!" << Console::nl;
							return false;
						}
						
						AllocSkyboxContainer(c);

						c = 0;
						
						continue;
					}
					
					/*
					if (buffer.length() > 64)
					{
						console << Console::err << "[SKYBOX] Too long file name!" << Console::nl;
						return false;
					}
					*/
					
					if (m_skyboxesGLContainer.find(buffer) != m_skyboxesGLContainer.end())
					{
						console << Console::imp << "[SKYBOX] Warning: multiple definitions of " << buffer << Console::nl;
						continue;
					}
					
					std::ostringstream orig_file;
					orig_file << "textures/";
					orig_file << buffer;
					
					if (!Utilities::FileExists(Utilities::GetDataFile(orig_file.str())))
					{
						console << Console::err << "[SKYBOX] " << orig_file.str() << " is missing!" << Console::nl;
						return false;
					}
					
					int ret = LoadSkyBox(orig_file.str());
					if (ret > -1)
					{
						unsigned pos = buffer.find_last_of('.');
						if (pos != std::string::npos)
							buffer = buffer.substr(0, pos);

						m_skyboxesGLContainer[buffer] = ret;
						
						c++;

						if (c == m_skyboxesCount)
							break;
					}
					else
					{
						return false;
					}
				}
				
				file.close();

				return true;
			}
			
			int FourthDimension::GetSkybox(const char* tag)
			{
				std::map<std::string, int>::const_iterator it = m_skyboxesGLContainer.find(tag);
				if (it == m_skyboxesGLContainer.end())
					return -1;
					
				return (*it).second;
			}

			int FourthDimension::LoadSkyBox(std::string path)
			{
				SkyBox* box = new SkyBox;
				console << "[ENV] Skybox: loading texture '" << path << "..." << Console::nl;
				box->texture = Utilities::LoadTexture(path);
				
				box->r = sqrt(Dimension::terrainOffsetX * Dimension::terrainOffsetX + Dimension::terrainOffsetY * Dimension::terrainOffsetY) * 2;
				console << "[ENV] Skybox: radius: " << box->r << Console::nl;

				m_skyboxes[m_skyboxCurIndex] = box;
				m_skyboxCurIndex++;

				return m_skyboxCurIndex - 1;
			}

			void FourthDimension::PrepareFog(void) const
			{
				glFogi(GL_FOG_MODE, GL_EXP);
				glHint(GL_FOG_HINT, GL_DONT_CARE);
			}

			void FourthDimension::SetFog(bool isEnabled)
			{
				if (isEnabled)
				{
					glFogfv(GL_FOG_COLOR,  m_curFogColor);
					glFogf(GL_FOG_DENSITY, m_curFogIntensity);
					
					glFogf(GL_FOG_START,   m_curFogBegin);
					glFogf(GL_FOG_END,     m_curFogEnd);

					if (!glIsEnabled(GL_FOG))
						glEnable(GL_FOG);
				}
				else
				{
					if (glIsEnabled(GL_FOG))
						glDisable(GL_FOG);
				}
			}
			
			float FourthDimension::CalculateLength(EnvironmentalCondition* cond, float from /* = 0 */, float to /* = 0 */)
			{
				if (cond != NULL)
				{
					EnvironmentalCondition* next = cond->_next;
					if (next == NULL)
						next = m_envFirst;
					
					from = cond->hourBegin;
					to   = next->hourBegin;
				}
				
				float length = from - to;

				if (length < 0)
					length += m_dayLength;
					
				return length;
			}
			
			void FourthDimension::RenderSkyBox(const SkyBox *sky1, const SkyBox *sky2, float value, float rotation)
			{
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);	

				float scale = sky1->r;
				glPushMatrix();
				glScalef(scale, scale, scale);
				glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
				glRotatef(rotation, 0.0f, 0.0f, 1.0f);

				GLfloat mixcol[] = {value, value, value, 1.0};

				//Texture 1 on the Texture Unit 0
				glActiveTextureARB( GL_TEXTURE0_ARB );
				glBindTexture( GL_TEXTURE_2D, sky2->texture);

				//Set up the blending of the 2 textures
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mixcol);
			
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE1_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR);

				//Switch on the Texture Unit 1, switch on texture mapping on this TU
				glActiveTextureARB( GL_TEXTURE1_ARB );
				glEnable( GL_TEXTURE_2D );

				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

				//Texture 0 on TU 1
				glBindTexture( GL_TEXTURE_2D, sky1->texture);

				//Rendering
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				glBegin(GL_QUADS);
					for(int i = 0; i < sphere_face_count; i++)
					{
						for(int b = 0; b < 4; b++)
						{
							glMultiTexCoord2fARB(
								GL_TEXTURE0_ARB, 
								sphere_texcoords[sphere_faces[i * 4 + b] * 2], 
								sphere_texcoords[sphere_faces[i * 4 + b] * 2 + 1]
							);
							glMultiTexCoord2fARB(
								GL_TEXTURE1_ARB, 
								sphere_texcoords[sphere_faces[i * 4 + b] * 2], 
								sphere_texcoords[sphere_faces[i * 4 + b] * 2 + 1]
							);
							glVertex3f(
								sphere_coords[sphere_faces[i * 4 + b] * 3],
								sphere_coords[sphere_faces[i * 4 + b] * 3 + 1],
								sphere_coords[sphere_faces[i * 4 + b] * 3 + 2]
							);
						}
					}
				glEnd();
		
				//Switch off the TU 1
				glDisable( GL_TEXTURE_2D );
				glActiveTextureARB( GL_TEXTURE0_ARB );
			
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

				glPopMatrix();

				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
			}

			void FourthDimension::RenderSkyBox(const SkyBox* sky, float rotation)
			{
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);	
				glEnable(GL_TEXTURE_2D);

				float scale = sky->r;
				glPushMatrix();
				glScalef(scale, scale, scale);
				glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
				glRotatef(rotation, 0.0f, 0.0f, 1.0f);
				glBindTexture(GL_TEXTURE_2D, sky->texture);

				glBegin(GL_QUADS);
					for(int i = 0; i < sphere_face_count; i++)
					{
						for(int b = 0; b < 4; b++)
						{
							glTexCoord2f(
								sphere_texcoords[sphere_faces[i * 4 + b] * 2], 
								sphere_texcoords[sphere_faces[i * 4 + b] * 2 + 1]
							);
							glVertex3f(
								sphere_coords[sphere_faces[i * 4 + b] * 3],
								sphere_coords[sphere_faces[i * 4 + b] * 3 + 1],
								sphere_coords[sphere_faces[i * 4 + b] * 3 + 2]
							);
						}
					}
				glEnd();

				glPopMatrix();

				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
			}

			void FourthDimension::RenderSkyBox()
			{
				if (!m_prepared)
					return;
			
				EnvironmentalCondition *interpCond_1, *interpCond_2;
				float interpProgress;

				if (m_progress >= 0.5)
				{
					interpCond_1 = m_curEnvCond;
					interpCond_2 = m_curEnvCond->_next;
					interpProgress = m_progress - 0.5;
				}
				else
				{
					interpCond_1 = m_curEnvCond->_last;
					interpCond_2 = m_curEnvCond;
					interpProgress = m_progress + 0.5;
				}

				RenderSkyBox(m_skyboxes[interpCond_1->skybox], m_skyboxes[interpCond_2->skybox], interpProgress, m_skyboxRotation);
			}

			void FourthDimension::AddCondition(EnvironmentalCondition* const cond)
			{
				if (m_envFirst == NULL)
				{
					m_envFirst   = cond;
					m_envLast    = cond;
					m_curEnvCond = cond;
					return;
				}
				
				m_envLast->_next = cond;
				cond->_last = m_envLast;

				m_envLast = cond;
				m_envLast->_next = m_envFirst;
				m_envFirst->_last = m_envLast;
			}

			bool FourthDimension::ValidateConditions(void)
			{
				if(m_prepared)
				{
					console << Console::err << "[ENV] Condition list already assembled." << Console::nl;
					return true;
				}

				if (m_envFirst == NULL)
					return false;

				EnvironmentalCondition* cond = m_envFirst;
				float curHour = 0;
				bool go = true;

				while (true)
				{
					EnvironmentalCondition* next = cond->_next;

					if (next == m_envFirst)
					{
						go = false;
					}
					
					curHour = cond->hourEnd;
					if (curHour < next->hourBegin - 0.00001 || curHour > next->hourBegin + 0.00001)
					{
						console << Console::err << "[ENV] Invalid hour " << curHour << ". Expected " << next->hourBegin << Console::nl;
						return false;
					}
					
					m_conditionsCount++;
					
					if (go)
						cond = cond->_next;
					else
						break;
				}
				m_prepared = true;
				return true;
			}
		}
	}
}

