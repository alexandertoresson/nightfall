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
#include "configuration.h"
#include "errors.h"
#include "paths.h"
#include "utilities.h"
#include <cassert>
#include <fstream>

namespace Utilities
{
	ConfigurationFile mainConfig;

	// Tar bort intendentering
	void StringRemoveIntendent(std::string& target);
	
	// Klipper en sträng från position 0 tills slutet av strängen - eller bokstaven/symbolen ending påträffas
	int StringCut(std::string target, std::string& result, char ending);
	
	// Returns end of token. Stores next token in result. ',' is considered a token.
	// Example: "0.1," Token[0] = "0.1" and Token[1] = ","
	// Handles token types: Normal (word), String type ("info mation")
	int StringGetToken(std::string target, std::string& result, int start);
	
	ConfigurationFile::ConfigurationFile(void)
	{
	}
	
	ConfigurationFile::ConfigurationFile(const std::string file)
	{
		SetFile(file);
	}
	
	ConfigurationFile::~ConfigurationFile(void)
	{
	}
	
	void ConfigurationFile::SetFile(const std::string file)
	{
		mFile = file;
	}
	
	void ConfigurationFile::SetRestriction(const std::string& key, ConfigurationFile::Restriction* restriction)
	{
		restrictions[key] = restriction;
	}
	
	int ConfigurationFile::Parse(void)
	{
		if (!mFile.length())
			return WINDOW_ERROR_NO_CONFIG_FILE;
	
		std::ifstream file;
		std::string filename = GetConfigFile(mFile);

		if (!filename.length())
		{
			return WINDOW_ERROR_CANNOT_FIND_CONFIGFILE;
		}
		
		file.open(filename.c_str());

		std::string buffer;
		std::string command;
		std::string value;
		while (file.eof() == false)
		{
			buffer.clear();

			Utilities::ReadLineFromFile(file, buffer);

			if(buffer.length() > 0)
			{
				Utilities::StringRemoveIntendent(buffer);
				
				if (buffer.length() == 0)
					continue;
				
				char trigger;
				trigger = buffer.at(0);
				
				// Konfigurationsvärde?
				if (trigger == '#')
					continue;
					
				// Klipp ut kommandot (kommando \t värde)
				// \t = tabulatur
				Utilities::StringRemoveIntendent(buffer);
				unsigned int endingPos = Utilities::StringCut(buffer, command, '\t');
				
				if (endingPos >= buffer.length())
					continue;
				
				// Hämta värdet
				value = buffer.substr(endingPos);
				Utilities::StringRemoveIntendent(value);
				
				// Does a restriction exist?
				std::map<std::string, Restriction*>::iterator it = restrictions.find(command);
				if (it != restrictions.end())
				{
					// If so, apply it!
					value = it->second->Apply(value);
				}

				// Spara i hashtabellen
				mData[command] = value;
				
				// Töm kommandot
				command.clear();
			}
		}
		
		file.close();

		return SUCCESS;
	}
	
	int ConfigurationFile::Update(std::string output /* = "" */)
	{
		if (output.length())
		{
			mFile = output;
		}
		
		std::ofstream file(mFile.c_str());
		
		std::map<std::string, std::string>::iterator iter;
		for (iter = mData.begin(); iter != mData.end(); iter++)
			file << '-' << (*iter).first << '\t' << (*iter).second << '\n';
		
		file.close();
		
		return SUCCESS;
	}

	void ConfigurationFile::Clear()
	{
		mData.clear();
	}
	
	std::string ConfigurationFile::GetValue(std::string key)
	{
		std::map<std::string, std::string>::iterator it = mData.find(key);
		
		if (it == mData.end())
		{
			// Do we have a default value through a restriction?
			std::map<std::string, Restriction*>::iterator it = restrictions.find(key);
			if (it != restrictions.end())
			{
				return it->second->GetDefault();
			}
			return "";
		}

		return (*it).second;
	}
	
	void ConfigurationFile::SetValue(std::string key, std::string value)
	{
		// Does a restriction exist?
		std::map<std::string, Restriction*>::iterator it = restrictions.find(key);
		if (it != restrictions.end())
		{
			// If so, apply it!
			value = it->second->Apply(value);
		}

		mData[key] = value;
	}

	StructuredInstructionsFile::StructuredInstructionsFile(void)
	{
		mFile = NULL;
		PrepareIterator();
	}
	
	StructuredInstructionsFile::StructuredInstructionsFile(const std::string file)
	{
		mFile = NULL;
		SetFile(file);
		PrepareIterator();
	}
	
