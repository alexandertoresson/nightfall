#include "curl.h"
#include "sdlheader.h"

#include <curl/curl.h>

namespace Utilities
{
	
	void CURLRequest::Init()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}

	void CURLRequest::Cleanup()
	{
		curl_global_cleanup();
	}

	void CURLRequest::Fetch(std::string url)
	{
		this->url = url;
		SDL_CreateThread(CURLRequest::_Thread, this);
	}
	
	int CURLRequest::_Thread(void *arg)
	{
		CURLRequest *req = (CURLRequest*) arg;
		CURL *curl;
		CURLcode res;

		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, req->url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLRequest::_WriteFunc);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);

			req->ret = "";

			res = curl_easy_perform(curl);

			if (res)
			{
				req->Fail();
			}
			else
			{
				req->Handle(req->ret);
			}

			curl_easy_cleanup(curl);
		}
		else
		{
			req->Fail();
		}
		return 1;
	}
	
	int CURLRequest::_WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		CURLRequest *req = (CURLRequest*) stream;
		req->ret.append((char*)ptr, size*nmemb);
		return size*nmemb;
	}
}

