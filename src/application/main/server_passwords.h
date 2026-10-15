#pragma once
#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <vector>
#include <string>
#include <system_error>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/filesystem/private_file.h"
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
				const auto key = e.key.rfind("0x", 0) == 0 ? e.key.substr(2) : e.key;

				if (key.size() == 64) {
					by_key[key] = e.password;
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

		/*
			Readable by this user only, like account.key - written whole under a temporary name, then put in place.
		*/

		auto temporary = path;
		temporary += ".tmp-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

		try {
			augs::create_directories_for(path);
		}
		catch (...) {
		}

		auto ec = std::error_code();

		if (!augs::write_new_private_file(temporary, augs::to_json_string(entries))) {
			LOG("Could not save the server passwords to %x.", path);
			std::filesystem::remove(temporary, ec);
			return;
		}

		std::filesystem::rename(temporary, path, ec);

		if (ec) {
			LOG("Could not save the server passwords to %x: %x", path, ec.message());
			std::filesystem::remove(temporary, ec);
			return;
		}

		augs::sync_if_persistent(path);
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
