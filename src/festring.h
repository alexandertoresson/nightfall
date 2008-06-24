
#ifndef __FESTRING_H__
#define __FESTRING_H__

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
