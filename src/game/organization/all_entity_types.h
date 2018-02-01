#pragma once
#include "augs/templates/type_list.h"

/* E.g. a player as a resistance soldier or metropolitan guard */

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
		components::rigid_body,
		components::movement,

		components::item_slot_transfers,
		components::sentience

		components::animation,
		components::driver,
		components::attitude
	>;
};

/* Crosshair's recoil body */

struct plain_invisible_body {
	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon
	>;

	using components = type_list<
		components::rigid_body
	>;
};

/* E.g. a crate, a wall, a bullet shell */

struct plain_sprited_body {
	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon,
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::rigid_body
	>;
};

/* E.g. an AK or a pistol */

struct shootable_weapon {
	using invariants = type_list<
		invariants::gun,

		invariants::container,
		invariants::item,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon,
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::gun,

		components::container,
		components::item,

		components::rigid_body
	>;
};

/* E.g. a cyan charge or an interference charge */

struct shootable_charge {
	using invariants = type_list<
		invariants::item,
		invariants::catridge,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon,
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::item,
		components::rigid_body
	>;
};

/* E.g. neon captions like "Welcome to metropolis" */

struct static_sprite_decoration {
	using invariants = type_list<
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::transform
	>;
};

using all_entity_types = type_list<
	controlled_character,
	plain_invisible_body,
	plain_sprited_body,
	shootable_weapon,
	shootable_charge,
	static_sprite_decoration
>;
