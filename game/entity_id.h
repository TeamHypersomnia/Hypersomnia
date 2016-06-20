#include "augs/misc/object_pool_id.h"
#include "game/types_specification/components_instantiation.h"

namespace augs {
	template<class...>
	class component_aggregate;
}

typedef augs::object_pool_id<typename put_all_components_into<augs::component_aggregate>::type> entity_id;