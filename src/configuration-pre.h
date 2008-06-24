
#ifndef __CONFIGURATION_H_PRE__
#define __CONFIGURATION_H_PRE__

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
