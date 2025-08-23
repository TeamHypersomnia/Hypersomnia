#pragma once
#include "augs/network/network_types.h"

namespace masterserver_out {
	struct webrtc_signalling_payload {
		static constexpr bool force_read_field_by_field = true;

		// GEN INTROSPECTOR struct masterserver_out::webrtc_signalling_payload
		int64_t message_guid = 0;
		signalling_message_str message;
		// END GEN INTROSPECTOR
	};
}

namespace masterserver_in {
	using webrtc_signalling_payload = masterserver_out::webrtc_signalling_payload;
}
