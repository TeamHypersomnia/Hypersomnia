#pragma once
#include <optional>
#include <string>
#include <vector>
#include "application/setups/server/rcon_level.h"

/*
	A server admin recognized by the authenticated account id - see server_private_vars::admin_profiles, e.g.:

	"admin_profiles": [
		{ "account": "steam_76561198000000000", "rcon": "MASTER_RCON" },
		{ "account": "key_0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef", "rcon": "RCON", "badge": "MOD" },
		{ "account": "steam_76561198000000001" }
	]

	rcon is one of NONE (default), RCON, MASTER_RCON.
	Regardless of it, the admin is shown with the [ADMIN] badge - or with the optional badge in its place.
*/

struct admin_profile_entry {
	// GEN INTROSPECTOR struct admin_profile_entry
	std::string account;
	std::string rcon = "NONE";
	std::string badge;
	// END GEN INTROSPECTOR

	bool operator==(const admin_profile_entry&) const = default;
};

using admin_profiles_type = std::vector<admin_profile_entry>;

struct admin_profile {
	std::string account_id;
	rcon_level_type rcon_level = rcon_level_type::DENIED;
	std::string badge;
};

inline bool is_valid_rcon_level_name(const std::string& level) {
	return level == "NONE" || level == "RCON" || level == "MASTER_RCON";
}

inline bool is_valid_admin_profile(const admin_profile_entry& entry) {
	return !entry.account.empty() && ::is_valid_rcon_level_name(entry.rcon);
}

inline std::optional<admin_profile> find_admin_profile(const admin_profiles_type& profiles, const std::string& account_id) {
	if (account_id.empty()) {
		return std::nullopt;
	}

	for (const auto& entry : profiles) {
		if (entry.account != account_id) {
			continue;
		}

		auto result = admin_profile();
		result.account_id = entry.account;

		if (entry.rcon == "MASTER_RCON") {
			result.rcon_level = rcon_level_type::MASTER;
		}
		else if (entry.rcon == "RCON") {
			result.rcon_level = rcon_level_type::BASIC;
		}

		result.badge = entry.badge;
		return result;
	}

	return std::nullopt;
}
