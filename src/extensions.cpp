
#include "extensions.h"
#include <string>
#include <iostream>

namespace Utilities
{
	static bool ExtensionSupported(std::string extension)
	{
		if (!glewIsExtensionSupported(extension.c_str()))
		{
#ifdef WIN32
			std::string str = "The OpenGL extension " + extension + " doesn't seem to be supported by your hardware.\nPlease make sure you have the latest drivers installed.";
			MessageBoxA(NULL, str, "OpenGL Extension error!", MB_OK | MB_ICONERROR);
#else
			std::cout << extension << " extension could not be found!" << std::endl;
#endif
			return false;
		}
		return true;
	}

	bool CheckExtensions()
	{
		if (!ExtensionSupported("GL_ARB_fragment_shader") ||
		    !ExtensionSupported("GL_ARB_vertex_shader") ||
		    !ExtensionSupported("GL_ARB_shader_objects") ||
		    !ExtensionSupported("GL_ARB_shading_language_100") ||
		    !ExtensionSupported("GL_ARB_vertex_buffer_object") ||
		    !ExtensionSupported("GL_EXT_framebuffer_object"))
		{
			return false;
		}
		return true;
	}
}

