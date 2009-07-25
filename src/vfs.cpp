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

		class ResolvedFile
		{
			private:
				std::string path;
				std::string zipPath;

			public:
				ResolvedFile(std::string path = "", std::string zipPath = "") : path(path), zipPath(zipPath) {}

				// Throws FileNotFoundException if the file is not found inside the archive
				std::string GetRealFileName()
				{
					if (zipPath.length())
					{
						ArchiveReader ar(zipPath);
						return ar.ExtractFile(path);
					}
					return path;
				}

				std::string ToString()
				{
					return zipPath.length() ? zipPath + ":" + path : path;
				}

				bool Exists()
				{
					if (zipPath.length())
					{
						ArchiveReader ar(zipPath);
						return ar.Exists(path);
					}
					return Utilities::FileExists(path);
				}
	
				bool ListFilesInDirectory(FSEntryList& list)
				{
					if (zipPath.length())
					{
						ArchiveReader ar(zipPath);
						return ar.ListFilesInDirectory(path, list);
					}
					return Utilities::ListFilesInDirectory(path, list);
				}
		};

		enum ResolveType
		{
			RESOLVE_READABLE,
			RESOLVE_WRITABLE,
			RESOLVE_DIRECTORY
		};

		ResolvedFile CheckAndResolvePath(const std::string& filename, ResolveType rType)
		{
			if (rType == RESOLVE_WRITABLE)
			{
				if (!Utilities::FileIsWritable(filename))
				{
					throw FileNotFoundException(filename);
				}
				return ResolvedFile(filename);
			}
			else
			{
				ResolvedFile rf;
				std::string path;
				if (filename.find(".zip") != std::string::npos)
				{
					bool first = true;
					std::vector<std::string> elems;
					split(filename, '/', elems);

					for (std::vector<std::string>::iterator it = elems.begin(); it != elems.end(); ++it)
					{
						if (!first)
						{
							path.push_back('/');
						}
						path += *it;
						first = false;

						if (it->size() >= 4 && it->substr(it->size()-4) == ".zip")
						{
							rf = ResolvedFile(path, rf.GetRealFileName());
							path = "";
						}
					}
					if (filename[filename.length()-1] == '/')
					{
						path.push_back('/');
					}
					rf = ResolvedFile(path, rf.GetRealFileName());
				}
				else
				{
					rf = ResolvedFile(filename);
				}

				if (rType == RESOLVE_DIRECTORY)
				{
					if (!rf.Exists())
					{
						throw FileNotFoundException(rf.ToString());
					}
					return rf;
				}
				else // if (rType == RESOLVE_READABLE)
				{
					path = rf.GetRealFileName();
					if (!Utilities::FileIsReadable(path))
					{
						throw FileNotFoundException(path);
					}
					return ResolvedFile(path);
				}
			}
		}

		void Resolve(const std::string& filename, std::vector<ResolvedFile>& ls, ResolveType rType = RESOLVE_READABLE, VFSLevel level = -1, int i = -1)
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
				try
				{
					ls.push_back(CheckAndResolvePath(newfn, rType));
				}
				catch (FileNotFoundException e)
				{
					return;
				}
			}
			else
			{
				Resolve(newfn, ls, rType, level, i-1);
				if (ls.size() && rType != RESOLVE_DIRECTORY)
				{
					return;
				}
				else if (modified)
				{
					Resolve(filename, ls, rType, level, i-1);
				}
			}
		}

		std::string ResolveReadable(const std::string& filename, VFSLevel level)
		{
			std::vector<ResolvedFile> ls;
			Resolve(filename, ls, RESOLVE_READABLE, level);
			if (ls.size())
				return ls.front().ToString();
			else
				return "";
		}

		std::string ResolveWritable(const std::string& filename, VFSLevel level)
		{
			std::vector<ResolvedFile> ls;
			Resolve(filename, ls, RESOLVE_WRITABLE, level);
			if (ls.size())
				return ls.front().ToString();
			else
				return "";
		}

		bool GetDirectoryListing(const std::string& path, FSEntryList& list, VFSLevel level)
		{
			std::vector<ResolvedFile> resolved;
			Resolve(path, resolved, RESOLVE_DIRECTORY, level);
			for (std::vector<ResolvedFile>::reverse_iterator it = resolved.rbegin(); it != resolved.rend(); ++it)
			{
				it->ListFilesInDirectory(list);
			}
			return !resolved.empty();
		}

		// TODO: Optimize
		bool FileIsReadable(const std::string& filename, VFSLevel level)
		{
			return ResolveReadable(filename, level).size();
		}

		// TODO: Optimize
		bool FileIsWritable(const std::string& filename, VFSLevel level)
		{
			return ResolveWritable(filename, level).size();
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
