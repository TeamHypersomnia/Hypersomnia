#pragma once

enum class nat_type : uint8_t {
	PUBLIC_INTERNET,
	CONE,
	ADDRESS_SENSITIVE,
	PORT_SENSITIVE
};

struct nat_type_detector {
	enum class state {
		RESOLVING,
		RESOLVED
	};

	state current_state;


};
