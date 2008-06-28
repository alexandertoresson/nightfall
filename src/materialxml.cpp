#include "materialxml.h"
#include "minixml.h"
#include "paths.h"
#include "console.h"
#include "configuration.h"
#include <map>
#include <sstream>
#include <iostream>

namespace Utilities
{
	TextureImageData::TextureImageData(std::string filename, std::string name) : name(name), image(NULL), buffer(0)
	{
		std::string path = GetDataFile(filename);
		
		if (!path.length())
		{
			std::cout << "Failed to load texture '" << filename << "' -- Not found!" << std::endl;
			return;
		}

		image = IMG_Load(path.c_str());

		if(!image) 
		{
			std::cout << "Failed to load texture '" << path << "' SDL Reports: " << IMG_GetError() << std::endl;
			return;
		}

		width = image->w;
		height = image->h;
		dimensions = 2;
		channelFormat = GL_UNSIGNED_BYTE;
		channels = image->format->BytesPerPixel;
		switch (image->format->BytesPerPixel)
		{
			case 3:
				pixelFormat = GL_RGB;
				break;
			case 4:
				pixelFormat = GL_RGBA;
				break;
			default:
				std::cout << path << ": Unknown pixel format" << std::endl;
				break;
		}
	}

	void TextureImageData::Lock()
	{
		if (image && !buffer)
		{
			console << "Material Texture: W: " << image->w << " H: " << image->h << Console::nl;

			glGenTextures(1, &buffer);
			glBindTexture(GL_TEXTURE_2D, buffer);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// use bilinear scaling for scaling down and trilinear scaling for scaling up
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate mipmap using GLU
			gluBuild2DMipmaps(GL_TEXTURE_2D, channels, width, height, pixelFormat, channelFormat, image->pixels);
			
		}
	}

	void TextureImageData::Unlock()
	{
	}

	static Material* material = NULL;
	static Colour* colour = NULL;
	static std::map<std::string, Material*> filenameToMaterial;
	static std::string vertFilename, fragFilename;

	void ParseColour(Utilities::XMLElement *elem)
	{
		std::stringstream rss(elem->GetAttribute("r", "0.0")),
		                  gss(elem->GetAttribute("g", "0.0")),
		                  bss(elem->GetAttribute("b", "0.0")),
		                  ass(elem->GetAttribute("a", "1.0"));
		rss >> colour->val[0]; gss >> colour->val[1]; bss >> colour->val[2]; ass >> colour->val[3];
	}

	void ParseShininess(Utilities::XMLElement *elem)
	{
		std::stringstream vss(elem->GetAttribute("value", "0"));
		vss >> material->shininess;
	}

	void ParseTexture(Utilities::XMLElement *elem)
	{
		material->textures.push_back(new TextureImageData(elem->GetAttribute("filename"), elem->GetAttribute("name")));
	}

	void ParseTextures(Utilities::XMLElement *elem)
	{
		elem->Iterate("texture", ParseTexture);
	}

	void ParseUniform4f(Utilities::XMLElement *elem)
	{
		std::stringstream v1_ss(elem->GetAttribute("v1", "0")),
		                  v2_ss(elem->GetAttribute("v2", "0")),
		                  v3_ss(elem->GetAttribute("v3", "0")),
		                  v4_ss(elem->GetAttribute("v4", "0"));
		GLfloat v1, v2, v3, v4;
		v1_ss >> v1; v2_ss >> v2; v3_ss >> v3; v4_ss >> v4;
		material->uniforms.push_back(new Uniform4f(elem->GetAttribute("name"), v1, v2, v3, v4));
	}

	void ParseUniformInt(Utilities::XMLElement *elem)
	{
		std::stringstream value_ss(elem->GetAttribute("value", "0"));
		GLint value;
		value_ss >> value;
		material->uniforms.push_back(new UniformInt(elem->GetAttribute("name"), value));
	}

	void ParseUniformFloat(Utilities::XMLElement *elem)
	{
		std::stringstream value_ss(elem->GetAttribute("value", "0"));
		GLfloat value;
		value_ss >> value;
		material->uniforms.push_back(new UniformFloat(elem->GetAttribute("name"), value));
	}

	void ParseUniforms(Utilities::XMLElement *elem)
	{
		elem->Iterate("uniform4f", ParseUniform4f);
		elem->Iterate("uniformInt", ParseUniformInt);
		elem->Iterate("uniformFloat", ParseUniformFloat);
	}

	static char* ReadFile(std::string filename)
	{
		std::ifstream file(filename.c_str());

		if (!file.good())
		{
			return NULL;
		}

		file.seekg(0, std::ios::end);
		long size = file.tellg();
		file.seekg(0, std::ios::beg);

		char* buffer = new char[size+1];
		memset(buffer, 0, size+1);
		
		file.read(buffer, size);

		return buffer;
	}

