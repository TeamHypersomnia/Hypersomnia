#include "http_requests.h"

#if BUILD_HTTP_REQUESTS

#include <winsock2.h>
#include <windows.h>
#undef min
#undef max
using namespace std;

void mParseUrl(string url, string &serverName, string &filepath, string &filename)
{
	string::size_type n;

	if (url.substr(0, 7) == "http://")
		url.erase(0, 7);

	if (url.substr(0, 8) == "https://")
		url.erase(0, 8);

	n = url.find('/');
	if (n != string::npos)
	{
		serverName = url.substr(0, n);
		filepath = url.substr(n);
		n = filepath.rfind('/');
		filename = filepath.substr(n + 1);
	}

	else
	{
		serverName = url;
		filepath = "/";
		filename = "";
	}
}
#include "augs/log.h"
namespace augs {
	std::string http_post_request(const std::string& url, const std::string& additional_headers, const std::string& post_data) {
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			LOG("WSAStartup failed.");
			return {};
		}

		SOCKET Socket;

		SOCKADDR_IN SockAddr;
		int lineCount = 0;
		int rowCount = 0;
		struct hostent *host;
		locale local;
		char buffer[10000];
		int i = 0;
		int nDataLength;
		string website_HTML;
		
		string serverName;  string filepath; string filename;

		mParseUrl(url, serverName, filepath, filename);

		Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		host = gethostbyname(serverName.c_str());

		if (host == nullptr) {
			LOG("Could not establish connection to %x", url);
			return{};
		}

		SockAddr.sin_port = htons(80);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

		if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
			return "";
		}

		string get_http = "POST ";
		get_http += filepath;
		get_http += " HTTP/1.1";
		get_http += "\r\nHost: " + serverName;
		get_http += "\r\nAccept: */*";
		get_http += additional_headers;
		get_http += "\r\nContent-Type: application/x-www-form-urlencoded";
		get_http += "\r\nContent-Length: ";
		get_http += std::to_string(post_data.length());
		get_http += "\r\n\r\n";
		get_http += post_data;
		// send GET / HTTP
		send(Socket, get_http.c_str(), static_cast<int>(get_http.length()), 0);

		// recieve html
		while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0) {
			int i = 0;
			while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {

				website_HTML += buffer[i];
				i += 1;
			}
		}

		closesocket(Socket);

		return website_HTML;
	}

	std::string http_get_request(const std::string& url) {
		WSADATA wsaData;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			LOG("WSAStartup failed.");
			return {};
		}

		SOCKET Socket;
		SOCKADDR_IN SockAddr;
		int lineCount = 0;
		int rowCount = 0;
		struct hostent *host;
		locale local;
		char buffer[10000];
		int i = 0;
		int nDataLength;
		string website_HTML;

		string serverName;  string filepath; string filename;

		mParseUrl(url, serverName, filepath, filename);

		Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		host = gethostbyname(serverName.c_str());

		if (host == nullptr) {
			LOG("Could not establish connection to %x", url);
			return{};
		}

		SockAddr.sin_port = htons(80);
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

		if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
			return "";
		}

		string get_http = "GET ";
		get_http += filepath;
		get_http += " HTTP/1.0";
		get_http += "\r\nHost: " + serverName;
		get_http += "\r\n\r\n";
		// send GET / HTTP
		send(Socket, get_http.c_str(), static_cast<int>(get_http.length()), 0);

		// recieve html
		while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0) {
			int i = 0;
			while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {

				website_HTML += buffer[i];
				i += 1;
			}
		}

		closesocket(Socket);

		return website_HTML;
}
}
#else

namespace augs {
	std::string http_post_request(const std::string& /* url */, const std::string& /* additional_headers */, const std::string& /* post_data */) {
		return std::string();
	}

	std::string http_get_request(const std::string& /* url */) {
		return std::string();
	}
}

#endif
