#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>

/*
	Secure connections to the official servers.

	Before connecting, the client gets a connect token from the masterserver over HTTPS.
	Its private part is encrypted with the netcode key of that one official server,
	so only that server can read it and learn the session keys the client got with the token.

	The token's user data says which official server it was issued for.
	netcode wipes the user data of the insecure tokens clients make themselves,
	so a server that has a netcode key can trust the server id it finds there.
*/

namespace secure_connect {
	constexpr std::size_t connect_token_bytes_v = 2048;
}

/*
	A secure connect token for one official server, as the client gets it from the masterserver.
*/

struct fetched_connect_token {
	uint64_t client_id = 0;
	std::string server_id;
	std::array<uint8_t, secure_connect::connect_token_bytes_v> token = {};
};

namespace secure_connect {
	constexpr std::size_t user_data_bytes = 256;
	constexpr std::size_t max_server_id_length = 64;

	/*
		How long a connect token issued by the masterserver is valid.
		The servers are told the same through maxConnectTokenLifetime.
	*/

	constexpr int token_lifetime_secs = 60;

	inline constexpr std::string_view user_data_marker = "HSEC1";

	using user_data_type = std::array<uint8_t, user_data_bytes>;

	inline std::optional<user_data_type> make_user_data(const std::string& server_id) {
		if (server_id.empty() || server_id.size() > max_server_id_length) {
			return std::nullopt;
		}

		auto out = user_data_type();
		std::memcpy(out.data(), user_data_marker.data(), user_data_marker.size());
		std::memcpy(out.data() + user_data_marker.size(), server_id.data(), server_id.size());

		return out;
	}

	/*
		What a client authenticates for: the server's per-connection challenge and the id of the official server
		its connect token was issued for. A ticket or a signature given to one official server
		thus can not be passed on to another, whose own token says a different id.
	*/

	inline std::string auth_identity(const std::string& challenge, const std::string& server_id) {
		return challenge + "@" + server_id;
	}

	inline std::optional<std::string> read_server_id(const uint8_t* const user_data) {
		if (user_data == nullptr) {
			return std::nullopt;
		}

		if (std::memcmp(user_data, user_data_marker.data(), user_data_marker.size()) != 0) {
			return std::nullopt;
		}

		const auto* const id_begin = reinterpret_cast<const char*>(user_data + user_data_marker.size());

		std::size_t length = 0;

		while (length < max_server_id_length && id_begin[length] != '\0') {
			++length;
		}

		if (length == 0) {
			return std::nullopt;
		}

		return std::string(id_begin, length);
	}
}
