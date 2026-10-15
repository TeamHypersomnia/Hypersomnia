#pragma once
#include <optional>
#include <string>
#include <vector>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/string/hex.h"
#include "augs/readwrite/json_readwrite.h"

/*
	The player's favorite servers, each by its public key - who the server is - in the order they were added.
	Two kinds:
	- starred on the server list: the key and the instance label (a host's instances share the key, see
	  server_instance_label.h), with the name and the address it had - only a hint where to look when the server list
	  does not have it; brought up to date from the list and on joining;
	- added by hand (Add an unlisted server, Connect - Add to favorites): the address and the key as the player gave
	  them, with the player's name - never matched against the server list nor brought up to date.
	Kept in favorite_servers.json.
*/

/*
	On CrazyGames the game's files do not outlive the tab (no IndexedDB there) - no favorites are shown, none would stay.
*/

#if WEB_CRAZYGAMES
inline constexpr bool favorites_kept = false;
#else
inline constexpr bool favorites_kept = true;
#endif

struct favorite_server_entry {
	// GEN INTROSPECTOR struct favorite_server_entry
	std::string key;
	std::string label;
	std::string address;
	std::string name;
	bool manual = false;
	// END GEN INTROSPECTOR
};

struct favorite_server {
	augs::ed25519_public_key_type key = {};
	std::string label;
	std::string address;
	std::string name;
	bool manual = false;
};

class favorite_servers {
	augs::path_type path;
	std::vector<favorite_server> servers;

	void load() {
		servers.clear();

		try {
			const auto entries = augs::from_json_file<std::vector<favorite_server_entry>>(path);

			for (const auto& e : entries) {
				auto hex = e.key;

				if (hex.rfind("0x", 0) == 0) {
					hex = hex.substr(2);
				}

				if (hex.size() != 64) {
					continue;
				}

				if (const auto key = augs::hex_to_array<32>(hex)) {
					servers.push_back({ *key, e.manual ? std::string() : e.label, e.address, e.name, e.manual });
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		auto entries = std::vector<favorite_server_entry>();

		for (const auto& s : servers) {
			entries.push_back({ "0x" + augs::bytes_to_hex(s.key), s.label, s.address, s.name, s.manual });
		}

		try {
			augs::create_directories_for(path);
			augs::save_as_json_atomic(entries, path);
		}
		catch (const std::exception& e) {
			LOG("Could not save the favorite servers to %x: %x", path, e.what());
		}
	}

public:
	static constexpr std::string_view file_name = "favorite_servers.json";

	explicit favorite_servers(const augs::path_type& path) : path(path) {
		load();
	}

	const std::vector<favorite_server>& all() const {
		return servers;
	}

	/*
		The one starred on the server list: that instance of the host.
	*/

	const favorite_server* find(const augs::ed25519_public_key_type& key, const std::string& label) const {
		for (const auto& s : servers) {
			if (!s.manual && s.key == key && s.label == label) {
				return &s;
			}
		}

		return nullptr;
	}

	/*
		Any with this key, the first added - where a join by the key alone looks when the server list does not have it.
	*/

	const favorite_server* find_any(const augs::ed25519_public_key_type& key) const {
		for (const auto& s : servers) {
			if (s.key == key && !s.address.empty()) {
				return &s;
			}
		}

		return nullptr;
	}

	bool contains(const augs::ed25519_public_key_type& key, const std::string& label) const {
		return find(key, label) != nullptr;
	}

	bool contains_manual(const augs::ed25519_public_key_type& key, const std::string& address) const {
		for (const auto& s : servers) {
			if (s.manual && s.key == key && s.address == address) {
				return true;
			}
		}

		return false;
	}

	/*
		Starred on the server list: added, or its name and address brought up to date (an empty one keeps what it was).
	*/

	void set(
		const augs::ed25519_public_key_type& key,
		const std::string& label,
		const std::string& address,
		const std::string& name
	) {
		for (auto& s : servers) {
			if (!s.manual && s.key == key && s.label == label) {
				const auto new_address = address.empty() ? s.address : address;
				const auto new_name = name.empty() ? s.name : name;

				if (new_address != s.address || new_name != s.name) {
					s.address = new_address;
					s.name = new_name;
					save();
				}

				return;
			}
		}

		servers.push_back({ key, label, address, name, false });
		save();
	}

	/*
		Added by hand - the name the address if none given. The same address and key again only renames it.
	*/

	void add_manual(const augs::ed25519_public_key_type& key, const std::string& address, const std::string& name) {
		const auto shown_name = name.empty() ? address : name;

		for (auto& s : servers) {
			if (s.manual && s.key == key && s.address == address) {
				s.name = shown_name;
				save();
				return;
			}
		}

		servers.push_back({ key, std::string(), address, shown_name, true });
		save();
	}

	/*
		Only what is known already - a server not among the favorites is not added by being seen,
		and one added by hand is never changed.
	*/

	void update_if_favorite(const augs::ed25519_public_key_type& key, const std::string& label, const std::string& address, const std::string& name) {
		if (contains(key, label)) {
			set(key, label, address, name);
		}
	}

	/*
		One added by hand, changed in place (Edit in Server details) - the name the address if none given.
		Changed to the address and key of another one added by hand: the two become one.
	*/

	void edit_manual(
		const augs::ed25519_public_key_type& old_key,
		const std::string& old_address,
		const augs::ed25519_public_key_type& key,
		const std::string& address,
		const std::string& name
	) {
		const auto is_old = [&](const auto& s) { return s.manual && s.key == old_key && s.address == old_address; };
		const bool same_place = old_key == key && old_address == address;

		if (!same_place && contains_manual(key, address)) {
			std::erase_if(servers, is_old);
			add_manual(key, address, name);
			return;
		}

		for (auto& s : servers) {
			if (is_old(s)) {
				s.key = key;
				s.address = address;
				s.name = name.empty() ? address : name;
				save();
				return;
			}
		}
	}

	void remove(const augs::ed25519_public_key_type& key, const std::string& label) {
		std::erase_if(servers, [&](const auto& s) { return !s.manual && s.key == key && s.label == label; });
		save();
	}

	void remove_manual(const augs::ed25519_public_key_type& key, const std::string& address) {
		std::erase_if(servers, [&](const auto& s) { return s.manual && s.key == key && s.address == address; });
		save();
	}
};