	static bool GetShaderCompileError(GLhandleARB handle, GLenum boolean, std::string filename)
	{
		GLint succeeded = 0;
		glGetObjectParameterivARB(handle, boolean, &succeeded);
		if (!succeeded)
		{
			char errorbuffer[1000];
			GLsizei errorbufferlen = 0;
			glGetInfoLogARB(handle, 999, &errorbufferlen, errorbuffer);
			errorbuffer[errorbufferlen] = 0;
#ifdef WIN32
			MessageBoxA( NULL, errorbuffer, ("Error compiling " + filename + "!").c_str(), MB_OK | MB_ICONERROR);
#endif
			std::cout << "Error compiling " << filename << ":" << std::endl;
			std::cout << errorbuffer << std::endl;
			return true;
		}
		return false;
	}

	void ParseShader(Utilities::XMLElement *elem)
	{
		std::string vertFilename = elem->GetAttribute("vertex_shader");
		char* vdata = ReadFile(GetDataFile(vertFilename));
		if (vdata)
		{
			const GLcharARB** d = (const GLcharARB**) &vdata;
			material->vertShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			glShaderSourceARB(material->vertShader, 1, d, NULL);
			glCompileShaderARB(material->vertShader);
			delete[] vdata;

			if (GetShaderCompileError(material->vertShader, GL_OBJECT_COMPILE_STATUS_ARB, vertFilename))
			{
				glDeleteObjectARB(material->vertShader);
				material->vertShader = 0;
			}
		}
		else
		{
			std::cout << "Could not open vertex shader \"" << vertFilename << "\"" << std::endl;
		}

		std::string fragFilename = elem->GetAttribute("fragment_shader");
		char* fdata = ReadFile(GetDataFile(fragFilename));
		if (fdata)
		{
			const GLcharARB** d = (const GLcharARB**) &fdata;
			material->fragShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			glShaderSourceARB(material->fragShader, 1, d, NULL);
			glCompileShaderARB(material->fragShader);
			delete[] fdata;

			if (GetShaderCompileError(material->fragShader, GL_OBJECT_COMPILE_STATUS_ARB, fragFilename))
			{
				glDeleteObjectARB(material->fragShader);
				material->fragShader = 0;
			}
		}
		else
		{
			std::cout << "Could not open fragment shader \"" << fragFilename << "\"" << std::endl;
		}

		if (material->vertShader && material->fragShader)
		{
			material->program = glCreateProgramObjectARB();
			glAttachObjectARB(material->program, material->vertShader);
			glAttachObjectARB(material->program, material->fragShader);
			glLinkProgramARB(material->program);

			if (GetShaderCompileError(material->program, GL_OBJECT_LINK_STATUS_ARB, fragFilename + "+" + vertFilename))
			{
				glDeleteObjectARB(material->program);
				material->program = 0;
				glDeleteObjectARB(material->fragShader);
				material->fragShader = 0;
				glDeleteObjectARB(material->vertShader);
				material->vertShader = 0;
			}
		}
		else
		{
			std::cout << "Didn't manage to load both a fragment and a vertex shader; can't link the shader program." << std::endl;
			if (material->vertShader)
			{
				glDeleteObjectARB(material->vertShader);
			}
			if (material->fragShader)
			{
				glDeleteObjectARB(material->fragShader);
			}
		}
	}

	void ParseInheritMaterial(Utilities::XMLElement *elem)
	{
		std::string filename = elem->GetAttribute("filename");
		Material* inherited = LoadMaterialXML(filename);
		*material = *inherited;
	}

	void ParseMaterial(Utilities::XMLElement *elem)
	{
		if (!material->program)
		{
			elem->Iterate("inherit_material", ParseInheritMaterial);

			elem->Iterate("shader", ParseShader);

			if (material->program || elem->Count("shader") == 0)
			{

				colour = &material->ambient;
				elem->Iterate("ambient", ParseColour);

				colour = &material->diffuse;
				elem->Iterate("diffuse", ParseColour);

				colour = &material->specular;
				elem->Iterate("specular", ParseColour);

				colour = &material->emission;
				elem->Iterate("emission", ParseColour);

				elem->Iterate("shininess", ParseShininess);

				elem->Iterate("textures", ParseTextures);

				elem->Iterate("uniforms", ParseUniforms);
			}
		}
	}

	void ParseMaterials(Utilities::XMLElement *elem)
	{
		TagFuncMap tfmap;
		tfmap["material"] = ParseMaterial;
		elem->Iterate(tfmap, NULL);
	}

	Material* LoadMaterialXML(std::string name)
	{
		XMLReader xmlReader;
		std::string filename = Utilities::GetDataFile(name);

		material = filenameToMaterial[name];

		if (!material)
		{
			if (filename.length() && xmlReader.Read(filename))
			{
				material = new Material;
				material->filename = filename;
				xmlReader.root->Iterate("materials", ParseMaterials);
			}
			else
			{
				std::cout << "Warning: Material " << name << " not found!" << std::endl;
			}
		}

		filenameToMaterial[name] = material;

		xmlReader.Deallocate();

		return material;
	}
}
