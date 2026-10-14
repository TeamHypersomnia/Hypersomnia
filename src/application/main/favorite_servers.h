#pragma once
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "augs/string/hex.h"

/*
	The player's favorite servers, each by its public key - who the server is - with the name it had and the address
	it was last at: only a hint where to look when the server list does not have it (the masterserver down, the server
	listed elsewhere or not at all). In the order they were added.
	One line per server: the key in hex, a tab, the address, a tab, the name.
*/

struct favorite_server {
	augs::ed25519_public_key_type key = {};
	std::string address;
	std::string name;
};

class favorite_servers {
	augs::path_type path;
	std::vector<favorite_server> servers;

	void load() {
		servers.clear();

		try {
			auto in = std::istringstream(augs::file_to_string(path));
			std::string line;

			while (std::getline(in, line)) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}

				const auto first_tab = line.find('\t');
				const auto second_tab = first_tab == std::string::npos ? std::string::npos : line.find('\t', first_tab + 1);

				if (first_tab != 64 || second_tab == std::string::npos) {
					continue;
				}

				if (const auto key = augs::hex_to_array<32>(line.substr(0, 64))) {
					servers.push_back({ *key, line.substr(first_tab + 1, second_tab - first_tab - 1), line.substr(second_tab + 1) });
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		std::string content;

		for (const auto& s : servers) {
			content += augs::bytes_to_hex(s.key) + "\t" + s.address + "\t" + s.name + "\n";
		}

		try {
			augs::create_directories_for(path);
			augs::save_as_text_atomic(path, content);
		}
		catch (const std::exception& e) {
			LOG("Could not save the favorite servers to %x: %x", path, e.what());
		}
	}

	static std::string one_line(std::string s) {
		for (auto& c : s) {
			if (c == '\t' || c == '\n' || c == '\r') {
				c = ' ';
			}
		}

		return s;
	}

public:
	static constexpr std::string_view file_name = "favorite_servers.txt";

	explicit favorite_servers(const augs::path_type& path) : path(path) {
		load();
	}

	const std::vector<favorite_server>& all() const {
		return servers;
	}

	const favorite_server* find(const augs::ed25519_public_key_type& key) const {
		for (const auto& s : servers) {
			if (s.key == key) {
				return &s;
			}
		}

		return nullptr;
	}

	bool contains(const augs::ed25519_public_key_type& key) const {
		return find(key) != nullptr;
	}

	/*
		Added, or its name and address brought up to date (an empty one keeps what it was).
	*/

	void set(const augs::ed25519_public_key_type& key, const std::string& address, const std::string& name) {
		for (auto& s : servers) {
			if (s.key == key) {
				const auto new_address = address.empty() ? s.address : one_line(address);
				const auto new_name = name.empty() ? s.name : one_line(name);

				if (new_address != s.address || new_name != s.name) {
					s.address = new_address;
					s.name = new_name;
					save();
				}

				return;
			}
		}

		servers.push_back({ key, one_line(address), one_line(name) });
		save();
	}

	/*
		Only what is known already - a server not among the favorites is not added by being seen.
	*/

	void update_if_favorite(const augs::ed25519_public_key_type& key, const std::string& address, const std::string& name) {
		if (contains(key)) {
			set(key, address, name);
		}
	}

	void remove(const augs::ed25519_public_key_type& key) {
		std::erase_if(servers, [&](const auto& s) { return s.key == key; });
		save();
	}
};
