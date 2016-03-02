#include "ingredients.h"
#include "game_framework/systems/render_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../components/item_component.h"
#include "../components/trigger_component.h"

namespace ingredients {
	void make_always_visible(augs::entity_id e) {
		e->get_owner_world().get_system<render_system>().set_visibility_persistence(e, true);
	}

	void cancel_always_visible(augs::entity_id e) {
		e->get_owner_world().get_system<render_system>().set_visibility_persistence(e, false);
	}

	components::item& make_item(augs::entity_id e) {
		auto& item = *e += components::item();

		e->add<components::trigger>();
		e->get<components::trigger>().react_to_collision_detectors = true;
		e->get<components::trigger>().react_to_query_detectors = false;

		auto& force_joint = e->add<components::force_joint>();
		e->disable(force_joint);

		return item;
	}
}
