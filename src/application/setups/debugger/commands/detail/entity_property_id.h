#pragma once
#include "application/setups/debugger/detail/field_address.h"

struct entity_property_id {
	unsigned component_id = static_cast<unsigned>(-1);
	entity_field_address field;
};

