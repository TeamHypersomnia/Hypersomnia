#pragma once
#include <string>
#include "application/network/host_with_default_port.h"

using client_connect_string = std::string;

bool begins_with(const std::string& value, const std::string& beginning);
std::string& cut_preffix(std::string& value, const std::string& preffix);

inline bool is_official_webrtc_id(const client_connect_string& s) {
	/* Non-ip with a : */
	return s.find(":") != std::string::npos && s.find(".") == std::string::npos;
}

inline std::string find_webrtc_id(client_connect_string s) {
#if PLATFORM_WEB
	/* IP or not, it's always webrtc id */
	return s;
#endif

	if (::is_official_webrtc_id(s)) {
		return s;
	}

	if (begins_with(s, "localhost")) {
		return "";
	}

	if (s.find_first_of(".:[]") != std::string::npos) {
		return "";
	}

	return s;
}

inline bool uses_webrtc(const client_connect_string& s) {
	return find_webrtc_id(s) != "";
}
