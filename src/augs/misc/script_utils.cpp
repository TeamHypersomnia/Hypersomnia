#include "script_utils.h"

namespace augs {
	sol::protected_function_result lua_error_callback(lua_State* s, sol::protected_function_result pfr) {
		LOG(pfr.operator std::string());
		ensure(pfr.valid());
		return pfr;
	}
}