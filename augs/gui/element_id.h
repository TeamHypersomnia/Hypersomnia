#pragma once
#include "augs/misc/pool_id.h"

namespace augs {
	namespace gui {
		template<class element>
		using element_id = pool_id<element>;
	}
}