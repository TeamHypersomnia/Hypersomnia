#pragma once
#include <cctype>
#include <string>

/*
	An account id as a person may write it - a key's public key copied from the game is "0x<64 hex>" -
	in the form servers compare: "key_<64 hex>". Anything else stays as it is, trimmed.
*/

inline std::string normalize_account_id(const std::string& written) {
	const auto first = written.find_first_not_of(" \t\r\n");

	if (first == std::string::npos) {
		return std::string();
	}

	const auto last = written.find_last_not_of(" \t\r\n");
	const auto id = written.substr(first, last - first + 1);

	auto is_hex_key = [](const std::string& hex) {
		if (hex.size() != 64) {
			return false;
		}

		for (const auto c : hex) {
			if (!std::isxdigit(static_cast<unsigned char>(c))) {
				return false;
			}
		}

		return true;
	};

	auto lowercase = [](std::string text) {
		for (auto& c : text) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}

		return text;
	};

	if (id.size() == 66 && id[0] == '0' && (id[1] == 'x' || id[1] == 'X') && is_hex_key(id.substr(2))) {
		return "key_" + lowercase(id.substr(2));
	}

	if (is_hex_key(id)) {
		return "key_" + lowercase(id);
	}

	return id;
}
