#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include "augs/log.h"
#include "augs/filesystem/file.h"
#include "augs/misc/ed25519.h"
#include "augs/misc/date_time.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/main/identity_key.h"

/*
	user/ranked_match.json - a ranked match the player is in, kept so that the game brings the player back to it on its own
	after a crash or a restart - to that server only, expecting its key.

	rejoin_until is rewritten during the match as now + what is left of the player's rejoin budget,
	so after a crash at any moment it is right within the rewrite period. Past it a rejoin is refused anyway -
	a stale file does nothing.
*/

/*
	The kick reasons that refuse a rejoin for good (the match ended, or the record was another account's) - the server
	kicks with them, and the game forgets its record on seeing one.
*/

inline constexpr std::string_view rejoin_too_late_reason_v = "Joined too late! Match is ongoing.";
inline constexpr std::string_view rejoin_wrong_match_reason_v = "Connected to a wrong match.";

inline bool refuses_rejoin(const std::string& kick_reason) {
	return
		kick_reason.find(rejoin_too_late_reason_v) != std::string::npos
		|| kick_reason.find(rejoin_wrong_match_reason_v) != std::string::npos
	;
}

/*
	The file's form - the server key as identity_key::full_form.
*/

struct ranked_match_record_file {
	// GEN INTROSPECTOR struct ranked_match_record_file
	std::string connect;
	std::string server_key;
	double rejoin_until = 0.0;
	// END GEN INTROSPECTOR
};

struct ranked_match_record {
	std::string connect;
	augs::ed25519_public_key_type server_key = {};
	int64_t rejoin_until = 0;

	bool valid_now() const {
		return static_cast<int64_t>(augs::date_time::secs_since_epoch()) < rejoin_until;
	}

	static std::optional<ranked_match_record> load(const augs::path_type& path) {
		if (!augs::exists(path)) {
			return std::nullopt;
		}

		try {
			const auto file = augs::from_json_file<ranked_match_record_file>(path);
			const auto key = identity_key::parse_public_key(file.server_key);

			if (file.connect.empty() || !key.has_value()) {
				return std::nullopt;
			}

			return ranked_match_record { file.connect, *key, static_cast<int64_t>(file.rejoin_until) };
		}
		catch (...) {
			LOG("Could not read %x.", path);
			return std::nullopt;
		}
	}

	void save(const augs::path_type& path) const {
		try {
			augs::save_as_json_atomic(
				ranked_match_record_file { connect, identity_key::full_form(server_key), static_cast<double>(rejoin_until) },
				path
			);
		}
		catch (...) {
			LOG("Could not save %x.", path);
		}
	}
};
