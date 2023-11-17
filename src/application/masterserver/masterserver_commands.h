#pragma once
#include "application/masterserver/server_heartbeat.h"

namespace masterserver_in {
	using heartbeat = server_heartbeat;

	struct tell_me_my_address { double session_timestamp = 0.0; };
	struct goodbye {};
}

namespace masterserver_out {
	struct tell_me_my_address { 
		double session_timestamp = 0.0;
		netcode_address_t address;
	};
}

