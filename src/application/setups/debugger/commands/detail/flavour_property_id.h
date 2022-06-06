#pragma once
#include "application/setups/debugger/detail/field_address.h"

struct flavour_property_id {
	unsigned invariant_id = static_cast<unsigned>(-1);
	flavour_field_address field;
};

