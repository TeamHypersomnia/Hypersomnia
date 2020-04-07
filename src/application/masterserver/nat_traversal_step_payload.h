#pragma once

using nat_session_guid_type = double;

enum class nat_traversal_step_type : uint8_t {
	RESTUN,
	PINGBACK
};

struct nat_traversal_step_payload {
	nat_session_guid_type session_guid;
	int source_port_delta = 0;

	nat_traversal_step_type type = nat_traversal_step_type::PINGBACK;
	bool ping_back_at_multiple_ports = false;
};
