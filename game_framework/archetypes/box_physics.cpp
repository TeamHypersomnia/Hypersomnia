#include "utilities/entity_system/entity_id.h"
#include "game_framework/components/physics_component.h"

#include "game_framework/game/body_helper.h"

#include "game_framework/globals/filters.h"

namespace archetypes {
	void crate_physics(augs::entity_id e) {
		helpers::physics_info info;
		info.from_renderable(e);

		info.filter = filters::dynamic_object();
		info.density = 0.5;
		info.fixed_rotation = false;
		info.angular_damping = 5;
		info.linear_damping = 5;
		info.angled_damping = false;

		helpers::create_physics_component(info, e, b2_dynamicBody);
	}
}