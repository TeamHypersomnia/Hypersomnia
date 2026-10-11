#pragma once
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <sodium.h>
#include "augs/misc/ed25519.h"
#include "augs/readwrite/to_bytes.h"
#include "application/masterserver/masterserver_commands.h"

/*
	See server_request_signature.h. The signature covers the request serialized as a whole masterserver_request,
	tagged "hs-hb-v1" - so its variant index tells a heartbeat from a goodbye and neither passes for the other -
	with the signature itself zeroed.
*/

/*
	For the logs of requests anyone can send: at most one line every 10 seconds per call site,
	so that a flood of them does not fill the disk.
*/

inline bool official_log_allowed(uint64_t& last_logged_ms) {
	const auto now = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

	if (now - last_logged_ms < 10000 && last_logged_ms != 0) {
		return false;
	}

	last_logged_ms = now;
	return true;
}

template <class Request>
std::string server_request_signed_message(Request request) {
	if (request.signature.has_value()) {
		request.signature->signature = {};
	}

	const auto bytes = augs::to_bytes(masterserver_request(std::move(request)));

	auto out = std::string("hs-hb-v1");
	out.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	return out;
}

template <class Request>
bool sign_server_request(Request& request, const augs::ed25519_seed_type& server_key, const masterserver_cookie_type& cookie, const uint64_t nonce) {
	const auto public_key = augs::ed25519_public_key(server_key);

	if (!public_key.has_value()) {
		request.signature.reset();
		return false;
	}

	request.signature = server_request_signature { *public_key, cookie, nonce, {} };

	if (const auto signature = augs::ed25519_sign(server_key, ::server_request_signed_message(request))) {
		request.signature->signature = *signature;
		return true;
	}

	request.signature.reset();
	return false;
}

/*
	Only that the key signed it - whether the cookie is fresh and for this address is the masterserver's to check.
*/

template <class Request>
bool server_request_signature_valid(const Request& request) {
	if (!request.signature.has_value()) {
		return false;
	}

	return augs::ed25519_verify(request.signature->server_public_key, ::server_request_signed_message(request), request.signature->signature);
}

/*
	The masterserver's cookies: 8 bytes of its steady time in milliseconds and a 16-byte keyed BLAKE2b of the address and that time.
	The key is random per run, so nothing to keep or leak.
*/

class masterserver_cookies {
	std::array<uint8_t, 32> key = {};

	static uint64_t now_ms() {
		return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
	}

	void mac(uint8_t* out, const netcode_address_t& address, const uint64_t timestamp) const {
		uint8_t data[64];
		std::size_t n = 0;

		data[n++] = address.type;
		data[n++] = static_cast<uint8_t>(address.port & 0xff);
		data[n++] = static_cast<uint8_t>(address.port >> 8);

		if (address.type == NETCODE_ADDRESS_IPV4) {
			std::memcpy(data + n, address.data.ipv4, 4);
			n += 4;
		}
		else {
			std::memcpy(data + n, address.data.ipv6, 16);
			n += 16;
		}

		for (int i = 0; i < 8; ++i) {
			data[n++] = static_cast<uint8_t>((timestamp >> (8 * i)) & 0xff);
		}

		crypto_generichash(out, 16, data, n, key.data(), key.size());
	}

public:
	static constexpr uint64_t max_age_ms = 30000;

	masterserver_cookies() {
		randombytes_buf(key.data(), key.size());
	}

	masterserver_cookie_type make(const netcode_address_t& address) const {
		auto out = masterserver_cookie_type();
		const auto timestamp = now_ms();

		for (int i = 0; i < 8; ++i) {
			out[i] = static_cast<uint8_t>((timestamp >> (8 * i)) & 0xff);
		}

		mac(out.data() + 8, address, timestamp);
		return out;
	}

	bool valid(const masterserver_cookie_type& cookie, const netcode_address_t& address) const {
		uint64_t timestamp = 0;

		for (int i = 0; i < 8; ++i) {
			timestamp |= static_cast<uint64_t>(cookie[i]) << (8 * i);
		}

		const auto now = now_ms();

		if (timestamp > now || now - timestamp > max_age_ms) {
			return false;
		}

		uint8_t expected[16];
		mac(expected, address, timestamp);

		return sodium_memcmp(expected, cookie.data() + 8, 16) == 0;
	}
};
