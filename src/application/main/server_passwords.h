#pragma once
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/secure_hash.h"

/*
	The passwords the player entered, each saved for the public key of its server -
	so a password goes only to the server it was given for, sealed to that key, and never to another one.
	One line per server: the key in hex, a space, the password.
*/

class server_passwords {
	augs::path_type path;
	std::map<std::string, std::string> by_key;

	void load() {
		by_key.clear();

		try {
			auto in = std::istringstream(augs::file_to_string(path));
			std::string line;

			while (std::getline(in, line)) {
				const auto space = line.find(' ');

				if (space == 64) {
					by_key[line.substr(0, space)] = line.substr(space + 1);
				}
			}
		}
		catch (...) {
		}
	}

	void save() const {
		std::string content;

		for (const auto& [key, password] : by_key) {
			content += key + " " + password + "\n";
		}

		try {
			augs::create_directories_for(path);
			augs::save_as_text_atomic(path, content);
		}
		catch (const std::exception& e) {
			LOG("Could not save the server passwords to %x: %x", path, e.what());
		}
	}

public:
	static constexpr std::string_view file_name = "server_passwords.txt";

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

		if (password.find('\n') != std::string::npos) {
			return;
		}

		by_key[augs::bytes_to_hex(server_key)] = password;
		save();
	}
};
