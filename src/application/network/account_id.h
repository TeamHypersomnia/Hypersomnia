#pragma once
#include <string>
#include <vector>
#include "application/network/identity_cert.h"
#include "application/main/identity_key.h"

/*
	An account id as a person may write it - a key's public key copied from the game is "0x<64 hex>" -
	in the form servers compare: "key_<64 hex>". Anything else stays as it is, trimmed.
*/

inline std::string normalize_account_id(const std::string& written) {
	const auto first = written.find_first_not_of(" \t\r\n");

	if (first == std::string::npos) {
		return std::string();
	}

	const auto id = written.substr(first, written.find_last_not_of(" \t\r\n") - first + 1);
	const auto key_text = identity_cert::is_key_account(id) ? id.substr(identity_cert::key_account_prefix.size()) : id;

	if (const auto key = identity_key::parse_public_key(key_text)) {
		return identity_cert::key_account(*key);
	}

	return id;
}

/*
	An account id as servers know them: a key's (key_<64 hex>, or 0x<64 hex>, or the bare 64 hex),
	steam_<17 digits>, crazygames_<id>.
*/

inline bool is_valid_account_id(const std::string& written) {
	const auto id = ::normalize_account_id(written);

	if (identity_cert::is_key_account(id)) {
		return identity_key::parse_public_key(id.substr(identity_cert::key_account_prefix.size())).has_value();
	}

	const auto steam_prefix = std::string("steam_");
	const auto crazygames_prefix = std::string("crazygames_");

	if (id.rfind(steam_prefix, 0) == 0) {
		return id.size() == steam_prefix.size() + 17 && id.find_first_not_of("0123456789", steam_prefix.size()) == std::string::npos;
	}

	if (id.rfind(crazygames_prefix, 0) == 0) {
		return id.size() > crazygames_prefix.size() && id.find_first_of(" \t\r\n=,;") == std::string::npos;
	}

	return false;
}

/*
	A server's allowed_accounts: empty - anyone; otherwise the listed ones, however each was written (0x... or key_...).
*/

inline bool is_allowed_account(const std::vector<std::string>& allowed, const std::string& account) {
	if (allowed.empty()) {
		return true;
	}

	const auto normalized = ::normalize_account_id(account);

	for (const auto& listed : allowed) {
		if (::normalize_account_id(listed) == normalized) {
			return true;
		}
	}

	return false;
}
