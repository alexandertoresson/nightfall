#ifndef __CURL_H_PRE__
#define __CURL_H_PRE__

#ifdef DEBUG_DEP
#warning "curl.h-pre"
#endif

#include <string>

namespace Utilities
{
	class CURLRequest
	{
		private:
			std::string url;
			std::string ret;
			static int _Thread(void *arg);
			static int _WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
			static void Init();
			static void Cleanup();
		public:
			void Fetch(std::string url);
			virtual void Handle(std::string ret) = 0;
			virtual void Fail() = 0;
	};
}

#endif
