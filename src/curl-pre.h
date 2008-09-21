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
