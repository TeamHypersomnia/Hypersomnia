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
		[ "discord_100000000000000000", "RCON", "MOD" ],
		[ "discord_200000000000000000", "MASTER_RCON" ]
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
