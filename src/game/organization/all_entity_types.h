#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_in_list_id.h"

#include "game/organization/all_components_declaration.h"

/* E.g. a player as a resistance soldier or metropolitan guard */

struct controlled_character {
	static constexpr std::size_t statically_allocated_num = 300;

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
	static constexpr std::size_t statically_allocated_num = 300;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon
	>;

	using components = type_list<
		components::rigid_body,
		components::force_joint
	>;
};

/* E.g. a crate, a wall, a bullet shell */

struct plain_sprited_body {
	static constexpr std::size_t statically_allocated_num = 1500;

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
	static constexpr std::size_t statically_allocated_num = 1500;

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
	static constexpr std::size_t statically_allocated_num = 1500;

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
	static constexpr std::size_t statically_allocated_num = 1500;

	using invariants = type_list<
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::transform
	>;
};

struct static_light {
	static constexpr std::size_t statically_allocated_num = 200;

	using invariants = type_list<
		invariants::light,
		invariants::sprite,
		invariants::render
	>;

	using components = type_list<
		components::transform
	>;
};

struct throwable_explosive {
	static constexpr std::size_t statically_allocated_num = 1500;

	using invariants = type_list<
		invariants::sprite,
		invariants::item,
		invariants::render,
		invariants::hand_fuse,
		invariants::explosive,
		invariants::shape_polygon
	>;

	using components = type_list<
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::sender
	>;
};

struct plain_missile {
	static constexpr std::size_t statically_allocated_num = 1500;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon,

		invariants::sprite,
		invariants::render,

		invariants::trace,

		invariants::missile
	>;

	using components = type_list<
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::missile,
		components::sender
	>;
};

struct explosive_missile {
	static constexpr std::size_t statically_allocated_num = 1500;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::shape_polygon,

		invariants::sprite,
		invariants::render,

		invariants::trace,

		invariants::missile,
		invariants::explosive
	>;

	using components = type_list<
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::missile,
		components::sender
	>;
};

using all_entity_types = type_list<
	controlled_character,
	plain_invisible_body,
	plain_sprited_body,
	shootable_weapon,
	shootable_charge,
	static_sprite_decoration,
	throwable_explosive,
	plain_missile,
	explosive_missile
>;

using entity_type_id = type_in_list_id<all_entity_types>;
