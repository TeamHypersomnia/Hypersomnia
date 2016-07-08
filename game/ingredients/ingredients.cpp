#include "ingredients.h"
#include "game/systems/render_system.h"

#include "game/cosmos.h"
#include "game/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/trigger_component.h"
#include "game/components/force_joint_component.h"

namespace ingredients {
	components::item& make_item(entity_handle e) {
		auto& item = e += components::item();

		e.add(components::trigger());
		e.get<components::trigger>().react_to_collision_detectors = true;
		e.get<components::trigger>().react_to_query_detectors = false;

		auto& force_joint = e.add(components::force_joint());
		e.get<components::processing>().remove_from(processing_subjects::WITH_FORCE_JOINT);

		return item;
	}
}
