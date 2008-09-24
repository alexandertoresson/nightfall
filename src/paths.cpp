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
#include <vector>

namespace Utilities
{
#if !defined(WIN32) && !defined(MAC)
	std::vector<std::string> paths_split;
#endif
	
	std::string path_from_argv0;

	void InitPaths(std::string programpath)
	{
		path_from_argv0 = GetDirectoryInPath(programpath);
		if (path_from_argv0.size() >= 4)
		{
			std::string last_4_chars = path_from_argv0.substr(path_from_argv0.size()-4);
			if (last_4_chars == "src/" || last_4_chars == "src\\")
			{
				path_from_argv0 = path_from_argv0.substr(0, path_from_argv0.size()-4);
			}
		}
		if (path_from_argv0 == "" || path_from_argv0 == "./" || path_from_argv0 == ".\\")
		{
			path_from_argv0 = "../";
		}

#if defined(__unix__)
		const char* data_paths = getenv("XDG_DATA_DIRS");

		if (data_paths)
		{

			paths_split.push_back("");

			for (unsigned i = 0; i < strlen(data_paths); i++)
			{
				if (data_paths[i] == ':')
				{
					paths_split.push_back("");
				}
				else
				{
					paths_split.back().push_back(data_paths[i]);
				}
			}

			for (unsigned i = 0; i < paths_split.size(); i++)
			{
				paths_split[i] += "/nightfall/";
			}
		}
#endif
	}

	std::string GetFile(PathType type, std::string file)
	{
		std::string path;
		int path_index = 0;
		while ((path = GetPath(type, path_index++)).length())
		{
			if (FileExists(path + file))
			{
				return path + file;
			}
		}
		return "";
	}

	std::string GetConfigFile(std::string file)
	{
		return GetFile(PATHTYPE_CONFIG, file);
	}

	std::string GetDataFile(std::string file)
	{
		return GetFile(PATHTYPE_DATA, file);
	}

	std::string GetWritableFile(PathType type, std::string file, bool &exists)
	{
		std::string path;
		int path_index = 0;
		while ((path = GetPath(type, path_index++)).length())
		{
			exists = false;
			if (FileExists(path + file))
			{
				exists = true;
			}
			if (exists || CanOpenForWriting(path + file))
			{
				return (path + file).c_str();
			}
			else
			{
				CreateDirectory(GetDirectoryInPath(path + file));
				if (CanOpenForWriting(path + file))
				{
					return (path + file).c_str();
				}
			}
		}
		return "";
	}

	std::string GetWritableConfigFile(std::string file, bool &exists)
	{
		return GetWritableFile(PATHTYPE_CONFIG, file, exists);
	}

	std::string GetWritableDataFile(std::string file, bool &exists)
	{
		return GetWritableFile(PATHTYPE_DATA, file, exists);
	}

	std::string GetPath(PathType type, unsigned num)
	{
	#ifdef WIN32
		switch (type)
		{
			case PATHTYPE_CONFIG:
				switch (num)
				{
					case 0:
						return "resources\\configuration\\";
					case 1:
						return path_from_argv0 + "resources\\configuration\\";
					default:
						return "";
				}
			case PATHTYPE_DATA:
				switch (num)
				{
					case 0:
						return "resources\\";
					case 1:
						return path_from_argv0 + "resources\\";
					default:
						return "";
				}
		}
	#elif defined(MAC)
		switch (type)
		{
			case PATHTYPE_CONFIG:
				switch (num)
				{
					case 0:
						return "resources/configuration/";
					case 1:
						return path_from_argv0 + "resources/configuration/";
					default:
						return "";
				}
			case PATHTYPE_DATA:
				switch (num)
				{
					case 0:
						return "resources/";
					case 1:
						return path_from_argv0 + "resources/";
					default:
						return "";
				}
		}
	#elif defined(__unix__)
		switch (type)
		{
			case PATHTYPE_CONFIG:
				switch (num)
				{
					case 0:
						return (std::string) getenv("HOME") + "/.config/nightfall/";
					case 1:
						return "resources/configuration/";
					case 2:
						return path_from_argv0 + "resources/configuration/";
					case 3:
						return "/etc/nightfall/";
					default:
						return "";
				}
			case PATHTYPE_DATA:
				switch (num)
				{
					case 0:
						return (std::string) getenv("HOME") + "/.nightfall/";
					case 1:
						return "resources/";
					case 2:
						return path_from_argv0 + "resources/";
					default:

						if (num - 3 < paths_split.size())
						{
							return paths_split[num-3];
						}

						return "";
				}
			default:
				return "";
		}
	#else
		switch (type)
		{
			case PATHTYPE_CONFIG:
				switch (num)
				{
					case 0:
						return "resources/configuration/";
					case 1:
						return path_from_argv0 + "resources/configuration/";
					default:
						return "";
				}
			case PATHTYPE_DATA:
				switch (num)
				{
					case 0:
						return "resources/";
					case 1:
						return path_from_argv0 + "resources/";
					default:
						return "";
				}
		}
	#endif
	}
}

