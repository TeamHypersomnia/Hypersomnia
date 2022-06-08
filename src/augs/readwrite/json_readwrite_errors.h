#pragma once
#include "augs/templates/exception_templates.h"

namespace augs {
	struct json_deserialization_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};
}
