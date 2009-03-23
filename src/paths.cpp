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
#include "vfs-pre.h"
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
			// when not being in the src directory.
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

		// TODO: Needs some refactoring?
		if (xdg_data_dirs)
		{

			xdg_data_dirs_split.push_back("");

			for (unsigned i = 0; i < strlen(xdg_data_dirs); i++)
			{
				if (xdg_data_dirs[i] == ':')
				{
					xdg_data_dirs_split.push_back("");
				}
				else
				{
					xdg_data_dirs_split.back().push_back(xdg_data_dirs[i]);
				}
			}

			for (unsigned i = 0; i < xdg_data_dirs_split.size(); i++)
			{
				xdg_data_dirs_split[i] += "/nightfall/";
			}
		}
		
		const char* xdg_config_dirs = getenv("XDG_CONFIG_DIRS");

		if (xdg_config_dirs)
		{

			xdg_config_dirs_split.push_back("");

			for (unsigned i = 0; i < strlen(xdg_config_dirs); i++)
			{
				if (xdg_config_dirs[i] == ':')
				{
					xdg_config_dirs_split.push_back("");
				}
				else
				{
					xdg_config_dirs_split.back().push_back(xdg_config_dirs[i]);
				}
			}

			for (unsigned i = 0; i < xdg_config_dirs_split.size(); i++)
			{
				xdg_config_dirs_split[i] += "/nightfall/";
			}
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
		VFS::Mount(home + "/.config/nightfall/", "/config/");
		for (std::vector<std::string>::reverse_iterator it = xdg_config_dirs_split.rbegin(); it != xdg_config_dirs_split.rend(); it++)
		{
			VFS::Mount(*it, "/config/");
		}

		VFS::Mount(path_from_argv0 + "resources/", "/data/");
		VFS::Mount("resources/", "/data/");
		VFS::Mount(home + "/.config/nightfall/", "/data/");
		for (std::vector<std::string>::reverse_iterator it = xdg_data_dirs_split.rbegin(); it != xdg_data_dirs_split.rend(); it++)
		{
			VFS::Mount(*it, "/data/");
		}
#else
		VFS::Mount(path_from_argv0 + "resources/configuration/", "/config/");
		VFS::Mount("resources/configuration/", "/config/");

		VFS::Mount(path_from_argv0 + "resources/", "/data/");
		VFS::Mount("resources/", "/data/");
#endif
	}

}

