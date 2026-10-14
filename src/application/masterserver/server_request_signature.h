#pragma once
#include <array>
#include <cstdint>

/*
	Every request a server sends the masterserver (a heartbeat or a goodbye) is signed with its key (server.key).

	cookie is what the masterserver last answered this address with - a MAC of the address and the time - so a request
	is accepted only from the address it was made at and only for a while: a recorded one can neither be sent from
	elsewhere nor later. A server that has no fresh cookie yet gets one in the answer and sends again.
*/

using masterserver_cookie_type = std::array<uint8_t, 24>;

struct server_request_signature {
	// GEN INTROSPECTOR struct server_request_signature
	std::array<uint8_t, 32> server_public_key = {};
	masterserver_cookie_type cookie = {};
	std::array<uint8_t, 64> signature = {};
	// END GEN INTROSPECTOR

	bool operator==(const server_request_signature&) const = default;
};
