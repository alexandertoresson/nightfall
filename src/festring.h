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

#ifndef FESTRING_H
#define FESTRING_H

#include <map>
#include <string>

namespace Utilities
{
	class FEString
	{
		private:
			static std::map<std::string, unsigned> strToID;
			static unsigned nextID;
			unsigned id;
			const std::string *str;
			
			void SetStr(const std::string& str);
		public:
			FEString(const char* const str);
			FEString(const std::string& str);
			FEString(const FEString& str);
			FEString();

			bool operator ==(const FEString& a) const;
			bool operator ==(const std::string& a) const;
			bool operator !=(const FEString& a) const;
			bool operator !=(const std::string& a) const;
			
			bool operator <(const FEString& a) const;
	};
}

#endif
