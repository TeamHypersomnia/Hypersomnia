#pragma once
#include <optional>
#include <string>
#include <vector>
#include "application/setups/server/rcon_level.h"

/*
	A server admin recognized by the authenticated account id - see server_private_vars::admin_profiles.
	Written in the config as [ "account_id", "RCON level", "optional badge" ], e.g.:

	"admin_profiles": [
		[ "steam_76561198000000000", "NONE" ],
		[ "key_0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", "RCON", "MOD" ],
		[ "steam_76561198000000001", "MASTER_RCON" ]
	]

	The RCON level is one of NONE, RCON, MASTER_RCON.
	Regardless of it, the admin is shown with the [ADMIN] badge - or with the optional badge in its place.
*/

using admin_profiles_type = std::vector<std::vector<std::string>>;

struct admin_profile {
	std::string account_id;
	rcon_level_type rcon_level = rcon_level_type::DENIED;
	std::string badge;
};

inline bool is_valid_rcon_level_name(const std::string& level) {
	return level == "NONE" || level == "RCON" || level == "MASTER_RCON";
}

inline bool is_valid_admin_profile(const std::vector<std::string>& entry) {
	return entry.size() >= 2 && entry.size() <= 3 && !entry[0].empty() && ::is_valid_rcon_level_name(entry[1]);
}

inline std::optional<admin_profile> find_admin_profile(const admin_profiles_type& profiles, const std::string& account_id) {
	if (account_id.empty()) {
		return std::nullopt;
	}

	for (const auto& entry : profiles) {
		if (entry.empty() || entry[0] != account_id) {
			continue;
		}

		auto result = admin_profile();
		result.account_id = entry[0];

		if (entry.size() > 1) {
			const auto& level = entry[1];

			if (level == "MASTER_RCON") {
				result.rcon_level = rcon_level_type::MASTER;
			}
			else if (level == "RCON") {
				result.rcon_level = rcon_level_type::BASIC;
			}
		}

		if (entry.size() > 2) {
			result.badge = entry[2];
		}

		return result;
	}

	return std::nullopt;
}
