#pragma once
#include <optional>
#include <string>
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"
#include "augs/misc/secure_hash.h"
#include "augs/network/network_types.h"

/*
	What a join names - one string everywhere (--connect, the config's last join, Steam's invites, the server browser):
	- "<public key>": a server with this key, wherever it is now (the server list - of a host's instances the best ping -
	  else the favorites' last address);
	- "<address>,<public key>": there, expecting that key (the address picks the instance of the host);
	- "<address>": there, whatever key it has - nothing verifies who answers.
	A server is known by its key; an address is only where it is.
*/

struct connect_target {
	std::string address;
	std::optional<augs::ed25519_public_key_type> key;

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
		An address in one form - without the game's default port, which a join without a port uses anyway:
		"1.2.3.4:8412" is "1.2.3.4", "[::1]:8412" is "[::1]", "host:8412" is "host"; any other port stays.
		Not a bare IPv6 (all of "::1:8412" is the address) nor a WebRTC id ("rtc:..." - its port is a part of it).
		Never in a browser: every address there is a WebRTC id.
	*/

	static std::string without_default_port(const std::string& untrimmed) {
		const auto in = trimmed(untrimmed);

#if PLATFORM_WEB
		return in;
#else
		const auto suffix = ":" + std::to_string(DEFAULT_GAME_PORT_V);

		if (in.rfind("rtc:", 0) == 0 || in.size() <= suffix.size()) {
			return in;
		}

		if (in.compare(in.size() - suffix.size(), suffix.size(), suffix) != 0) {
			return in;
		}

		const auto host = in.substr(0, in.size() - suffix.size());
		const bool bracketed_ipv6 = host.front() == '[' && host.back() == ']';

		if (!bracketed_ipv6 && host.find(':') != std::string::npos) {
			return in;
		}

		return host;
#endif
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

	/*
		What an unlisted server's admin publishes: "<address>,0x<key>" or "<address>,0x<key>,<name>" (the name may hold
		commas; only its first line is taken) - see unlisted_server_fields.h for when it is taken.
	*/

	struct server_identifiers {
		std::string address;
		augs::ed25519_public_key_type key = {};
		std::string name;
	};

	static std::optional<server_identifiers> find_server_identifiers(const std::string& text) {
		const auto first = text.find(',');

		if (first == std::string::npos) {
			return std::nullopt;
		}

		const auto second = text.find(',', first + 1);
		const auto address = trimmed(text.substr(0, first));
		/*
			Only with "0x" - so that no other text with a comma in the clipboard is taken for one.
		*/

		const auto key_text = trimmed(text.substr(first + 1, second == std::string::npos ? std::string::npos : second - first - 1));

		if (key_text.rfind("0x", 0) != 0 && key_text.rfind("0X", 0) != 0) {
			return std::nullopt;
		}

		const auto key = key_of(key_text);

		if (address.empty() || !key.has_value() || address.find('\n') != std::string::npos) {
			return std::nullopt;
		}

		auto out = server_identifiers();
		out.address = address;
		out.key = *key;
		out.name = second == std::string::npos ? std::string() : trimmed(text.substr(second + 1));
		out.name = trimmed(out.name.substr(0, out.name.find('\n')));
		return out;
	}

	std::string to_string() const {
		const auto shown_address = without_default_port(address);

		if (!key.has_value()) {
			return shown_address;
		}

		const auto hex = augs::bytes_to_hex(*key);

		if (!shown_address.empty()) {
			return shown_address + "," + hex;
		}

		return hex;
	}
};
