
#ifndef __MATERIALXML_H_PRE__
#define __MATERIALXML_H_PRE__

#ifdef DEBUG_DEP
#warning "materialxml.h-pre"
#endif

#include "sdlheader.h"
#include <string>
#include <vector>

namespace Utilities
{
	struct Colour
	{
		GLfloat val[4];

		Colour()
		{
			val[0] = 0.0f;
			val[1] = 0.0f;
			val[2] = 0.0f;
			val[3] = 0.0f;
		}
		
		Colour(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0)
		{
			val[0] = r;
			val[1] = g;
			val[2] = b;
			val[3] = a;
		}
	};

	struct Uniform
	{
		std::string name;
		virtual void Set(GLint id) = 0;
		virtual ~Uniform() {};
	};

	struct Uniform4f : public Uniform
	{
		GLfloat val[4];

		Uniform4f(std::string name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
		{
			this->name = name;
			val[0] = v1;
			val[1] = v2;
			val[2] = v3;
			val[3] = v4;
		}
		
		Uniform4f(std::string name, Colour colour)
		{
			this->name = name;
			val[0] = colour.val[0];
			val[1] = colour.val[1];
			val[2] = colour.val[2];
			val[3] = colour.val[3];
		}

		void Set(GLint id)
		{
			glUniform4fARB(id, val[0], val[1], val[2], val[3]);
		}
		
		virtual ~Uniform4f() {};
	};

	struct UniformInt : public Uniform
	{
		GLint val;

		UniformInt(std::string name, GLint val) : val(val)
		{
			this->name = name;
		}
		
		void Set(GLint id)
		{
			glUniform1iARB(id, val);
		}
		
		virtual ~UniformInt() {};
	};

	struct UniformFloat : public Uniform
	{
		GLfloat val;

		UniformFloat(std::string name, GLfloat val) : val(val)
		{
			this->name = name;
		}
		
		void Set(GLint id)
		{
			glUniform1fARB(id, val);
		}
		
		virtual ~UniformFloat() {};
	};

	struct TextureImageData
	{
		std::string name;
		unsigned dimensions;
		unsigned width, height, channels;
		GLenum pixelFormat, channelFormat;
		SDL_Surface *image;
		GLuint buffer;

		TextureImageData(std::string filename, std::string name);

		void Lock();
		void Unlock();
	};

	struct Material
	{
		std::string filename;
		Colour ambient;
		Colour diffuse;
		Colour specular;
		Colour emission;
		GLfloat shininess;
		std::vector<TextureImageData*> textures;
		std::vector<Uniform*> uniforms;
		GLhandleARB program;
		GLhandleARB vertShader;
		GLhandleARB fragShader;

		Material() : shininess(0.0f), program(0), vertShader(0), fragShader(0)
		{
			
		}

	};
}

#endif
