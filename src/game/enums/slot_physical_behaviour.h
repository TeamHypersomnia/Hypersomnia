#pragma once

enum class slot_physical_behaviour : unsigned char {
	DEACTIVATE_BODIES,
	CONNECT_AS_FIXTURE_OF_BODY,
	CONNECT_AS_JOINTED_BODY
};