#pragma once
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <unordered_map>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "application/masterserver/official_server_signing.h"

/*
	The official servers, read from a directory where every file is named after a server id (e.g. "de", "us-central")
	and holds nothing but that server's official_server_secret.

	Adding a server is adding a file, revoking it is removing the file.
	The masterserver rescans the directory periodically, so neither needs a restart.
*/

struct official_server_entry {
	std::string server_id;
	official_server_keys keys;
	uint64_t last_timestamp_ms = 0;
};

class official_servers_registry {
	augs::path_type directory;
	std::unordered_map<uint64_t, official_server_entry> by_key_id;

	static uint64_t to_map_key(const official_server_key_id_type& key_id) {
		static_assert(sizeof(uint64_t) == sizeof(official_server_key_id_type));

		uint64_t out = 0;
		std::memcpy(&out, key_id.data(), sizeof(out));
		return out;
	}

	static std::string trimmed(std::string s) {
		auto is_space = [](const char c) {
			return c == ' ' || c == '\t' || c == '\r' || c == '\n';
		};

		while (!s.empty() && is_space(s.back())) {
			s.pop_back();
		}

		std::size_t first = 0;

		while (first < s.size() && is_space(s[first])) {
			++first;
		}

		return s.substr(first);
	}

public:
	/*
		A request signed within this many milliseconds of the masterserver's clock is accepted,
		which tolerates moderate clock drift of the hosts.
	*/

	static constexpr uint64_t max_clock_difference_ms = 30000;

	explicit official_servers_registry(augs::path_type directory) : directory(std::move(directory)) {
		reload();
	}

	void reload() {
		auto previous = std::move(by_key_id);
		by_key_id.clear();

		if (directory.empty()) {
			return;
		}

		std::error_code ec;

		/*
			A secret shared by two files would let the tokens, tickets and signatures for one server
			pass on the other, and which file wins would depend on the directory order - neither is used.
		*/

		auto duplicated_keys = std::unordered_map<uint64_t, std::string>();

		/*
			A new entry accepts only the timestamps from now on, so a signed request recorded
			before the masterserver started can not be replayed after it.
		*/

		const auto load_time_ms = ::official_signature_now_ms();

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

			const auto secret = [&]() -> std::string {
				try {
					return trimmed(augs::file_to_string(file.path()));
				}
				catch (const std::exception& err) {
					LOG("Official server %x: could not read the secret: %x", server_id, err.what());
					return std::string();
				}
			}();
			const auto keys = official_server_keys::derive(secret);

			if (!keys.has_value()) {
				LOG("Official server %x: empty secret or OpenSSL failure, skipping.", server_id);
				continue;
			}

			const auto key = to_map_key(keys->key_id);

			if (by_key_id.contains(key) || duplicated_keys.contains(key)) {
				LOG("Official server %x: the same secret as another one, skipping both.", server_id);
				duplicated_keys.emplace(key, server_id);
				continue;
			}

			auto entry = official_server_entry { server_id, *keys, load_time_ms };

			if (const auto it = previous.find(key); it != previous.end()) {
				entry.last_timestamp_ms = it->second.last_timestamp_ms;
			}

			by_key_id.emplace(key, std::move(entry));
		}

		for (const auto& duplicated : duplicated_keys) {
			if (const auto it = by_key_id.find(duplicated.first); it != by_key_id.end()) {
				LOG("Official server %x: the same secret as another one, skipping both.", it->second.server_id);
				by_key_id.erase(it);
			}
		}

		if (ec) {
			LOG("Could not read the official servers from %x: %x", directory, ec.message());
		}
	}

	std::size_t size() const {
		return by_key_id.size();
	}

	const official_server_entry* find_by_server_id(const std::string& server_id) const {
		for (const auto& entry : by_key_id) {
			if (entry.second.server_id == server_id) {
				return std::addressof(entry.second);
			}
		}

		return nullptr;
	}

	/*
		The official server that has signed this request, or nullptr if the request is unsigned,
		signed with an unknown secret, forged, too old, from too far in the future, or replayed.
	*/

	template <class Request>
	const official_server_entry* authenticate(const Request& request, const uint64_t now_ms, const std::string& from) {
		if (!request.signature.has_value()) {
			return nullptr;
		}

		const auto& signature = *request.signature;
		const auto it = by_key_id.find(to_map_key(signature.key_id));

		if (it == by_key_id.end()) {
			static uint64_t last_logged_ms = 0;

			if (::official_log_allowed(last_logged_ms)) {
				LOG("Rejected a signed request from %x: unknown key id.", from);
			}
			return nullptr;
		}

		auto& entry = it->second;

		if (!::official_request_mac_matches(request, entry.keys)) {
			LOG("Rejected a request from %x signed as %x: wrong MAC.", from, entry.server_id);
			return nullptr;
		}

		const auto ts = signature.timestamp_ms;

		if (ts + max_clock_difference_ms < now_ms || ts > now_ms + max_clock_difference_ms) {
			LOG(
				"Rejected a request from %x signed as %x: its clock is off by %x ms. Synchronize the time on that host.",
				from,
				entry.server_id,
				static_cast<double>(ts) - static_cast<double>(now_ms)
			);

			return nullptr;
		}

		if (ts <= entry.last_timestamp_ms) {
			LOG("Rejected a request from %x signed as %x: replayed (timestamp not newer than the last one).", from, entry.server_id);
			return nullptr;
		}

		entry.last_timestamp_ms = ts;
		return std::addressof(entry);
	}
};
