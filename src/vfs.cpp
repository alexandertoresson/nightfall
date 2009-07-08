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

		// TODO: Support archives
		bool FileExists(const std::string& filename, ResolveType rType)
		{
			return rType == RESOLVE_READABLE ? Utilities::FileIsReadable(filename) : Utilities::FileIsWritable(filename);
		}

		std::string Resolve(const std::string& filename, ResolveType rType = RESOLVE_READABLE, VFSLevel level = -1, int i = -1)
		{
			MountVector &lmounts = level == -1 ? mounts : stateStack[level];
			if (i == -1)
				i = lmounts.size()-1;
			
			MountType& cur = lmounts[i];
			unsigned len = cur.first.length();
			std::string newfn = filename;

			if (cur.first.find_last_of("\\/") == cur.first.size()-1)
			{
				if (len <= filename.length() && cur.first == filename.substr(0, len))
				{
					newfn = cur.second + filename.substr(len);
				}
			}
			else
			{
				if (cur.first == filename)
				{
					newfn = cur.second;
				}
			}
			if (i == 0)
			{
				return FileExists(newfn, rType) ? newfn : "";
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
					return Resolve(filename, rType, level, i-1);
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
