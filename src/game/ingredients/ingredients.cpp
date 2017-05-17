#include "ingredients.h"
#include "game/systems_stateless/render_system.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/tree_of_npo_node_component.h"
#include "game/components/motor_joint_component.h"

namespace ingredients {
	components::item& make_item(const entity_handle e) {
		auto& item = e += components::item();

		components::motor_joint motor;
		motor.activated = false;
		motor.max_force = 20000.f;
		motor.max_torque = 2.f;
		motor.correction_factor = 0.82;

		e += motor;
		
		return item;
	}

	void make_always_visible(entity_handle e) {
		components::tree_of_npo_node node;
		node.always_visible = true;
		e += node;
	}
}
