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
#ifndef __CURL_H_PRE__
#define __CURL_H_PRE__

#ifdef DEBUG_DEP
#warning "curl.h-pre"
#endif

#include <string>
#include <map>

namespace Utilities
{
	class CURLRequest
	{
		private:
			static int _Thread(void *arg);
			static int _WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
		protected:
			std::string url;
			std::string ret;
		public:
			static void Init();
			static void Cleanup();

			void Fetch(std::string url);
			void HTTPGET(std::string url, std::map<std::string, std::string> params);

			virtual void Handle() = 0;
			virtual void Fail() = 0;
	};
}

#endif
