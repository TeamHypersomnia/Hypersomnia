#pragma once
#include <sol.hpp>

#include "augs/templates/exception_templates.h"

namespace augs {
	struct lua_state_creation_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	sol::state create_lua_state();
}
