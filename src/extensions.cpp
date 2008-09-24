/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */

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
			MessageBoxA(NULL, str.c_str(), "OpenGL Extension error!", MB_OK | MB_ICONERROR);
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

