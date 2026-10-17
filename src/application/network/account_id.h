#pragma once
#include <string>
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
