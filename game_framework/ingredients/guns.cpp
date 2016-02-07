#include "augs/entity_system/entity_id.h"
#include "game_framework/components/gun_component.h"

#include "game_framework/globals/filters.h"

namespace ingredients {
	void assault_rifle(augs::entity_id e) {
		components::gun gun;


		e->add(gun);
	}
}


