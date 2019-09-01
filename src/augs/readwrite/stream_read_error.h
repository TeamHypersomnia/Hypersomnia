#pragma once
#include "augs/templates/exception_templates.h"

namespace augs {
	struct stream_read_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};
}
