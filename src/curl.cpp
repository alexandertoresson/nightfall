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
	
	void CURLRequest::HTTPGET(std::string url, std::map<std::string, std::string> params)
	{
		bool first = true;
		CURL *curl = curl_easy_init();
		
		url += "?";

		for (std::map<std::string, std::string>::iterator it = params.begin(); it != params.end(); it++)
		{
			char* encodedKey = curl_easy_escape(curl, it->first.c_str(), it->first.length());
			char* encodedVal = curl_easy_escape(curl, it->second.c_str(), it->second.length());
			if (!first)
			{
				url += "&";
			}
			url += std::string(encodedKey) + "=" + std::string(encodedVal);
			first = false;
		}
		curl_easy_cleanup(curl);
		Fetch(url);
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

		delete req;

		return 1;
	}
	
	int CURLRequest::_WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		CURLRequest *req = (CURLRequest*) stream;
		req->ret += std::string((char*)ptr, size*nmemb);
		return size*nmemb;
	}
}

