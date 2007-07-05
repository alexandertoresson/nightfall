#include "window.h"

#include "errors.h"
#include "utilities.h"
#include <iostream>
#include <fstream>

namespace Window
{
	SDL_Surface* pWindow;
	bool initialized = false;

	int windowWidth, windowHeight;

	float guiResolution, guiHeight, guiWidth;

	int Init(int flags)
	{
		if (initialized)
			return WINDOW_ERROR_ALREADY_INITIALIZED;
	
		if (SDL_Init(flags) < 0)
			return WINDOW_ERROR_INIT_FAILURE;
			
#ifdef MAC		
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
#else
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif
		
		// Begär double buffering
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		
		initialized = true;
		
		return SUCCESS;
	}
	
	void OnClose(void (*fptrExit)(void))
	{
		atexit(fptrExit);
	}
	
	void Exit(void)
	{
		SDL_Quit();
	}
	
	int OpenDynamic(Utilities::ConfigurationFile* pConfigInterpreter)
	{
		if (initialized == false)
			return WINDOW_ERROR_NOT_INITIALIZED;
		if (pConfigInterpreter == NULL)
			return WINDOW_ERROR_NO_CONFIGURATION_INTERPRETER;

		int   width = 0, height = 0, bpp = 0, result = 0;
		bool  fullscreen = false;
		
		result = pConfigInterpreter->Parse();
		if (result != SUCCESS)
			return result;
		
		width      = atoi(pConfigInterpreter->GetValue("screen width"));
		height     = atoi(pConfigInterpreter->GetValue("screen height"));
		bpp        = atoi(pConfigInterpreter->GetValue("screen bpp"));
		fullscreen = atoi(pConfigInterpreter->GetValue("fullscreen")) == 1;
		
		return OpenStatic(width, height, bpp, fullscreen);
	}

	int OpenStatic(int width, int height, int bpp, bool fullscreen)
	{
		std::cout << "Using screen configuration: " << width << "x" << height << "x" << bpp << std::endl;
		if (initialized == false)
			return WINDOW_ERROR_NOT_INITIALIZED;

		//  this holds some info about our display 
		const SDL_VideoInfo *videoInfo;
		
		//  Fetch the video info 
		videoInfo = SDL_GetVideoInfo( );
	
		if ( !videoInfo )
		{
			std::cerr << "[FAILED] Video query failed: " <<	 SDL_GetError( ) << std::endl;
			return WINDOW_ERROR_INIT_FAILURE;
		}
	
		int flags = SDL_OPENGL; 	  //  Enable OpenGL in SDL 
		flags |= SDL_GL_DOUBLEBUFFER; //  Enable double buffering 
		flags |= SDL_HWPALETTE;       //  Store the palette in hardware 
	
		//  This checks to see if surfaces can be stored in memory 
		if ( videoInfo->hw_available )
		{
			flags |= SDL_HWSURFACE;
			std::cout<< "Using hardware surface: Yes" << std::endl;
		}
		else
		{
			flags |= SDL_SWSURFACE;
			std::cout<< "Using hardware surface: No" << std::endl;
		}
	
		//  This checks if hardware blits can be done 
		if ( videoInfo->blit_hw )
		{
			flags |= SDL_HWACCEL;
			std::cout<< "Using hardware blits: Yes" << std::endl;
		}
		else
		{
			std::cout<< "Using hardware blits: No" << std::endl;
		}
		
		if (fullscreen == true)
		{
			flags |= SDL_FULLSCREEN;
			std::cout<< "Fullscreen: Yes" << std::endl;
		}
		else
		{
			std::cout<< "Fullscreen: No" << std::endl;
		}

		pWindow = SDL_SetVideoMode(width, height, bpp, flags);
		
		if (!pWindow)
		{
			std::cout << "Init Failed: " << SDL_GetError( ) << std::endl;
			return ERROR_GENERAL;
		}
		
		// Skriv ut resultatet:
		SDL_GLattr attributes[] = { 
			SDL_GL_RED_SIZE,
			SDL_GL_BLUE_SIZE,
			SDL_GL_GREEN_SIZE,
			SDL_GL_ALPHA_SIZE,
			SDL_GL_BUFFER_SIZE,
			SDL_GL_DEPTH_SIZE
		};
		
		char* descriptions[] = {
			"Red Comp size:  . . . %d\n",
			"Blue Comp size: . . . %d\n",
			"Green Comp size:  . . %d\n",
			"Alpha Comp size:  . . %d\n",
			"Color buffer size:. . %d\n",
			"Depth buffer size:. . %d\n"
		};
		
		std::cout << "Sizes -------------- Bits\n";
		
		int value;
		for (unsigned int i = 0; i < sizeof(attributes) / sizeof(SDL_GLattr); i++)
		{
			if (SDL_GL_GetAttribute(attributes[i], &value) < 0)
				continue;
			printf(descriptions[i], value);
		}

		//  Height / width ration 
		GLfloat ratio;
	 
		//  Protect against a divide by zero 
		if ( height == 0 )
			height = 1;
		
		ratio = ( GLfloat )width / ( GLfloat )height;
		
		//  change to the projection matrix and set our viewing volume. 
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
			
		//  Setup our viewport. 
		glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );
		
