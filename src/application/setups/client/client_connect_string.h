#pragma once
#include <string>
#include "application/network/host_with_default_port.h"

using client_connect_string = std::string;

bool begins_with(const std::string& value, const std::string& beginning);
std::string& cut_preffix(std::string& value, const std::string& preffix);

inline std::string find_webrtc_id(client_connect_string s) {
	if (begins_with(s, "web://")) {
		cut_preffix(s, "web://");

		return s;
	}

	return "";
}

inline bool uses_webrtc(const client_connect_string& s) {
	return find_webrtc_id(s) != "";
}
