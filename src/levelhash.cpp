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

#include "levelhash.h"

#include "vfs.h"
#include "filesystem.h"

#include "Poco/StreamCopier.h"
#include "Poco/DigestStream.h"
#include "Poco/SHA1Engine.h"
#include <fstream>

namespace Game
{
	std::string GetLevelBySHA1Sum(std::string sha1sum)
	{
		Utilities::FSEntryList ls;
		Utilities::VFS::GetDirectoryListing("/data/levels/", ls);
		for (Utilities::FSEntryList::iterator it = ls.begin(); it != ls.end(); ++it)
		{
			std::string sum = GetLevelSHA1Sum(it->first);
			if (sum == sha1sum)
				return it->first;
		}
		return "";
	}
	
	static void StreamDirectoryContents(std::string path, std::string relpath, std::ostream& os)
	{
		Utilities::FSEntryList ls;
		std::string curpath = path + relpath;
		Utilities::VFS::GetDirectoryListing(curpath, ls);
		for (Utilities::FSEntryList::iterator it = ls.begin(); it != ls.end(); ++it)
		{
			os << relpath << it->first;
			if (it->second.isDirectory)
			{
				StreamDirectoryContents(path, relpath + it->first + "/", os);
			}
			else
			{
				std::cout << curpath + it->first << std::endl;
				std::string file = Utilities::VFS::ResolveReadable(curpath + it->first);
				std::ifstream is(file.c_str(), std::ios::binary);
				Poco::StreamCopier::copyStream(is, os);
			}
		}
	}

	std::string GetLevelSHA1Sum(std::string level)
	{
		Poco::SHA1Engine sha1;
		Poco::DigestOutputStream dos(sha1);

		StreamDirectoryContents("/data/levels/" + level + "/", "", dos);

		dos.close();

		return Poco::DigestEngine::digestToHex(sha1.digest());
	}
}

