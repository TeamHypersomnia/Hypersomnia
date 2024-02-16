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

	inline auto make_client(const std::string& scheme_host_port, const int io_timeout = 5) {
		const auto ca_path = CA_CERT_PATH;

		auto http_client_ptr = std::make_unique<http_client_type>(scheme_host_port.c_str());
		auto& http_client = *http_client_ptr;

#if BUILD_OPENSSL
		http_client.set_ca_cert_path(ca_path.c_str());
#endif

		http_client.set_follow_location(true);
		http_client.set_read_timeout(io_timeout);
		http_client.set_write_timeout(io_timeout);

		return http_client_ptr;
	}

	inline auto make_client(const parsed_url& parsed, const int io_timeout = 5) {
		return make_client(parsed.get_scheme_host_port(), io_timeout);
	}
}
