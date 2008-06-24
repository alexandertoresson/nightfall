
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
