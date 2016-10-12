#pragma once
#include <string>

namespace augs {
	std::string http_post_request(const std::string& url, const std::string& additional_headers, const std::string& post_data);
}