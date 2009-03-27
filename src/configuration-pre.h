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

#ifndef CONFIGURATION_H_PRE
#define CONFIGURATION_H_PRE

#ifdef DEBUG_DEP
#warning "configuration.h-pre"
#endif

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <set>

namespace Utilities
{
	class ConfigurationFile
	{
		public:
			class Restriction
			{
				public:
					virtual std::string Apply(std::string str) = 0;
					virtual std::string GetDefault() = 0;
					virtual ~Restriction() {}
			};
			
			template<typename T>
			class TypeRestriction : public Restriction
			{
				private:
					T def;
				public:
					TypeRestriction(T def)
					{
						this->def = def;
					}

					virtual std::string Apply(std::string str)
					{
						std::stringstream ss(str);
						T a;
						ss >> a;

						if (ss.fail())
							return GetDefault();

						return str;
					}

					virtual std::string GetDefault()
					{
						std::stringstream ss;
						ss << def;
						return ss.str();
					}
			};

			template<typename T>
			class EnumRestriction : public TypeRestriction<T>
			{
				private:
					std::set<std::string> vals;
				public:
					EnumRestriction(std::set<std::string> vals, T def) : TypeRestriction<T>::TypeRestriction(def)
					{
						this->vals = vals;
					}

					virtual std::string Apply(std::string str)
					{
						std::string s = TypeRestriction<T>::Apply(str);

						if (vals.find(s) == vals.end())
							return TypeRestriction<T>::GetDefault();
						
						return s;
					}
			};
	
			void SetRestriction(const std::string& key, Restriction* restriction);

		private:
			std::map<std::string, std::string> mData;
			std::map<std::string, Restriction*> restrictions;
			std::string mFile;
	
		public:
			ConfigurationFile(void);
			ConfigurationFile(const std::string);
			
			~ConfigurationFile(void);
			
			void SetFile(const std::string);
			
			int Parse(void);
			int Update(std::string = "");
			void Clear(void);
			
			std::string GetValue(std::string);
			void        SetValue(std::string, std::string);
	};
	
	struct StructuredInstructionsItem
	{
		std::string instruction;
		std::string value;
	};	
	
	typedef std::vector<StructuredInstructionsItem*> StructuredInstructionsVector;
	
	class StructuredInstructionsFile
	{
		private:
			char* mFile;
			int   mIndex;
			int   mLength;
			StructuredInstructionsVector mItems;
			
		public:
			StructuredInstructionsFile(void);
			StructuredInstructionsFile(const std::string);
			
			~StructuredInstructionsFile(void);
			
			void SetFile(const std::string);
			
			int Parse(void);
			
			const StructuredInstructionsVector* GetInstructionVector(void) const;
			void GetInstructionVector(StructuredInstructionsVector*);
			
			void PrepareIterator(void);
			bool End(void) const;
			void GotoItem(int index);
			StructuredInstructionsItem* NextItem(void);
	};

	extern ConfigurationFile mainConfig;
}

#endif
