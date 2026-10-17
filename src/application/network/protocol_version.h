#pragma once
#include <string>

/*
	The protocol version is the game's major.minor ("HS1.2"): patch releases play together, a development build (0.0.0)
	only with development builds. The handshake refuses another one (augs::network::set_protocol_version), and the server
	list shows such servers as incompatible (server_heartbeat::versions_compatible).
*/

inline std::string protocol_version_of(const std::string& version) {
	const auto second_dot = version.find('.', version.find('.') + 1);
	return "HS" + version.substr(0, second_dot);
}
