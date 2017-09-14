#pragma once
#include <sol.hpp>

#include "augs/log.h"
#include "augs/ensure.h"

namespace augs {
	sol::state create_lua_state();
}
