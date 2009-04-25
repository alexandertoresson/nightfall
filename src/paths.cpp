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
#include "paths.h"
#include "filesystem.h"
#include "vfs.h"
#include "utilities.h"
#include <vector>

namespace Utilities
{
	
	void InitPaths(std::string programpath)
	{
		std::string path_from_argv0 = GetDirectoryInPath(programpath);

		if (path_from_argv0.size() >= 4)
		{
			std::string last_4_chars = path_from_argv0.substr(path_from_argv0.size()-4);

			// Handle special case of running the binary when it is in the src directory
			// when not being in the src directory. Just chop of src[/\].
			//
			if (last_4_chars == "src/" || last_4_chars == "src\\")
			{
				path_from_argv0 = path_from_argv0.substr(0, path_from_argv0.size()-4);
			}
		}

		// Allow executing the program from the src directory (./nightfall) and still have it find all its files;
		// ./ will still be checked by other means, so check in ../ too.
		if (path_from_argv0 == "" || path_from_argv0 == "./" || path_from_argv0 == ".\\")
		{
			path_from_argv0 = "../";
		}

#if defined(__unix__)
		std::vector<std::string> xdg_data_dirs_split;
		std::vector<std::string> xdg_config_dirs_split;
		const char* xdg_data_dirs = getenv("XDG_DATA_DIRS");
		const char* xdg_data_home = getenv("XDG_DATA_HOME");

		if (xdg_data_dirs)
		{
			Utilities::split(std::string(xdg_data_dirs), ':', xdg_data_dirs_split);
		}
		
		const char* xdg_config_dirs = getenv("XDG_CONFIG_DIRS");
		const char* xdg_config_home = getenv("XDG_CONFIG_HOME");

		if (xdg_config_dirs)
		{
			Utilities::split(std::string(xdg_config_dirs), ':', xdg_config_dirs_split);
		}
#endif
		std::string home = getenv("HOME");
#ifdef WIN32
		VFS::Mount(path_from_argv0 + "resources\\configuration\\", "/config/");
		VFS::Mount("resources\\configuration\\", "/config/");

		VFS::Mount(path_from_argv0 + "resources\\", "/data/");
		VFS::Mount("resources\\", "/data/");
#elif defined(MAC)
		VFS::Mount(path_from_argv0 + "resources/configuration/", "/config/");
		VFS::Mount("resources/configuration/", "/config/");
		VFS::Mount(home + "/Library/Application Support/Nightfall/configuration/", "/config/");

		VFS::Mount(path_from_argv0 + "resources/", "/data/");
		VFS::Mount("resources/", "/data/");
		VFS::Mount(home + "/Library/Application Support/Nightfall/", "/data/");
#elif defined(__unix__)
		VFS::Mount("/etc/nightfall/", "/config/");
		VFS::Mount(path_from_argv0 + "resources/configuration/", "/config/");
		VFS::Mount("resources/configuration/", "/config/");
		for (std::vector<std::string>::reverse_iterator it = xdg_config_dirs_split.rbegin(); it != xdg_config_dirs_split.rend(); it++)
		{
			VFS::Mount(*it, "/config/");
		}
		VFS::Mount((xdg_config_home ? std::string(xdg_config_home) : home + "/.config") +  "/nightfall/", "/config/");

		VFS::Mount(path_from_argv0 + "resources/", "/data/");
		VFS::Mount("resources/", "/data/");
		for (std::vector<std::string>::reverse_iterator it = xdg_data_dirs_split.rbegin(); it != xdg_data_dirs_split.rend(); it++)
		{
			VFS::Mount(*it, "/data/");
		}
		VFS::Mount((xdg_data_home ? std::string(xdg_data_home) : home + "/.local/share") +  "/nightfall/", "/data/");
#else
		VFS::Mount(path_from_argv0 + "resources/configuration/", "/config/");
		VFS::Mount("resources/configuration/", "/config/");

		VFS::Mount(path_from_argv0 + "resources/", "/data/");
		VFS::Mount("resources/", "/data/");
#endif
	}

}

