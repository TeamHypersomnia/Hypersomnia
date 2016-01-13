#pragma once
#include "entity_system/entity.h"

namespace components {
	struct trigger {
		augs::entity_id entity_to_be_notified;
	};
}
