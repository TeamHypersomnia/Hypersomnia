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
	The player's favorite servers, each by its public key - who the server is - and its instance label (a host's
	instances share the key, see server_instance_label.h), with the name it had and the address it was last at: only a
	hint where to look when the server list does not have it (the masterserver down, the server listed elsewhere or not
	at all). In the order they were added.
	One line per server: the key in hex, a tab, the label, a tab, the address, a tab, the name - and a tab and "named" when
	the player gave the name (Add a server): that one is kept, never brought up to date from the list or the server.
*/

/*
	On CrazyGames the game's files do not outlive the tab (no IndexedDB there) - no favorites are shown, none would stay.
*/

#if WEB_CRAZYGAMES
inline constexpr bool favorites_kept = false;
#else
inline constexpr bool favorites_kept = true;
#endif

struct favorite_server {
	augs::ed25519_public_key_type key = {};
	std::string label;
	std::string address;
	std::string name;
	bool named_by_player = false;
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

				auto fields = std::vector<std::string>();
				std::size_t from = 0;

				while (fields.size() < 3) {
					const auto tab = line.find('\t', from);

					if (tab == std::string::npos) {
						break;
					}

					fields.push_back(line.substr(from, tab - from));
					from = tab + 1;
				}

				if (fields.size() != 3 || fields[0].size() != 64) {
					continue;
				}

				if (const auto key = augs::hex_to_array<32>(fields[0])) {
					auto name = line.substr(from);
					const auto marker = std::string("\tnamed");
					const bool named = name.size() >= marker.size() && name.compare(name.size() - marker.size(), marker.size(), marker) == 0;

					if (named) {
						name.resize(name.size() - marker.size());
					}

					servers.push_back({ *key, fields[1], fields[2], name, named });
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		std::string content;

		for (const auto& s : servers) {
			content += augs::bytes_to_hex(s.key) + "\t" + s.label + "\t" + s.address + "\t" + s.name + (s.named_by_player ? "\tnamed" : "") + "\n";
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

	/*
		That instance, else a favorite of the host whose instance is not known yet (added by hand with an address,
		no label) - a label not known here (e.g. a join by the key alone) takes the host's first favorite.
	*/

	const favorite_server* find(const augs::ed25519_public_key_type& key, const std::string& label = "") const {
		const favorite_server* unlabeled = nullptr;

		for (const auto& s : servers) {
			if (s.key != key) {
				continue;
			}

			if (label.empty() || s.label == label) {
				return &s;
			}

			if (s.label.empty() && unlabeled == nullptr) {
				unlabeled = &s;
			}
		}

		return unlabeled;
	}

	bool contains(const augs::ed25519_public_key_type& key, const std::string& label) const {
		return find(key, label) != nullptr;
	}

	/*
		Added, or its name and address brought up to date (an empty one keeps what it was).
	*/

	void set(
		const augs::ed25519_public_key_type& key,
		const std::string& label,
		const std::string& address,
		const std::string& name,
		const bool named_by_player = false
	) {
		for (auto& s : servers) {
			if (s.key == key && s.label == label) {
				const auto new_address = address.empty() ? s.address : one_line(address);
				const bool keep_name = name.empty() || (s.named_by_player && !named_by_player);
				const auto new_name = keep_name ? s.name : one_line(name);
				const bool new_named = s.named_by_player || named_by_player;

				if (new_address != s.address || new_name != s.name || new_named != s.named_by_player) {
					s.address = new_address;
					s.name = new_name;
					s.named_by_player = new_named;
					save();
				}

				return;
			}
		}

		servers.push_back({ key, one_line(label), one_line(address), one_line(name), named_by_player });
		save();
	}

	/*
		Only what is known already - a server not among the favorites is not added by being seen.
	*/

	void update_if_favorite(const augs::ed25519_public_key_type& key, const std::string& label, const std::string& address, const std::string& name) {
		const auto found = find(key, label);

		if (found == nullptr) {
			return;
		}

		/*
			A favorite of the host learns its instance on joining.
		*/

		auto found_label = found->label;

		if (found_label.empty() && !label.empty()) {
			for (auto& s : servers) {
				if (&s == found) {
					s.label = one_line(label);
				}
			}

			found_label = one_line(label);
			save();
		}

		set(key, found_label, address, name);
	}

	void remove(const augs::ed25519_public_key_type& key, const std::string& label) {
		std::erase_if(servers, [&](const auto& s) { return s.key == key && (s.label == label || s.label.empty()); });
		save();
	}
};
