#pragma once
#include <unordered_map>
#include "test_scenes/test_scene_flavour_ids.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "augs/templates/traits/is_variant.h"

struct editor_official_resource_map {
	template <class A, class B>
	using Map = std::unordered_map<A, editor_typed_resource_id<B>>;

	// GEN INTROSPECTOR struct editor_official_resource_map
	Map<test_static_decorations, editor_sprite_resource> static_decorations;
	Map<test_plain_sprited_bodies, editor_sprite_resource> plain_sprited_bodies;
	Map<test_dynamic_decorations, editor_sprite_resource> dynamic_decorations;
	Map<test_sound_decorations, editor_sound_resource> sound_decorations;
	Map<area_marker_type, editor_area_marker_resource> box_markers;
	Map<test_static_lights, editor_light_resource> lights;
	Map<test_particles_decorations, editor_particles_resource> particles_decorations;
	Map<test_wandering_pixels_decorations, editor_wandering_pixels_resource> wandering_pixels;

	Map<test_shootable_weapons, editor_firearm_resource> firearms;
	Map<test_melee_weapons, editor_melee_resource> melees;
	// END GEN INTROSPECTOR

	auto create_name_to_id_map() const;

	template <class S, class T>
	static auto& get_container(S& self, const T&);

	template <class T>
	const auto& dereference_enum(const T& typed) const;

	template <class T>
	auto& dereference_enum(const T& typed);

	template <class V>
	auto& operator[](const V& var);

	template <class V>
	const auto& operator[](const V& var) const;
};

