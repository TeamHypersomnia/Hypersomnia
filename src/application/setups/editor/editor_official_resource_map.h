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
	Map<test_static_lights, editor_light_resource> lights;
	Map<test_particles_decorations, editor_particles_resource> particles_decorations;
	Map<test_wandering_pixels_decorations, editor_wandering_pixels_resource> wandering_pixels;
	Map<point_marker_type, editor_point_marker_resource> point_markers;
	Map<area_marker_type, editor_area_marker_resource> area_markers;

	Map<test_shootable_weapons, editor_firearm_resource> firearms;
	Map<test_container_items, editor_ammunition_resource> magazines;
	Map<test_shootable_charges, editor_ammunition_resource> shootable_charges;
	Map<test_melee_weapons, editor_melee_resource> melees;
	Map<test_hand_explosives, editor_explosive_resource> explosives;
	Map<test_tool_items, editor_tool_resource> tools;

	Map<test_scene_physical_material_id, editor_material_resource> materials;

	Map<editor_builtin_prefab_type, editor_prefab_resource> prefabs;
	// END GEN INTROSPECTOR

	auto create_name_to_id_map() const;

	template <class S>
	auto create_id_to_name_map(S& taken_names) const;

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

