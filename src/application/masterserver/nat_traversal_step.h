#pragma once

#include "application/masterserver/nat_traversal_step_payload.h"
#include "augs/network/port_type.h"

struct masterserver_visible_client {
	netcode_address_t address;
	port_type probe = 0;
};

namespace masterserver_out {
	struct webrtc_signalling_payload;
}

namespace masterserver_in {
	/* This is what masterserver gets once client requests step */
	struct nat_traversal_step { 
		nat_traversal_step_payload payload;
		netcode_address_t target_server;
	};

	/* This is what masterserver gets once server finishes resolution */
	struct stun_result_info { 
		nat_session_guid_type session_guid;
		masterserver_visible_client client_origin;
		port_type resolved_external_port = 0;
	};

	using webrtc_signalling_payload = masterserver_out::webrtc_signalling_payload;
}

namespace masterserver_out {
	/* This is what server gets once client requests step */

	struct nat_traversal_step { 
		nat_traversal_step_payload payload;
		masterserver_visible_client client_origin;
	};

	/* This is what client gets once server finishes resolution */
	struct stun_result_info { 
		nat_session_guid_type session_guid;
		port_type resolved_external_port;
	};

	struct webrtc_signalling_payload {
		static constexpr bool force_read_field_by_field = true;

		// GEN INTROSPECTOR struct masterserver_out::webrtc_signalling_payload
		int64_t message_guid = 0;
		signalling_message_str message;
		// END GEN INTROSPECTOR
	};
}