		//  Set our perspective 
		gluPerspective( 45.0f, ratio, 0.1f, 200.0f );		
				
		//  Make sure we're chaning the model view and not the projection 
		glMatrixMode( GL_MODELVIEW );
		
		//  Reset The View 
		glLoadIdentity();

		windowWidth  = width;
		windowHeight = height;

		guiResolution = 1.0f / (float)height;
		guiWidth = (float)width / (float)height;
		guiHeight = 1.0f;

#ifndef WIN32
		if (!Utilities::IsOGLExtensionSupported("GL_ARB_multitexture"))
		{
			std::cout << "The OpenGL extension GL_ARB_multitexture is needed for this program" << std::endl;
			return ERROR_GENERAL;
		}

		if (!Utilities::IsOGLExtensionSupported("GL_ARB_texture_env_combine"))
		{
			std::cout << "The OpenGL extension GL_ARB_texture_env_combine is needed for this program" << std::endl;
			return ERROR_GENERAL;
		}
		
		
		if (!Utilities::IsOGLExtensionSupported("GL_ARB_texture_env_crossbar"))
		{
			std::cout << "The OpenGL extension GL_ARB_texture_env_crossbar is needed for this program" << std::endl;
			return ERROR_GENERAL;
		}
		
#else
		{
			GLenum err = glewInit();

			if (GLEW_OK != err)
			{
				std::ostringstream iss;
				iss << "Critical error: " << glewGetErrorString(err) << "\n\nTerminating application!";
				MessageBoxA(NULL, iss.str().c_str(), "GLEW error!", MB_OK | MB_ICONERROR);
				return ERROR_GENERAL;
			}
		}

		if (!glewIsExtensionSupported("GL_ARB_multitexture"))
		{
			MessageBoxA(NULL, "The OpenGL extension GL_ARB_multitexture doesn't seem to be supported by your hardware.\nPlease make sure you have the latest drivers installed.", "OpenGL Extension error!", MB_OK | MB_ICONERROR);
			return ERROR_GENERAL;

		}

		if (!glewIsExtensionSupported("GL_ARB_texture_env_combine"))
		{
			MessageBoxA(NULL, "The OpenGL extension GL_ARB_texture_env_combine doesn't seem to be supported by your hardware.\nPlease make sure you have the latest drivers installed.", "OpenGL Extension error!", MB_OK | MB_ICONERROR);
			return ERROR_GENERAL;
		}
	/*
		if (!glewIsExtensionSupported("GL_ARB_texture_env_crossbar"))
		{
			MessageBoxA(NULL, "The OpenGL extension GL_ARB_texture_env_crossbar doesn't seem to be supported by your hardware.\nPlease make sure you have the latest drivers installed.", "OpenGL Extension error!", MB_OK | MB_ICONERROR);
			return ERROR_GENERAL;
		}
	*/	
#endif
		GLint maxTextureUnits;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
		GLint maxModelviewStackDepth;
		glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &maxModelviewStackDepth);
		GLint maxProjectionViewStackDepth;
		glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &maxProjectionViewStackDepth);
		cout << "Maximum number of texture units: " << maxTextureUnits << endl;
		cout << "Maximum projection matrix stack depth: " << maxProjectionViewStackDepth << endl;
		cout << "Maximum modelview matrix stack depth: " << maxModelviewStackDepth << endl;
		
		return SUCCESS;
	}
	
	void SetTitle(const char* title)
	{
		if (initialized == false)
			return;
	
		SDL_WM_SetCaption(title, NULL);
	}

	void  MakeScreenshot(void)
	{
		static int screenshotCount = 1;

		if (pWindow->format->BitsPerPixel != 32)
		{
			std::cout << "Screenshot failed - unsupported bits per pixel." << std::endl;
			return;
		}

		char filename[256];
		while (true)
		{
			sprintf(filename, "./screenshots/SCR%d.bmp", screenshotCount);

			ifstream file(filename);

			if (file.good() == 0)
			{
				file.close();
				break;
			}

			screenshotCount++;
			file.close();
		}

		SDL_Surface *image;
		SDL_Surface *temp;
		int idx;

		image = SDL_CreateRGBSurface(SDL_SWSURFACE, pWindow->w, pWindow->h, pWindow->format->BitsPerPixel, 0x000000FF, 0x0000FF00,0x00FF0000, 0xFF000000);
		temp = SDL_CreateRGBSurface(SDL_SWSURFACE, pWindow->w, pWindow->h, pWindow->format->BitsPerPixel, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

		glReadPixels(0, 0, pWindow->w, pWindow->h, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);

		// Following equation copies each pixel into the temp.
		for (idx = 0; idx < pWindow->h; idx++) 
		{
			memcpy((char *)temp->pixels + 4 * pWindow->w * idx,
				   (char *)image->pixels + 4 * pWindow->w*(pWindow->h-1 - idx),
				   4*pWindow->w);
		}

		int result = SDL_SaveBMP(temp, filename);

		SDL_FreeSurface(image);
		SDL_FreeSurface(temp);
		if (result == 0)
			std::cout << "Screenshot " << filename << " written." << std::endl;
	}
}
