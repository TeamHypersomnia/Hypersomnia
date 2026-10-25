#pragma once
#include <cstddef>
#include <cstdint>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/masterserver/webrtc_signalling_payload.h"
#include "augs/network/port_type.h"

#define NETCODE_AUXILIARY_COMMAND_PACKET 200

struct gameserver_ping_request {
	uint64_t sequence = -1;
};

struct gameserver_ping_response {
	uint64_t sequence = -1;
};

/*
	Sent next to a ping only to a server the server list does not have (a favorite added by hand, one on
	the local network) - the server answers with its heartbeat, within its limits (see heartbeat_on_ping.h).
*/

struct gameserver_heartbeat_request {
	uint64_t sequence = -1;
};

/*
	The request is padded up to this many bytes and a shorter one is ignored - a heartbeat is then at most a few times
	bigger than what asked for it, so a spoofed request is worth little for flooding someone.
*/

constexpr std::size_t min_heartbeat_request_bytes_v = 256;

using gameserver_command = std::variant<
	gameserver_ping_request,
	int, // dummy for backwards compat
	masterserver_out::webrtc_signalling_payload,
	gameserver_heartbeat_request
>;

struct gameserver_command_wrapper {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct gameserver_command_wrapper
	uint8_t marker = NETCODE_AUXILIARY_COMMAND_PACKET;
	gameserver_command payload;
	// END GEN INTROSPECTOR
};
