#include "entity_scripts.h"
#include "game/components/movement_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/components/car_component.h"

void unset_input_flags_of_orphaned_entity(augs::entity_id e) {
	auto* gun = e->find<components::gun>();
	auto* melee = e->find<components::melee>();
	auto* car = e->find<components::car>();
	auto* movement = e->find<components::movement>();
	auto* damage = e->find<components::damage>();

	if (car)
		car->reset_movement_flags();

	if (movement)
		movement->reset_movement_flags();

	if (gun)
		gun->trigger_pressed = false;

	if (melee) {
		melee->reset_weapon(e);
	}
}