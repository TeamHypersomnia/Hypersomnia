#include <sol.hpp>

#include "augs/log.h"
#include "augs/ensure.h"

namespace augs {
	sol::state create_lua_state();
	sol::state& get_thread_local_lua_state();

	sol::protected_function_result lua_error_callback(lua_State* s, sol::protected_function_result pfr);
}
