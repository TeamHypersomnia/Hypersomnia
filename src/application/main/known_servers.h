#pragma once
#include <cctype>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"
#include "augs/network/network_types.h"

/*
	The keys of the servers this game connected to, by the address it connected with - like SSH's known_hosts.
	A server joined by address later shows the same key, or the game warns before connecting (see the key mismatch
	in client_setup) - also while the masterserver, whose list has the keys, can not be reached.
	An official server's key from the server list (fetched over HTTPS) comes first; a community server's comes after this one -
	the list only repeats what the server itself said. Connect anyway replaces the key here.
	One line per address: the address, a space, the key in hex.
*/

class known_servers {
	augs::path_type path;
	std::map<std::string, std::string> key_by_address;

	std::string last_set_address;
	augs::ed25519_public_key_type last_set_key = {};

	/*
		One server written two ways is one entry: "Host.com" and "host.com:8412" alike.
	*/

	static std::string normalized(std::string address) {
		for (auto& c : address) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		if (address.find(':') == std::string::npos) {
			address += ":" + std::to_string(DEFAULT_GAME_PORT_V);
		}

		return address;
	}

	void load() {
		key_by_address.clear();

		try {
			auto in = std::istringstream(augs::file_to_string(path));
			std::string line;

			while (std::getline(in, line)) {
				/*
					Saved again by an editor on Windows.
				*/

				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}

				const auto space = line.rfind(' ');

				if (space != std::string::npos && space > 0 && line.size() - space - 1 == 64) {
					key_by_address[normalized(line.substr(0, space))] = line.substr(space + 1);
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
		if (const auto it = key_by_address.find(normalized(address)); it != key_by_address.end()) {
			return augs::hex_to_array<32>(it->second);
		}

		return std::nullopt;
	}

	/*
		Saved only when it changes - called every frame while connected.
		Read again first: another game running on this computer may have added its own servers since.
	*/

	void set(const std::string& written_address, const augs::ed25519_public_key_type& key) {
		if (written_address == last_set_address && key == last_set_key) {
			return;
		}

		last_set_address = written_address;
		last_set_key = key;

		if (written_address.empty() || written_address.find_first_of(" \r\n") != std::string::npos) {
			return;
		}

		const auto address = normalized(written_address);
		const auto hex = augs::bytes_to_hex(key);

		if (const auto it = key_by_address.find(address); it != key_by_address.end() && it->second == hex) {
			return;
		}

		load();
		key_by_address[address] = hex;
		save();
	}
};
