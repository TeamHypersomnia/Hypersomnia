#pragma once
#include <array>
#include <cstdint>
#include "augs/misc/hmac_sha256.h"

/*
	Proves that a request to the masterserver (a heartbeat or a goodbye)
	comes from an official server, i.e. one that knows its official_server_secret.

	key_id lets the masterserver find the secret in O(1) without the server having to know its own id.
	timestamp_ms only ever grows per server, so a captured request can not be replayed.
	mac is the HMAC of the whole serialized request with this mac zeroed.
*/

using official_server_key_id_type = std::array<uint8_t, 8>;

struct official_server_signature {
	// GEN INTROSPECTOR struct official_server_signature
	official_server_key_id_type key_id = {};
	uint64_t timestamp_ms = 0;
	augs::hmac_sha256_type mac = {};
	// END GEN INTROSPECTOR

	bool operator==(const official_server_signature&) const = default;
};
