#pragma once
#include "misc/object_pool_id.h"

namespace augs {
	namespace gui {
		template<class element>
		using element_id = object_pool_id<element>;
	}
}