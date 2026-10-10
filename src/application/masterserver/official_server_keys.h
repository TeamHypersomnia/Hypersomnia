#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "augs/misc/hmac_sha256.h"
#include "application/masterserver/official_server_signature.h"

/*
	Everything an official server proves itself with is derived from its single official_server_secret,
	each with its own context string, so that no derived key reveals the secret or any other derived key.

	The secret itself is still sent as is to report ranked matches.
*/

struct official_server_keys {
	augs::hmac_sha256_type netcode_key = {};
	augs::hmac_sha256_type heartbeat_key = {};
	official_server_key_id_type key_id = {};

	static std::optional<official_server_keys> derive(const std::string& secret) {
		if (secret.empty()) {
			return std::nullopt;
		}

		auto derive_one = [&](const std::string_view context) {
			return augs::hmac_sha256(secret, context);
		};

		const auto netcode_key = derive_one("hypersomnia-netcode-key-v1");
		const auto heartbeat_key = derive_one("hypersomnia-heartbeat-v1");
		const auto key_id = derive_one("hypersomnia-key-id-v1");

		if (!netcode_key.has_value() || !heartbeat_key.has_value() || !key_id.has_value()) {
			return std::nullopt;
		}

		auto out = official_server_keys();

		out.netcode_key = *netcode_key;
		out.heartbeat_key = *heartbeat_key;

		for (std::size_t i = 0; i < out.key_id.size(); ++i) {
			out.key_id[i] = (*key_id)[i];
		}

		return out;
	}
};
