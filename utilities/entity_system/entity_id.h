#pragma once
#include "misc/memory_pool.h"

namespace augs {
		class entity;

		typedef memory_pool::typed_id<entity> entity_id;
}