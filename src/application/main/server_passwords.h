#pragma once
#include <filesystem>
#include <map>
#include <system_error>
#include <optional>
#include <vector>
#include <string>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"
#include "augs/readwrite/json_readwrite.h"

/*
	The passwords the player entered, each saved for the public key of its server -
	so a password goes only to the server it was given for, sealed to that key, and never to another one.
	Kept in server_passwords.json.
*/

struct server_password_entry {
	// GEN INTROSPECTOR struct server_password_entry
	std::string key;
	std::string password;
	// END GEN INTROSPECTOR
};

class server_passwords {
	augs::path_type path;
	std::map<std::string, std::string> by_key;

	void load() {
		by_key.clear();

		try {
			for (const auto& e : augs::from_json_file<std::vector<server_password_entry>>(path)) {
				if (e.key.size() == 64) {
					by_key[e.key] = e.password;
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		auto entries = std::vector<server_password_entry>();

		for (const auto& [key, password] : by_key) {
			entries.push_back({ key, password });
		}

		try {
			augs::create_directories_for(path);
			augs::save_as_json_atomic(entries, path);

			/*
				Readable by this user only, like account.key.
			*/

			auto ignored = std::error_code();
			std::filesystem::permissions(path, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, std::filesystem::perm_options::replace, ignored);
		}
		catch (const std::exception& e) {
			LOG("Could not save the server passwords to %x: %x", path, e.what());
		}
	}

public:
	static constexpr std::string_view file_name = "server_passwords.json";

	explicit server_passwords(const augs::path_type& path) : path(path) {
		load();
	}

	std::string find(const augs::ed25519_public_key_type& server_key) const {
		if (const auto it = by_key.find(augs::bytes_to_hex(server_key)); it != by_key.end()) {
			return it->second;
		}

		return "";
	}

	void set(const augs::ed25519_public_key_type& server_key, const std::string& password) {
		/*
			Longer than a handshake carries (64 bytes): never saved - it would block the server for good.
		*/

		if (password.size() > 64) {
			return;
		}

		by_key[augs::bytes_to_hex(server_key)] = password;
		save();
	}
};
