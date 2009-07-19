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
#include "vfs.h"

#include "filesystem.h"
#include "utilities.h"
#include "archive.h"

#include <deque>
#include <vector>

namespace Utilities
{
	namespace VFS
	{
		typedef std::pair<std::string, std::string> MountType;
		typedef std::vector<MountType> MountVector;
		std::deque<MountVector> stateStack;
		MountVector mounts;

		enum ResolveType
		{
			RESOLVE_READABLE,
			RESOLVE_WRITABLE
		};

		std::string CheckAndResolvePath(const std::string& filename, ResolveType rType)
		{
			if (rType == RESOLVE_WRITABLE)
			{
				return Utilities::FileIsWritable(filename) ? filename : "";
			}
			else
			{
				std::string zipPath;
				std::string path;
				bool first = true;
				if (filename.find(".zip") != std::string::npos)
				{
					std::vector<std::string> elems;
					split(filename, '/', elems);

					for (std::vector<std::string>::iterator it = elems.begin(); it != elems.end(); ++it)
					{
						if (!first)
						{
							path.push_back('/');
						}
						first = false;
						path += *it;

						if (it->size() >= 4 && it->substr(it->size()-4) == ".zip")
						{
							if (zipPath.length())
							{
								try
								{
									ArchiveReader ar(zipPath);
									zipPath = ar.ExtractFile(path);
								}
								catch (FileNotFoundException e)
								{
									return "";
								}
							}
							zipPath = path;
							path = "";
							first = true;
						}
					}
				}
				if (zipPath.length())
				{
					try
					{
						ArchiveReader ar(zipPath);
						return ar.ExtractFile(path);
					}
					catch (FileNotFoundException e)
					{
						return "";
					}
				}
				else
				{
					return Utilities::FileIsReadable(filename) ? filename : "";
				}
			}
		}

		std::string Resolve(const std::string& filename, ResolveType rType = RESOLVE_READABLE, VFSLevel level = -1, int i = -1)
		{
			MountVector &lmounts = level == -1 ? mounts : stateStack[level];
			if (i == -1)
				i = lmounts.size()-1;
			
			MountType& cur = lmounts[i];
			unsigned len = cur.first.length();
			std::string newfn = filename;
			bool modified = false;

			// Is it a directory mount or a file mount?
			if (cur.first.find_last_of("\\/") == cur.first.size()-1)
			{
				if (len <= filename.length() && cur.first == filename.substr(0, len))
				{
					newfn = cur.second + filename.substr(len);
					modified = true;
				}
			}
			else
			{
				if (cur.first == filename)
				{
					newfn = cur.second;
					modified = true;
				}
			}
			if (i == 0)
			{
				return CheckAndResolvePath(newfn, rType);
			}
			else
			{
				newfn = Resolve(newfn, rType, level, i-1);
				if (newfn.length())
				{
					return newfn;
				}
				else
				{
					return modified ? Resolve(filename, rType, level, i-1) : "";
				}
			}
		}

		std::string ResolveReadable(const std::string& filename, VFSLevel level)
		{
			return Resolve(filename, RESOLVE_READABLE, level);
		}

		std::string ResolveWritable(const std::string& filename, VFSLevel level)
		{
			return Resolve(filename, RESOLVE_WRITABLE, level);
		}

		bool FileIsReadable(const std::string& filename, VFSLevel level)
		{
			return Resolve(filename, RESOLVE_READABLE, level).length();
		}

		bool FileIsWritable(const std::string& filename, VFSLevel level)
		{
			return Resolve(filename, RESOLVE_WRITABLE, level).length();
		}

		VFSLevel PushState()
		{
			stateStack.push_back(mounts);
			return stateStack.size()-1;
		}
		
		void PopState()
		{
			if (stateStack.empty())
			{
				std::cout << "[VFS] Error: attempted to pop with an empty stack" << std::endl;
				return;
			}
			mounts = stateStack.back();
			stateStack.pop_back();
		}

		void Mount(const std::string& path, const std::string& mountPoint)
		{
			mounts.push_back(std::make_pair(mountPoint, path));
		}
	}
}
