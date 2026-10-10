#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/masterserver_commands.h"
#include "application/masterserver/official_server_keys.h"

/*
	The MAC covers the request serialized as a whole masterserver_request,
	so its variant index tells a heartbeat from a goodbye and neither can be passed off as the other.
*/

inline uint64_t official_signature_now_ms() {
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

/*
	The masterserver only accepts a newer timestamp than the last one,
	and a server can send several requests within the same millisecond.
*/

inline uint64_t next_official_signature_timestamp_ms() {
	static std::atomic<uint64_t> last = 0;

	auto previous = last.load();

	while (true) {
		const auto next = std::max(::official_signature_now_ms(), previous + 1);

		if (last.compare_exchange_weak(previous, next)) {
			return next;
		}
	}
}

template <class Request>
std::optional<augs::hmac_sha256_type> official_request_mac(Request request, const augs::hmac_sha256_type& heartbeat_key) {
	if (!request.signature.has_value()) {
		return std::nullopt;
	}

	request.signature->mac = {};

	const auto bytes = augs::to_bytes(masterserver_request(std::move(request)));
	return augs::hmac_sha256(heartbeat_key, bytes);
}

template <class Request>
bool sign_official_request(Request& request, const official_server_keys& keys, const uint64_t timestamp_ms) {
	request.signature = official_server_signature { keys.key_id, timestamp_ms, {} };

	if (const auto mac = ::official_request_mac(request, keys.heartbeat_key); mac.has_value()) {
		request.signature->mac = *mac;
		return true;
	}

	request.signature.reset();
	return false;
}

template <class Request>
bool official_request_mac_matches(const Request& request, const official_server_keys& keys) {
	if (!request.signature.has_value()) {
		return false;
	}

	if (const auto expected = ::official_request_mac(request, keys.heartbeat_key); expected.has_value()) {
		return augs::constant_time_equal(*expected, request.signature->mac);
	}

	return false;
}