	StructuredInstructionsFile::~StructuredInstructionsFile(void)
	{
		if (mItems.size() > 0)
		{
			for (unsigned int i = 0; i < mItems.size(); i++)
			{
				StructuredInstructionsItem* pItem = &*mItems.at(i);
				assert(pItem != NULL);
				delete pItem;
				pItem = NULL;
			}
			mItems.clear();
		}
	}
	
	void StructuredInstructionsFile::SetFile(const std::string file)
	{
		if (mFile)
		{
			delete mFile;
		}
		mFile = new char[file.size()+1];
		memcpy(mFile, file.c_str(), file.size()+1);
	}
	
	int StructuredInstructionsFile::Parse(void)
	{
		if (!mFile)
			return STRUCTURED_INSTRUCTIONS_ERROR_NO_LIST;
		
		std::ifstream file(mFile);
		
		if (file.good() == false)
			return STRUCTURED_INSTRUCTIONS_ERROR_NO_LIST;
		
		std::string buffer;
		std::string value;
		while (file.eof() == false)
		{
			Utilities::ReadLineFromFile(file, buffer);
			
			if (buffer.size() == 0)
				continue;
			
			StringRemoveIntendent(buffer);
			
			// Kommentar?
			if (buffer[0] == '#' || buffer[0] == 0)
				continue;

			unsigned int endingPos = StringCut(buffer, value, '\t');
						
			StructuredInstructionsItem* pSII = new StructuredInstructionsItem;
			pSII->instruction = (char*)value.c_str();
			if (endingPos >= buffer.length())
				pSII->value = "";
			else
			{
				value = buffer.substr(endingPos);
				StringRemoveIntendent(value);
				pSII->value = (char*)value.c_str();
			}
			
			mItems.push_back(pSII);
			
			buffer.clear();
			value.clear();
		}
		
		return SUCCESS;
	}
	
	const StructuredInstructionsVector* StructuredInstructionsFile::GetInstructionVector(void) const
	{
		return &mItems;
	}

	void StructuredInstructionsFile::GetInstructionVector(StructuredInstructionsVector* pVector)
	{
		pVector = &mItems;
	}
	
	void StructuredInstructionsFile::PrepareIterator(void)
	{
		mIndex  = 0;
		mLength = mItems.size(); 
	}
	
	bool StructuredInstructionsFile::End(void) const
	{
		return mIndex == mLength;
	}
	
	void StructuredInstructionsFile::GotoItem(int index)
	{
		if (index >= mLength)
			index = mLength - 1;
		
		mIndex = index;
	}
	
	StructuredInstructionsItem* StructuredInstructionsFile::NextItem(void)
	{
		return mIndex >= mLength ? NULL : mItems[mIndex++];
	}

	void StringRemoveIntendent(std::string& target)
	{
		while ((target.at(0) == ' ' || target.at(0) == '\t') && target.length() > 0)
		       target = target.substr(1);
	}
	
	int StringCut(std::string target, std::string& result, char ending)
	{ 
		char letter;
		for (unsigned int i = 0; i < target.length(); i++)
		{
			letter = target.at(i);
			
			if (letter == ending)
				return i;
			
			result += letter;
		}
		
		return target.length();
	}

	int StringGetToken(std::string target, std::string& result, int start)
	{
		int begin = -1;
		int end = -1;
		int length = target.length();
		bool stringToken = false;
		
		//Find begin
		for(int i = start; i < length; i++)
		{
			char current = target.at(i);
			if(current != '\t' && current != ' ')
			{
				if(current == '"')
				{
					stringToken = true;
				}
				begin = i;
				break;
			}				
		}
		
		if(begin == -1)
		{
			result = target.substr(start);
			return length;
		}
		
		char target_at = target.at(begin);
		switch(target_at)
		{
			case ',':
			{
				result = ",";
				return begin + 1;
			}
			case '[':
			{
				result = "[";
				return begin + 1;
			}
			case ']':
			{
				result = "]";
				return begin + 1;
			}
		}
		if(target_at == ',')
		{
			result = ",";
			return begin + 1;
		}
		
		//Find end
		if(stringToken == true)
		{
			for(int i = begin + 1; i < length; i++)
			{
				char current = target.at(i);
				if(current == '"')
				{
					end = i + 1;
					break;
				}				
			}			
		}
		else {
			for(int i = begin + 1; i < length; i++)
			{
				char current = target.at(i);
				if(current == '\t' || current == ' ' || current == ',' || current == '\r')
				{
					end = i;
					break;
				}				
			}
		}
		
		if(end == -1)
		{
			result = target.substr(begin);
			return length;
		}
		
		result = target.substr(begin, end - begin);
		return end;
		
	}
}
