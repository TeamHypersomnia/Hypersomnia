#pragma once
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"

/*
	The keys of the servers this game connected to, by the address it connected with - like SSH's known_hosts.
	A server joined by address later shows the same key, or the game warns before connecting (see the key mismatch
	in client_setup) - also while the masterserver, whose list has the keys, can not be reached.
	The server list's key, when there is one, comes first - it is fetched over HTTPS.
	One line per address: the address, a space, the key in hex.
*/

class known_servers {
	augs::path_type path;
	std::map<std::string, std::string> key_by_address;

	void load() {
		key_by_address.clear();

		try {
			auto in = std::istringstream(augs::file_to_string(path));
			std::string line;

			while (std::getline(in, line)) {
				const auto space = line.rfind(' ');

				if (space != std::string::npos && space > 0 && line.size() - space - 1 == 64) {
					key_by_address[line.substr(0, space)] = line.substr(space + 1);
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		std::string content;

		for (const auto& [address, key] : key_by_address) {
			content += address + " " + key + "\n";
		}

		try {
			augs::create_directories_for(path);
			augs::save_as_text_atomic(path, content);
		}
		catch (const std::exception& e) {
			LOG("Could not save the known servers to %x: %x", path, e.what());
		}
	}

public:
	static constexpr std::string_view file_name = "known_servers.txt";

	explicit known_servers(const augs::path_type& path) : path(path) {
		load();
	}

	std::optional<augs::ed25519_public_key_type> find(const std::string& address) const {
		if (const auto it = key_by_address.find(address); it != key_by_address.end()) {
			return augs::hex_to_array<32>(it->second);
		}

		return std::nullopt;
	}

	/*
		Saved only when it changes - called every frame while connected.
	*/

	void set(const std::string& address, const augs::ed25519_public_key_type& key) {
		if (address.empty() || address.find_first_of(" \n") != std::string::npos) {
			return;
		}

		const auto hex = augs::bytes_to_hex(key);

		if (const auto it = key_by_address.find(address); it != key_by_address.end() && it->second == hex) {
			return;
		}

		key_by_address[address] = hex;
		save();
	}
};
