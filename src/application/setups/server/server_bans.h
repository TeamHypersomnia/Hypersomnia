#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "application/network/account_id.h"

/*
	Bans of one server, kept in server_bans.json in its user folder, so they survive restarts
	and can be edited by hand. A ban holds the account and the address of the player:
	a new key gets around the first, a new address around the second.
*/

struct server_ban {
	// GEN INTROSPECTOR struct server_ban
	std::string account;
	std::string ip;
	std::string nickname;
	std::string reason;
	uint64_t until_unix = 0;
	// END GEN INTROSPECTOR

	bool is_forever() const {
		return until_unix == 0;
	}

	bool active_at(const uint64_t now) const {
		return is_forever() || now < until_unix;
	}

	bool operator==(const server_ban&) const = default;
};

struct server_bans {
	// GEN INTROSPECTOR struct server_bans
	std::vector<server_ban> bans;
	// END GEN INTROSPECTOR

	const server_ban* find_active(const std::string& account, const std::string& ip, const uint64_t now) const {
		for (const auto& b : bans) {
			if (!b.active_at(now)) {
				continue;
			}

			if ((!account.empty() && ::normalize_account_id(b.account) == account) || (!ip.empty() && b.ip == ip)) {
				return &b;
			}
		}

		return nullptr;
	}

	/*
		Drops the expired ones, so that the file does not grow without end.
	*/

	bool remove_expired(const uint64_t now) {
		const auto before = bans.size();

		std::erase_if(bans, [now](const auto& b) { return !b.active_at(now); });

		return bans.size() != before;
	}

	bool operator==(const server_bans&) const = default;
};
