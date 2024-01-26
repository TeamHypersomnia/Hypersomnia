#pragma once
#include "3rdparty/include_httplib.h"
#include "application/detail_file_paths.h"
#include "augs/string/parse_url.h"

namespace httplib_utils {
	inline bool successful(const int http_status_code) {
		return http_status_code >= 200 && http_status_code < 300;
	}

	template <class... F>
	decltype(auto) launch_download(http_client_type& client, const std::string& resource, F&&... args) {
		return client.Get(resource.c_str(), std::forward<F>(args)...);
	}

	inline auto client_from_parsed(const parsed_url& parsed) {
		auto proto_and_host = parsed.protocol.empty() ? parsed.host : (parsed.protocol + "://" + parsed.host);

		if (parsed.port != -1) {
			return std::make_unique<http_client_type>(proto_and_host, parsed.port);
		}

		return std::make_unique<http_client_type>(proto_and_host);
	}

	inline auto make_client(const parsed_url& parsed, const int io_timeout = 5) {
		const auto ca_path = CA_CERT_PATH;

		auto http_client_ptr = client_from_parsed(parsed);
		auto& http_client = *http_client_ptr;

#if BUILD_OPENSSL
		http_client.set_ca_cert_path(ca_path.c_str());

		const bool is_https = parsed.protocol.empty() || parsed.protocol == "https";

		http_client.enable_server_certificate_verification(is_https);
#endif

		http_client.set_follow_location(true);
		http_client.set_read_timeout(io_timeout);
		http_client.set_write_timeout(io_timeout);

		return http_client_ptr;
	}
}
