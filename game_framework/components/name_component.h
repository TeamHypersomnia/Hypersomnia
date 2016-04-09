#pragma once
#include "../globals/entity_name.h"

namespace components {
	struct name {
		entity_name id;
	};
}

augs::entity_id get_first_named_ancestor(augs::entity_id);