#pragma once
#include "augs/templates/type_list.h"

struct controlled_character {
	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::movement,
		invariants::shape_polygon,
		invariants::sentience,
		invariants::container,
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::item_slot_transfers
		components::rigid_body,
		components::fixtures,
		components::movement,
		components::animation,
		components::driver,
		components::attitude,
		components::sentience
	>;
};

struct environment_obstacle {

};

struct shootable {

};

struct shootable_charge {

};

struct crosshair_recoil {

};

using all_entity_types = type_list<
	controlled_character,
	environment_obstacle,
	shootable,
	shootable_charge,
	crosshair_recoil
>;
