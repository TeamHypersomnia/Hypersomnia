#pragma once
#include "augs/templates/type_list.h"

#include "game/organization/all_components_declaration.h"
#include "game/organization/all_entity_types_declaration.h"

struct items_of_slots_cache;
struct rigid_body_cache;
struct colliders_cache;
struct tree_of_npo_cache_data;

/* E.g. a player as a resistance soldier or metropolitan guard */

struct controlled_character {
	static constexpr std::size_t statically_allocated_entities = 300;
	static constexpr std::size_t statically_allocated_flavours = 20;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::movement,

		invariants::crosshair,
		invariants::sentience,
		invariants::container,
		invariants::item_slot_transfers,
		invariants::melee_fighter,

		invariants::sprite,
		invariants::torso,
		invariants::head,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::rigid_body,
		components::movement,

		components::item_slot_transfers,
		components::melee_fighter,
		components::crosshair,
		components::sentience,

		components::driver,
		components::attitude,
		components::head
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		items_of_slots_cache,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. a crate, a wall */

struct plain_sprited_body {
	static constexpr std::size_t statically_allocated_entities = 3000;
	static constexpr std::size_t statically_allocated_flavours = 1000;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::render,
		invariants::animation,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::sprite,
		components::sorting_order,
		components::overridden_geo,
		components::rigid_body,
		components::animation
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. an AK or a pistol */

struct shootable_weapon {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::gun,

		invariants::container,
		invariants::item,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::gun,

		components::item,

		components::rigid_body
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		items_of_slots_cache,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. a knife, a machete */

struct melee_weapon {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::melee,
		invariants::item,

		invariants::rigid_body,
		invariants::fixtures,

		invariants::interpolation,
		invariants::continuous_particles,
		invariants::continuous_sound
	>;

	using component_list = type_list<
		components::animation,
		components::continuous_particles,
		components::melee,
		components::item,
		components::rigid_body,
		components::sender
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. a cyan charge or an interference charge */

struct shootable_charge {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::item,
		invariants::cartridge,

		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::item,
		components::rigid_body
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. floors, neon captions like "Welcome to metropolis" */

struct static_decoration {
	static constexpr std::size_t statically_allocated_entities = 20000;
	static constexpr std::size_t statically_allocated_flavours = 2000;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::render,
		invariants::ground
	>;

	using component_list = type_list<
		components::sprite,
		components::sorting_order,
		components::transform,
		components::overridden_geo
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct dynamic_decoration {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 1000;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::render,
		invariants::ground,
		invariants::movement_path
	>;

	using component_list = type_list<
		components::sprite,
		components::sorting_order,
		components::animation,
		components::transform,
		components::movement_path,
		components::overridden_geo
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};


struct wandering_pixels_decoration {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 20;

	using invariant_list = type_list<
		invariants::wandering_pixels
	>;

	using component_list = type_list<
		components::position,
		components::overridden_geo,
		components::wandering_pixels
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct static_light {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 50;

	using invariant_list = type_list<
		invariants::light
	>;

	using component_list = type_list<
		components::transform,
		components::light
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct hand_explosive {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::item,
		invariants::hand_fuse,
		invariants::explosive,
		invariants::rigid_body,
		invariants::fixtures,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::animation,
		components::rigid_body,
		components::hand_fuse,
		components::item,
		components::sender
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

struct plain_missile {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,

		invariants::sprite,

		invariants::trace,

		invariants::missile,

		invariants::interpolation,
		invariants::explosive
	>;

	using component_list = type_list<
		components::rigid_body,
		components::missile,
		components::sender,
		components::trace
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

struct finishing_trace {
	static constexpr std::size_t statically_allocated_entities = 3000;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::trace,
		invariants::interpolation
	>;

	using component_list = type_list<
		components::transform,
		components::trace
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		tree_of_npo_cache_data
	>;
};

struct container_item {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,

		invariants::sprite,

		invariants::container,
		invariants::item,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::rigid_body,
		components::item
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		items_of_slots_cache,
		rigid_body_cache,
		colliders_cache
	>;
};

/* E.g. a bullet shell, a round remnant */

struct remnant_body {
	static constexpr std::size_t statically_allocated_entities = 4000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::sprite,
		invariants::interpolation,
		invariants::remnant
	>;

	using component_list = type_list<
		components::sprite,
		components::rigid_body,

		components::remnant
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

struct sound_decoration {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 500;

	using invariant_list = type_list<
		invariants::continuous_sound
	>;

	using component_list = type_list<
		components::transform
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct particles_decoration {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 500;

	using invariant_list = type_list<
		invariants::continuous_particles
	>;

	using component_list = type_list<
		components::continuous_particles,
		components::transform,
		components::overridden_geo
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct point_marker {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 100;

	using invariant_list = type_list<
		invariants::point_marker
	>;

	using component_list = type_list<
		components::marker,
		components::transform
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct area_marker {
	static constexpr std::size_t statically_allocated_entities = 1000;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::area_marker
	>;

	using component_list = type_list<
		components::sorting_order,
		components::marker,
		components::transform,
		components::overridden_geo
	>;

	using synchronized_arrays = type_list<
		tree_of_npo_cache_data
	>;
};

struct explosion_body {
	static constexpr std::size_t statically_allocated_entities = 2000;
	static constexpr std::size_t statically_allocated_flavours = 300;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::cascade_explosion,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::rigid_body,
		components::cascade_explosion,
		components::sender
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		rigid_body_cache,
		colliders_cache
	>;
};

struct tool_item {
	static constexpr std::size_t statically_allocated_entities = 1500;
	static constexpr std::size_t statically_allocated_flavours = 150;

	using invariant_list = type_list<
		invariants::sprite,
		invariants::animation,
		invariants::container,
		invariants::item,
		invariants::rigid_body,
		invariants::fixtures,
		invariants::tool,

		invariants::interpolation
	>;

	using component_list = type_list<
		components::animation,
		components::rigid_body,
		components::item,
		components::sender
	>;

	using synchronized_arrays = type_list<
		components::interpolation,
		items_of_slots_cache,
		rigid_body_cache,
		colliders_cache
	>;
};

struct area_sensor {
	static constexpr std::size_t statically_allocated_entities = 100;
	static constexpr std::size_t statically_allocated_flavours = 10;

	using invariant_list = type_list<
		invariants::rigid_body,
		invariants::fixtures,
		invariants::area_marker,
		invariants::continuous_particles,
		invariants::continuous_sound,
		invariants::interpolation
	>;

	using component_list = type_list<
		components::sorting_order,
		components::marker,
		components::overridden_geo,
		components::rigid_body,
		components::portal,
		components::continuous_particles
	>;

	using synchronized_arrays = type_list<
		rigid_body_cache,
		colliders_cache,
		components::interpolation
	>;
};
