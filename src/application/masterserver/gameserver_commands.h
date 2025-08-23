#pragma once
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

using gameserver_command = std::variant<
	gameserver_ping_request,
	int, // dummy for backwards compat
	masterserver_out::webrtc_signalling_payload
>;

struct gameserver_command_wrapper {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct gameserver_command_wrapper
	uint8_t marker = NETCODE_AUXILIARY_COMMAND_PACKET;
	gameserver_command payload;
	// END GEN INTROSPECTOR
};
