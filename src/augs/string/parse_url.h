#pragma once
#include <string>

struct parsed_url {
	std::string protocol;
	std::string host;
	std::string location;
	std::string query;

	parsed_url(const std::string& url_s) {
		using namespace std;

		const string prot_end("://");
		string::const_iterator prot_i = search(url_s.begin(), url_s.end(),
		prot_end.begin(), prot_end.end());
		protocol.reserve(distance(url_s.begin(), prot_i));
		transform(url_s.begin(), prot_i,
		back_inserter(protocol),
		[](int a) -> int { return(tolower(a)); } ); // protocol is icase
		if( prot_i == url_s.end() )
		return;
		advance(prot_i, prot_end.length());
		string::const_iterator path_i = find(prot_i, url_s.end(), '/');
		host.reserve(distance(prot_i, path_i));
		transform(prot_i, path_i,
		back_inserter(host),
		[](int a) -> int { return(tolower(a)); }); // host is icase
		string::const_iterator query_i = find(path_i, url_s.end(), '?');
		location.assign(path_i, query_i);
		if( query_i != url_s.end() )
		++query_i;
		query.assign(query_i, url_s.end());
	}
};

