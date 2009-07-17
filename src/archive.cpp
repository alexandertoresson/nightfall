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

#include "archive.h"
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipStream.h>
#include <Poco/Zip/Compress.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/TemporaryFile.h>
#include <Poco/StreamCopier.h>
#include <fstream>
#include <iostream>

namespace Utilities
{

	TempFile::TempFile(const std::string& file) : file(file == "" ? Poco::TemporaryFile::tempName() : file)
	{
	}

	TempFile::~TempFile()
	{
		Poco::File(file).remove();
	}

	struct ArchiveReader::pimpl
	{
		pimpl(const std::string& filename) : inp(filename.c_str(), std::ios::binary), arch(NULL)
		{
			if (!inp.good())
			{
				throw FileNotFoundException(filename);
			}
			arch = new Poco::Zip::ZipArchive(inp);
		}
		~pimpl() { delete arch; }

		std::ifstream inp;
		Poco::Zip::ZipArchive* arch;
	};

	ArchiveReader::ArchiveReader(const std::string& filename) : priv(NULL), filename(filename)
	{
		priv = new pimpl(filename);
	}

	ArchiveReader::~ArchiveReader()
	{
		delete priv;
	}

	bool ArchiveReader::Exists(const std::string& cfile)
	{
		Poco::Zip::ZipArchive::FileHeaders::const_iterator it = priv->arch->findHeader(cfile);
		return it != priv->arch->headerEnd();
	}

	std::string ArchiveReader::ExtractFile(const std::string& cfile)
	{
		Poco::Zip::ZipArchive::FileHeaders::const_iterator it = priv->arch->findHeader(cfile);
		if (it == priv->arch->headerEnd())
		{
			throw FileNotFoundException(cfile + " in " + filename);
		}

		Poco::Zip::ZipInputStream zipin(priv->inp, it->second);

		std::string ofile = Poco::TemporaryFile::tempName();
		std::ofstream out(ofile.c_str(), std::ios::binary);
		Poco::StreamCopier::copyStream(zipin, out);
		return ofile;
	}
			
	struct ArchiveWriter::pimpl
	{
		pimpl(const std::string& filename) : out(filename.c_str(), std::ios::binary), c(NULL)
		{
			if (!out.good())
			{
				throw FileOpenForWriteException(filename);
			}
			c = new Poco::Zip::Compress(out, true);
		}
		~pimpl() { c->close(); delete c; }

		std::ofstream out;
		Poco::Zip::Compress* c;
	};

	ArchiveWriter::ArchiveWriter(const std::string& filename) : priv(NULL)
	{
		priv = new pimpl(filename);
	}
	
	ArchiveWriter::~ArchiveWriter()
	{
		delete priv;
	}
	
	void ArchiveWriter::AddRecursive(const std::string& path)
	{
		priv->c->addRecursive(Poco::Path(path));
	}
		
}
