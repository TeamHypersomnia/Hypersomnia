#pragma once
#include "application/masterserver/server_heartbeat.h"
#include "application/masterserver/webrtc_signalling_payload.h"

namespace masterserver_in {
	using heartbeat = server_heartbeat;

	struct tell_me_my_address { double session_timestamp = 0.0; };

	struct goodbye {
		// GEN INTROSPECTOR struct masterserver_in::goodbye
		std::optional<server_request_signature> signature;
		// END GEN INTROSPECTOR
	};
}

namespace masterserver_out {
	struct tell_me_my_address { 
		double session_timestamp = 0.0;
		netcode_address_t address;
	};

	/*
		The answer to every heartbeat: a cookie for the next request (see server_request_signature.h)
		and whether the masterserver knows this server's key as an official one.
	*/

	struct heartbeat_ack {
		masterserver_cookie_type cookie = {};
		bool accepted = false;
		bool official = false;
	};
}

using masterserver_request = std::variant<
	masterserver_in::heartbeat,
	masterserver_in::tell_me_my_address,
	masterserver_in::goodbye,

	int, // dummy
	float, // dummy

	masterserver_in::webrtc_signalling_payload
>;

using masterserver_response = std::variant<
	masterserver_out::tell_me_my_address,
	masterserver_out::heartbeat_ack
>;
