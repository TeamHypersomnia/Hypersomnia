#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <iterator>

struct parsed_url {
    std::string protocol;
    std::string host;
    int port = -1; // Default value -1 indicates no port specified
    std::string location;
    std::string query;

    parsed_url(const std::string& url_s) {
        if (url_s.empty()) {
            return;
        }

        using namespace std;

        const string prot_end("://");
        auto prot_i = search(url_s.begin(), url_s.end(), prot_end.begin(), prot_end.end());

        if (prot_i != url_s.end()) {
            protocol.reserve(distance(url_s.begin(), prot_i));
            transform(url_s.begin(), prot_i, back_inserter(protocol), [](int c) { return tolower(c); });
            advance(prot_i, prot_end.length());
        } else {
            prot_i = url_s.begin();
        }

        // Parse host, separating it from path and query
        auto host_end = find(prot_i, url_s.end(), '/');
        auto port_i = find(prot_i, host_end, ':');
        host.reserve(distance(prot_i, port_i));
        transform(prot_i, port_i, back_inserter(host), [](int c) { return tolower(c); });

        // Extract port if present
        if (port_i != host_end && port_i != url_s.end()) {
            string port_str(port_i + 1, host_end);
            try {
                port = stoi(port_str);
            } catch (const invalid_argument& e) {
                port = -1; // Invalid or no port specified
            }
        }

        // Parse path and query
        auto query_i = find(host_end, url_s.end(), '?');
        if (host_end != url_s.end()) {
            location.assign(host_end, query_i);
        }
        if (query_i != url_s.end()) {
            query.assign(query_i + 1, url_s.end());
        }
    }

    bool valid() const {
        return !host.empty();
    }
};
