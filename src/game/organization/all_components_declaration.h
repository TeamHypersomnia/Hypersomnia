#pragma once
// disables the warning due to type name length exceeded
#if PLATFORM_WINDOWS
#pragma warning(disable : 4503)
#endif

#include "augs/templates/type_pair.h"
#include "augs/templates/type_list.h"
#include "augs/templates/folded_finders.h"

#include "game/components/transform_component_declaration.h"
#include "game/components/sprite_component_declaration.h"
#include "game/components/polygon_component_declaration.h"

namespace invariants {
	struct gun;
	struct render;
	struct shape_polygon;
	struct shape_circle;
	struct trace;
	struct interpolation;
	struct flags;
	struct text_details;
	struct fixtures;
	struct rigid_body;
	struct container;
	struct item;
	struct missile;
	struct sentience;
	struct wandering_pixels;
	struct cartridge;
	struct hand_fuse;
	struct explosive;
	struct movement;
	struct light;
	struct crosshair;
	struct item_slot_transfers;
	struct torso;
	struct head;
	struct ground;
	struct movement_path;
	struct animation;
	struct remnant;
	struct continuous_sound;
	struct continuous_particles;
	struct point_marker;
	struct area_marker;
	struct cascade_explosion;
	struct tool;
	struct touch_collectible;
	struct melee;
	struct melee_fighter;
	struct decal;
	struct destructible;
}

namespace components {
	struct crosshair;
	struct missile;
	struct gun;
	struct movement;
	struct rigid_body;
	struct car;
	struct driver;
	struct specific_colliders_connection;
	struct item;
	struct force_joint;
	struct item_slot_transfers;
	struct trace;
	struct melee;
	struct sentience;
	struct attitude;
	struct processing;
	struct interpolation;
	struct light;
	struct wandering_pixels;
	struct motor_joint;
	struct hand_fuse;
	struct sender;
	struct head;
	struct movement_path;
	struct animation;
	struct remnant;
	struct continuous_particles;
	struct overridden_geo;
	struct cascade_explosion;
	struct melee_fighter;
	struct marker;
	struct sorting_order;
	struct portal;
	struct decal;
	struct destructible;
}

using assert_always_together = type_list<
	type_pair<invariants::gun, components::gun>,
	type_pair<invariants::trace, components::trace>,
	type_pair<invariants::rigid_body, components::rigid_body>,
	type_pair<invariants::rigid_body, invariants::fixtures>,
	type_pair<invariants::item, components::item>,
	type_pair<invariants::missile, components::missile>,
	type_pair<invariants::sentience, components::sentience>,
	type_pair<invariants::wandering_pixels, components::wandering_pixels>,
	type_pair<invariants::hand_fuse, components::hand_fuse>,
	type_pair<invariants::movement, components::movement>,
	type_pair<invariants::light, components::light>,
	type_pair<invariants::crosshair, components::crosshair>,
	type_pair<invariants::movement_path, components::movement_path>,
	type_pair<invariants::animation, components::animation>,
	type_pair<invariants::remnant, components::remnant>,
	type_pair<invariants::cascade_explosion, components::cascade_explosion>,
	type_pair<invariants::melee, components::melee>,
	type_pair<invariants::decal, components::decal>,
	type_pair<invariants::destructible, components::destructible>
>;

using assert_first_implies_second = type_list<
	type_pair<invariants::item, invariants::sprite>,
	type_pair<invariants::wandering_pixels, components::position>,
	type_pair<components::continuous_particles, invariants::continuous_particles>,
	type_pair<invariants::light, components::transform>,
	type_pair<invariants::area_marker, components::marker>,
	type_pair<invariants::point_marker, components::marker>
>;

using assert_never_together = type_list<
	type_pair<components::rigid_body, components::transform>,
	type_pair<components::rigid_body, components::position>,
	type_pair<components::position, components::transform>,
	type_pair<components::rigid_body, components::wandering_pixels>,
	type_pair<invariants::melee, invariants::missile>,
	type_pair<invariants::area_marker, invariants::point_marker>
>;

using always_present_invariants = type_list<
	invariants::text_details,
	invariants::flags
>;

template <template <class...> class List>
using component_list_t = List<
	components::sprite,
	components::crosshair,
	components::missile,
	components::gun,
	components::movement,
	components::rigid_body,
	components::specific_colliders_connection,
	components::transform,
	components::position,
	components::car,
	components::driver,
	components::item,
	components::force_joint,
	components::item_slot_transfers,
	components::trace,
	components::melee,
	components::melee_fighter,
	components::sentience,
	components::attitude,
	components::interpolation,
	components::light,
	components::wandering_pixels,
	components::motor_joint,
	components::hand_fuse,
	components::sender,
	components::head,
	components::movement_path,
	components::animation,
	components::remnant,
	components::continuous_particles,
	components::overridden_geo,
	components::cascade_explosion,
	components::marker,
	components::sorting_order,
	components::portal,
	components::decal,
	components::destructible
>;

template <template <class...> class List>
using invariant_list_t = List<
	invariants::text_details,
	invariants::flags,
	invariants::gun,
	invariants::render,
	invariants::shape_polygon,
	invariants::shape_circle,
	invariants::polygon,
	invariants::sprite,
	invariants::trace,
	invariants::interpolation,
	invariants::fixtures,
	invariants::rigid_body,
	invariants::container,
	invariants::item,
	invariants::missile,
	invariants::sentience,
	invariants::movement,
	invariants::wandering_pixels,
	invariants::cartridge,
	invariants::explosive,
	invariants::hand_fuse,
	invariants::crosshair,
	invariants::light,
	invariants::item_slot_transfers,
	invariants::torso,
	invariants::head,
	invariants::ground,
	invariants::movement_path,
	invariants::animation,
	invariants::remnant,
	invariants::continuous_sound,
	invariants::continuous_particles,
	invariants::point_marker,
	invariants::area_marker,
	invariants::cascade_explosion,
	invariants::tool,
	invariants::touch_collectible,
	invariants::melee,
	invariants::melee_fighter,
	invariants::decal,
	invariants::destructible
>;

template <class... Types>
struct type_count {
	static const unsigned value = sizeof...(Types);
};

class cosmos;

constexpr unsigned COMPONENTS_COUNT = component_list_t<type_count>::value;
constexpr unsigned INVARIANTS_COUNT = invariant_list_t<type_count>::value;

template <class D>
static constexpr auto invariant_index_v = index_in_list_v<D, invariant_list_t<type_list>>;

template <class D>
static constexpr bool is_invariant_v = is_one_of_list_v<D, invariant_list_t<type_list>>;