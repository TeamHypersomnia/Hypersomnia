#include "ingredients.h"
#include "game/systems/render_system.h"

#include "game/cosmos.h"
#include "game/entity_id.h"

#include "game/components/item_component.h"
#include "game/components/trigger_component.h"

namespace ingredients {
	void make_always_visible(entity_id e) {
		e->get_owner_world().systems.get<render_system>().set_visibility_persistence(e, true);
	}

	void cancel_always_visible(entity_id e) {
		e->get_owner_world().systems.get<render_system>().set_visibility_persistence(e, false);
	}

	components::item& make_item(entity_id e) {
		auto& item = *e += components::item();

		e->add<components::trigger>();
		e->get<components::trigger>().react_to_collision_detectors = true;
		e->get<components::trigger>().react_to_query_detectors = false;

		auto& force_joint = e->add<components::force_joint>();
		e.skip_processing_in(processing_subjects::force_joint>();

		return item;
	}
}
