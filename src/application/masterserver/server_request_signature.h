#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <sodium.h>

/*
	Every request a server sends the masterserver (a heartbeat or a goodbye) is signed with its key (server.key).

	cookie is what the masterserver last answered this address with - a MAC of the address and the time - so a request
	is accepted only from the address it was made at and only for a while: a recorded one can neither be sent from
	elsewhere nor later. A server that has no fresh cookie yet gets one in the answer and sends again.
*/

using masterserver_cookie_type = std::array<uint8_t, 24>;

/*
	A heartbeat's random nonce, sent back in its answer (see masterserver_out::heartbeat_ack).
*/

using heartbeat_nonce_type = std::array<uint8_t, 8>;

/*
	A server in the browser has no cookie: it signs its heartbeats with its WebSocket session's id in its place,
	so a native server's signed heartbeat can not be passed off as one of a browser's session (a clone on the list).
*/

inline masterserver_cookie_type web_session_cookie(const std::string& webrtc_id) {
	auto out = masterserver_cookie_type();

	/*
		A hash of the whole id - two ids with the same beginning never share it.
	*/

	const auto tag = std::string("hs-ws:") + webrtc_id;

	crypto_generichash(
		out.data(),
		out.size(),
		reinterpret_cast<const unsigned char*>(tag.data()),
		tag.size(),
		nullptr,
		0
	);

	return out;
}

struct server_request_signature {
	// GEN INTROSPECTOR struct server_request_signature
	std::array<uint8_t, 32> server_public_key = {};
	masterserver_cookie_type cookie = {};
	std::array<uint8_t, 64> signature = {};
	// END GEN INTROSPECTOR

	bool operator==(const server_request_signature&) const = default;
};
