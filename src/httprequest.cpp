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

#include "httprequest.h"

#include "utilities.h"
#include "sdlheader.h"

namespace Utilities
{	

	std::string HTTPRequest::URLEscape(std::string str)
	{
		std::string ret = "";
		for (std::string::iterator it = str.begin(); it != str.end(); it++)
		{
			char c = *it;
			if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			{
				ret += c;
			}
			else
			{
				std::stringstream ss;
				ss << std::hex << (int) c;
				ret += "%" + ss.str();
			}
		}
		return ret;
	}

	void HTTPRequest::Fetch(std::string host, int port, std::string request)
	{
		this->host = host;
		this->request = request;
		this->port = port;
		SDL_CreateThread(HTTPRequest::_Thread, this);
	}

	void HTTPRequest::Fetch(std::string method, std::string host, int port, std::string path, std::map<std::string, std::string> params, std::map<std::string, std::string> header, std::string contents)
	{
		bool first = true;
		
		path += "?";

		for (std::map<std::string, std::string>::iterator it = params.begin(); it != params.end(); it++)
		{
			std::string encodedKey = URLEscape(it->first);
			std::string encodedVal = URLEscape(it->second);
			if (!first)
			{
				path += "&";
			}
			path += encodedKey + "=" + encodedVal;
			first = false;
		}
		
		std::string request = method + " " + path + " HTTP/1.1\r\n";

		header["Connection"] = "close";
		header["Host"] = host;
		header["User-Agent"] = "Nightfall RTS/0.1 (nightfall-rts.org)";

		for (std::map<std::string, std::string>::iterator it = header.begin(); it != header.end(); it++)
		{
			request += it->first + ": " + it->second + "\r\n";
		}

		request += "\r\n" + contents;

		Fetch(host, port, request);
	}
	
	void HTTPRequest::GET(std::string url, std::map<std::string, std::string> params)
	{
		std::string::size_type index = url.find("//", 0);
		if (index != std::string::npos)
		{
			url = url.substr(index+2);
		}

		std::string host = url;
		std::string path = "/";

		index = url.find("/", 0);
		
		if (index != std::string::npos)
		{
			host = url.substr(0, index);
			path = url.substr(index);
		}

		int port = 80;

		index = host.find(":", 0);
		
		if (index != std::string::npos)
		{
			port = StringCast<int>(url.substr(index));
			host = host.substr(0, index);
		}

		Fetch("GET", host, port, path, params, std::map<std::string, std::string>(), "");
	}

	int HTTPRequest::_Thread(void *arg)
	{
		HTTPRequest *req = (HTTPRequest*) arg;

		std::string raw;
		TCPsocket socket;
		SDLNet_SocketSet set;
		int result;

		if (!req)
		{
			goto fail;
		}

		IPaddress clientConnect;

		if (SDLNet_ResolveHost(&clientConnect, req->host.c_str(), req->port) != 0)
		{
			goto fail;
		}

		socket = SDLNet_TCP_Open(&clientConnect);
		set = SDLNet_AllocSocketSet(1);

		if (socket != NULL)
		{
			SDLNet_TCP_AddSocket(set, socket);
		}
		else
		{
			goto fail;
		}

		std::cout << req->request << std::endl;

		result = SDLNet_TCP_Send(socket, req->request.c_str(), req->request.length());

		if (result < (signed) req->request.length()) 
		{
			goto fail;
		}

		while (true)
		{
			int numActive = SDLNet_CheckSockets(set, 10);
			char inBuf[1024];
			if (numActive && (SDLNet_SocketReady(socket)))
			{
				int numBytes = SDLNet_TCP_Recv(socket, &inBuf, 1024);
				if (numBytes <= 0)
				{
					break;
				}

				raw += std::string((char*)&inBuf, numBytes);
			}
		}

		std::cout << raw << std::endl;

		ret:

		SDLNet_FreeSocketSet(set);

		delete req;
		return 1;

		fail:
		req->Fail();
		goto ret;
	}	
}
