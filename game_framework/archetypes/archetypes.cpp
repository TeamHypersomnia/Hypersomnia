#include "archetypes.h"
#include "game_framework/systems/render_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

namespace archetypes {
	void always_visible(augs::entity_id e) {
		e->get_owner_world().get_system<render_system>().set_visibility_persistence(e, true);
	}

	void cancel_always_visible(augs::entity_id e) {
		e->get_owner_world().get_system<render_system>().set_visibility_persistence(e, false);
	}
}
