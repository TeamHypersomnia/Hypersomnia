#pragma once
#include <optional>
#include <string>
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"
#include "augs/misc/secure_hash.h"
#include "application/masterserver/server_instance_label.h"

/*
	What a join names - one string everywhere (--connect, the config's last join, Steam's invites, the server browser):
	- "<public key>": a server with this key, wherever it is now (the server list - of a host's instances the best ping -
	  else the favorites' last address);
	- "<label>-<public key>": that instance of the host with this key (e.g. "c2-<key>", see server_instance_label.h);
	- "<address>,<public key>": there, expecting that key (the address picks the instance);
	- "<address>": there, whatever key it has - nothing verifies who answers.
	A server is known by its key; an address is only where it is.
*/

struct connect_target {
	std::string address;
	std::optional<augs::ed25519_public_key_type> key;
	std::string label;

	/*
		Pasted text: spaces, a newline around it.
	*/

	static std::string trimmed(const std::string& s) {
		const auto first = s.find_first_not_of(" \t\r\n");

		if (first == std::string::npos) {
			return std::string();
		}

		const auto last = s.find_last_not_of(" \t\r\n");
		return s.substr(first, last - first + 1);
	}

	/*
		A key as the game shows it (with 0x) or bare.
	*/

	static std::optional<augs::ed25519_public_key_type> key_of(const std::string& text) {
		auto hex = trimmed(text);

		if (hex.rfind("0x", 0) == 0 || hex.rfind("0X", 0) == 0) {
			hex = hex.substr(2);
		}

		if (hex.size() != 64) {
			return std::nullopt;
		}

		return augs::hex_to_array<32>(hex);
	}

	static connect_target parse(const std::string& untrimmed) {
		auto out = connect_target();
		const auto in = trimmed(untrimmed);

		if (const auto key = key_of(in)) {
			out.key = key;
			return out;
		}

		if (const auto dash = in.find('-'); dash != std::string::npos && is_instance_label(in.substr(0, dash))) {
			if (const auto key = key_of(in.substr(dash + 1))) {
				out.label = in.substr(0, dash);
				out.key = key;
				return out;
			}
		}

		if (const auto comma = in.rfind(','); comma != std::string::npos) {
			if (const auto key = key_of(in.substr(comma + 1))) {
				out.address = trimmed(in.substr(0, comma));
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

		if (!address.empty()) {
			return address + "," + hex;
		}

		return label.empty() ? hex : label + "-" + hex;
	}
};
