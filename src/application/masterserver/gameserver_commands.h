#pragma once
#include <cstdint>
#include "3rdparty/yojimbo/netcode/netcode.h"
#include "application/masterserver/nat_traversal_step.h"

#define NETCODE_AUXILIARY_COMMAND_PACKET 200

struct gameserver_ping_request {
	uint64_t sequence = -1;
};

struct gameserver_ping_response {
	uint64_t sequence = -1;
};

struct gameserver_nat_traversal_response_packet {
	uint64_t magic_cookie = 0xad9b310678ed11ea;
	double session_guid = 0;

	bool valid() const {
		gameserver_nat_traversal_response_packet g;
		return magic_cookie == g.magic_cookie;
	}
};

using gameserver_command = std::variant<
	gameserver_ping_request,
	masterserver_out::nat_traversal_step,
	masterserver_out::webrtc_signalling_payload
>;

struct gameserver_command_wrapper {
	static constexpr bool force_read_field_by_field = true;

	// GEN INTROSPECTOR struct gameserver_command_wrapper
	uint8_t marker = NETCODE_AUXILIARY_COMMAND_PACKET;
	gameserver_command payload;
	// END GEN INTROSPECTOR
};
