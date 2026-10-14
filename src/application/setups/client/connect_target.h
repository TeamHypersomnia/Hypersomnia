#pragma once
#include <optional>
#include <string>
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"
#include "augs/misc/secure_hash.h"

/*
	What a join names - one string everywhere (--connect, the config's last join, Steam's invites, the server browser):
	- "<public key>": the server with this key, wherever it is now (the server list, else the favorites' last address);
	- "<address>,<public key>": there, expecting that key;
	- "<address>": there, whatever key it has - nothing verifies who answers.
	A server is known by its key; an address is only where it is.
*/

struct connect_target {
	std::string address;
	std::optional<augs::ed25519_public_key_type> key;

	static std::optional<augs::ed25519_public_key_type> key_of(const std::string& hex) {
		if (hex.size() != 64) {
			return std::nullopt;
		}

		return augs::hex_to_array<32>(hex);
	}

	static connect_target parse(const std::string& in) {
		auto out = connect_target();

		if (const auto key = key_of(in)) {
			out.key = key;
			return out;
		}

		if (const auto comma = in.rfind(','); comma != std::string::npos) {
			if (const auto key = key_of(in.substr(comma + 1))) {
				out.address = in.substr(0, comma);
				out.key = key;
				return out;
			}
		}

		out.address = in;
		return out;
	}

	std::string to_string() const {
		if (!key.has_value()) {
			return address;
		}

		const auto hex = augs::bytes_to_hex(*key);
		return address.empty() ? hex : address + "," + hex;
	}
};
