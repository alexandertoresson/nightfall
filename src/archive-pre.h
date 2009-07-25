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
#ifndef ARCHIVE_H_PRE
#define ARCHIVE_H_PRE

#ifdef DEBUG_DEP
#warning "archive.h-pre"
#endif

#include <string>
#include <stdexcept>

namespace Utilities
{
	class TempFile
	{
		private:
			std::string file;

			// disallow copying
			TempFile& operator =(const TempFile&);
			TempFile(const TempFile&);
		public:
			TempFile(const std::string& file = "");
			~TempFile();

			const std::string& str() const
			{
				return file;
			}
	};

	class FileNotFoundException : public std::runtime_error
	{
		public:
			FileNotFoundException(const std::string& file) : std::runtime_error("File \"" + file + "\" does not exist") { }
	};

	class FileOpenForWriteException : public std::runtime_error
	{
		public:
			FileOpenForWriteException(const std::string& file) : std::runtime_error("File \"" + file + "\" could not be opened for write") { }
	};

	class ArchiveWriter
	{
		private:
			struct pimpl;
			pimpl* priv;

			// disallow copying
			ArchiveWriter& operator =(const ArchiveWriter&);
			ArchiveWriter(const ArchiveWriter&);
		public:
			ArchiveWriter(const std::string&);
			~ArchiveWriter();

			void AddRecursive(const std::string&);
	};

}

#endif
