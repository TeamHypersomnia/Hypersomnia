#include "ingredients.h"
#include "game/systematic/render_system.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/trigger_component.h"
#include "game/components/dynamic_tree_node_component.h"
#include "game/components/force_joint_component.h"

namespace ingredients {
	components::item& make_item(entity_handle e) {
		auto& item = e += components::item();
		auto& processing = e += components::processing();

		e.add(components::trigger());
		e.get<components::trigger>().react_to_collision_detectors = true;
		e.get<components::trigger>().react_to_query_detectors = false;

		auto& force_joint = e.add(components::force_joint());
		processing.disable_in(processing_subjects::WITH_FORCE_JOINT);

		return item;
	}

	void make_always_visible(entity_handle e) {
		components::dynamic_tree_node node = components::dynamic_tree_node::get_default(e);
		node.always_visible = true;
		e += node;
	}
}
