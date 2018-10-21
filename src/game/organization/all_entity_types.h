#pragma once
#include "augs/templates/type_list.h"

#include "game/organization/all_components_declaration.h"
#include "game/organization/all_entity_types_declaration.h"

/* E.g. a player as a resistance soldier or metropolitan guard */

struct controlled_character {
	static constexpr std::size_t statically_allocated_entities = 300;
	static constexpr std::size_t statically_allocated_flavours = 20;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::movement,

		invariants::crosshair,
		invariants::sentience,
		invariants::container,
		invariants::item_slot_transfers,
		invariants::melee_fighter,

		invariants::sprite,
		invariants::render,
		invariants::torso,
		invariants::head,

		invariants::interpolation
	>;

	using components = type_list<
		components::rigid_body,
		components::movement,

		components::item_slot_transfers,
		components::melee_fighter,
		components::crosshair,
		components::sentience,

		components::driver,
		components::attitude,
		components::head,

		components::interpolation
	>;
};

/* E.g. a crate, a wall */

struct plain_sprited_body {
	static constexpr std::size_t statically_allocated_entities = 3000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,
		invariants::ground,

		invariants::interpolation
	>;

	using components = type_list<
		components::sprite,
		components::overridden_geo,
		components::rigid_body,
		components::interpolation
	>;
};

/* E.g. an AK or a pistol */

struct shootable_weapon {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::gun,

		invariants::container,
		invariants::item,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,

		invariants::interpolation
	>;

	using components = type_list<
		components::gun,

		components::item,

		components::rigid_body,

		components::interpolation
	>;
};

/* E.g. a knife, a machete */

struct melee_weapon {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::melee,
		invariants::item,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,

		invariants::interpolation
	>;

	using components = type_list<
		components::melee,
		components::item,
		components::rigid_body,
		components::interpolation,

		components::sender
	>;
};

/* E.g. a cyan charge or an interference charge */

struct shootable_charge {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::item,
		invariants::cartridge,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,

		invariants::interpolation
	>;

	using components = type_list<
		components::item,
		components::rigid_body,

		components::interpolation
	>;
};

/* E.g. floors, neon captions like "Welcome to metropolis" */

struct sprite_decoration {
	static constexpr std::size_t statically_allocated_entities = 20000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariants = type_list<
		invariants::sprite,
		invariants::render,
		invariants::ground
	>;

	using components = type_list<
		components::sprite,
		components::transform,
		components::overridden_geo
	>;
};

struct wandering_pixels_decoration {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 20;

	using invariants = type_list<
		invariants::wandering_pixels,
		invariants::render
	>;

	using components = type_list<
		components::position,
		components::overridden_geo,
		components::wandering_pixels
	>;
};

struct static_light {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 50;

	using invariants = type_list<
		invariants::light
	>;

	using components = type_list<
		components::transform,
		components::light
	>;
};

struct hand_explosive {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::item,
		invariants::render,
		invariants::hand_fuse,
		invariants::explosive,
		invariants::rigid_body,
		invariants::fixtures,

		invariants::interpolation
	>;

	using components = type_list<
		components::animation,
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::sender,

		components::interpolation
	>;
};

struct plain_missile {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,

		invariants::sprite,
		invariants::render,

		invariants::trace,

		invariants::missile,

		invariants::interpolation
	>;

	using components = type_list<
		components::rigid_body,
		components::missile,
		components::sender,
		components::trace,

		components::interpolation
	>;
};

struct finishing_trace {
	static constexpr std::size_t statically_allocated_entities = 3000;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::sprite,
		invariants::render,
		invariants::trace,
		invariants::interpolation
	>;

	using components = type_list<
		components::transform,
		components::trace,
		components::interpolation
	>;
};

struct container_item {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,

		invariants::sprite,
		invariants::render,

		invariants::container,
		invariants::item,

		invariants::interpolation
	>;

	using components = type_list<
		components::rigid_body,
		components::item,

		components::interpolation
	>;
};

struct explosive_missile {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::sprite,
		invariants::animation,

		invariants::rigid_body,
		invariants::fixtures,

		invariants::render,

		invariants::trace,

		invariants::item,
		invariants::hand_fuse,
		invariants::missile,
		invariants::explosive,

		invariants::interpolation
	>;

	using components = type_list<
		components::animation,
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::missile,
		components::sender,
		components::trace,

		components::interpolation
	>;
};

struct complex_decoration {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariants = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::render,
		invariants::ground,
		invariants::movement_path
	>;

	using components = type_list<
		components::sprite,
		components::animation,
		components::transform,
		components::movement_path
	>;
};

/* E.g. a bullet shell, a round remnant */

struct remnant_body {
	static constexpr std::size_t statically_allocated_entities = 4000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,
		invariants::ground,

		invariants::interpolation,
		invariants::remnant
	>;

	using components = type_list<
		components::sprite,
		components::rigid_body,

		components::interpolation,
		components::remnant
	>;
};

struct sound_decoration {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 500;

	using invariants = type_list<
		invariants::continuous_sound
	>;

	using components = type_list<
		components::transform
	>;
};

struct particles_decoration {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 500;

	using invariants = type_list<
		invariants::continuous_particles
	>;

	using components = type_list<
		components::continuous_particles,
		components::transform,
		components::overridden_geo
	>;
};

struct point_marker {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 50;

	using invariants = type_list<
		invariants::point_marker
	>;

	using components = type_list<
		components::transform
	>;
};

struct box_marker {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 50;

	using invariants = type_list<
		invariants::box_marker
	>;

	using components = type_list<
		components::transform,
		components::overridden_geo
	>;
};

struct explosion_body {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariants = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::cascade_explosion,

		invariants::interpolation
	>;

	using components = type_list<
		components::rigid_body,
		components::cascade_explosion,
		components::sender,
		components::interpolation
	>;
};

struct tool_item {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariants = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::item,
		invariants::render,
		invariants::rigid_body,
		invariants::fixtures,
		invariants::tool,

		invariants::interpolation
	>;

	using components = type_list<
		components::animation,
		components::rigid_body,
		components::item,
		components::sender,

		components::interpolation
	>;
};
