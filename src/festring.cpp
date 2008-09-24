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

#include "festring.h"
	
namespace Utilities
{

	void FEString::SetStr(const std::string& str)
	{
		std::map<std::string, unsigned>::iterator it = strToID.find(str);

		if (it != strToID.end())
		{
			id = it->second;
			this->str = &it->first;
		}
		else
		{
			strToID[str] = nextID;
			id = nextID;
			this->str = &strToID.find(str)->first;
			nextID++;
		}
	}

	FEString::FEString(const std::string& str)
	{
		SetStr(str);
	}

	FEString::FEString(const char* const str)
	{
		SetStr(std::string(str));
	}

	FEString::FEString(const FEString& a)
	{
		this->str = a.str;
		this->id = a.id;
	}

	FEString::FEString()
	{
		
	}

	bool FEString::operator ==(const FEString& a) const
	{
		return a.id == id;
	}
	
	bool FEString::operator ==(const std::string& a) const
	{
		return a == *str;
	}
	
	bool FEString::operator !=(const FEString& a) const
	{
		return a.id != id;
	}
	
	bool FEString::operator !=(const std::string& a) const
	{
		return a != *str;
	}
	
	bool FEString::operator <(const FEString& a) const
	{
		return a.id < id;
	}
	
	std::map<std::string, unsigned> FEString::strToID;
	unsigned FEString::nextID = 0;
}
