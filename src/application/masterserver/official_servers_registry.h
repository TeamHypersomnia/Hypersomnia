#pragma once
#include <cstdint>
#include <optional>
#include <filesystem>
#include <string>
#include <system_error>
#include <unordered_map>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/secure_hash.h"
#include "application/main/identity_key.h"

/*
	The official servers, read from a directory where every file is named after a server id (e.g. "de", "us-central")
	and holds nothing but that server's public key - what its --print-server-key prints, or as the game shows it (0x...).
	The keys are public: nothing here needs to be kept secret, and the same directory serves hypersomnia.io.

	Adding a server is adding a file, revoking it is removing the file.
	The masterserver rescans the directory periodically, so neither needs a restart.
*/

class official_servers_registry {
	augs::path_type directory;
	std::unordered_map<std::string, std::string> id_by_key;

public:
	explicit official_servers_registry(augs::path_type directory) : directory(std::move(directory)) {
		reload();
	}

	/*
		A scan that fails (the directory gone or unreadable for a moment) keeps the previous list:
		emptying it would take every official server off ranked at once.
	*/

	void reload() {
		if (directory.empty()) {
			id_by_key.clear();
			return;
		}

		auto fresh = std::unordered_map<std::string, std::string>();
		std::error_code ec;
		auto duplicated = std::unordered_map<std::string, std::string>();

		for (const auto& file : std::filesystem::directory_iterator(directory, ec)) {
			if (!file.is_regular_file(ec)) {
				continue;
			}

			const auto server_id = file.path().filename().string();

			if (server_id.empty() || server_id[0] == '.' || server_id.back() == '~') {
				continue;
			}

			/*
				An unreadable file (wrong owner, removed mid-reload) must not take the masterserver down.
			*/

			const auto parsed = [&]() -> std::optional<augs::ed25519_public_key_type> {
				try {
					return identity_key::parse_public_key(augs::file_to_string(file.path()));
				}
				catch (const std::exception& err) {
					LOG("Official server %x: could not read the key: %x", server_id, err.what());
					return std::nullopt;
				}
			}();

			if (!parsed.has_value()) {
				LOG("Official server %x: not a public key (64 hex characters), skipping.", server_id);
				continue;
			}

			const auto key = augs::bytes_to_hex(*parsed);

			/*
				One key in two files: which id wins would depend on the directory order - neither is used.
			*/

			if (fresh.contains(key) || duplicated.contains(key)) {
				LOG("Official server %x: the same key as another one, skipping both.", server_id);
				duplicated.emplace(key, server_id);
				continue;
			}

			fresh.emplace(key, server_id);
		}

		for (const auto& d : duplicated) {
			fresh.erase(d.first);
		}

		if (ec) {
			LOG("Could not read the official servers from %x: %x. Keeping the previous %x.", directory, ec.message(), id_by_key.size());
			return;
		}

		id_by_key = std::move(fresh);
	}

	std::size_t size() const {
		return id_by_key.size();
	}

	/*
		The official server id of this key, or nullptr if it is no official server's.
	*/

	const std::string* find(const std::array<uint8_t, 32>& public_key) const {
		if (const auto it = id_by_key.find(augs::bytes_to_hex(public_key)); it != id_by_key.end()) {
			return std::addressof(it->second);
		}

		return nullptr;
	}
};
