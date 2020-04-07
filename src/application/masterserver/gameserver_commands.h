#pragma once
#include "3rdparty/yojimbo/netcode.io/netcode.h"
#include "application/masterserver/nat_traversal_step_payload.h"
#include "augs/network/port_type.h"

#define NETCODE_AUXILIARY_COMMAND_PACKET 200

namespace masterserver_in {
	/* This is what masterserver gets once client requests step */
	struct nat_traversal_step { 
		nat_traversal_step_payload payload;
		port_type port_opened_for_server = 0;
		netcode_address_t target_server;
	};

	/* This is what masterserver gets once server finishes resolution */
	struct stun_result_info { 
		nat_session_guid_type session_guid;
		netcode_address_t recipient_client;
		port_type resolved_external_port;
	};
}

namespace masterserver_out {
	/* This is what server gets once client requests step */
	struct nat_traversal_step { 
		nat_traversal_step_payload payload;
		netcode_address_t predicted_open_address;
		port_type masterserver_visible_client_port;
	};

	/* This is what client gets once server finishes resolution */
	struct stun_result_info { 
		nat_session_guid_type session_guid;
		port_type resolved_external_port;
	};
}

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
	masterserver_out::nat_traversal_step
>;

struct gameserver_command_wrapper {
	uint8_t marker = NETCODE_AUXILIARY_COMMAND_PACKET;
	gameserver_command payload;
};
