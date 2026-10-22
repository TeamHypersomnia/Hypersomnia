#pragma once
#include <array>
#include <algorithm>
#include "augs/ensure_rel.h"
#include "augs/math/si_scaling.h"
#include "augs/misc/convex_partitioned_shape.hpp"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"
#include "game/balance_params.h"
#include "game/detail/movement/top_movement_speed.h"
#include "test_scenes/test_scene_flavours.h"
#include "view/viewables/image_cache.h"

/*
	Weapon densities are set from the sprint speed with the armor on,
	and the weapon stats report the same speed - so both compute it here.
*/

namespace test_flavours {
	/*
		The mass the fixtures of a flavour get once attached to a body,
		built like in physics_world_cache: from the image's convex partition,
		or a box of the sprite's size.
	*/

	template <class M>
	real32 calc_attached_mass(const M& meta, const real32 density, const loaded_image_caches_map& caches) {
		const auto& sprite = meta.template get<invariants::sprite>();
		const auto si = si_scaling();

		auto total_mass = 0.f;

		auto add_polygon = [&](const auto& verts) {
			augs::constant_size_vector<b2Vec2, b2_maxPolygonVertices> b2verts(verts.begin(), verts.end());

			for (auto& v : b2verts) {
				v = si.get_meters(v);
			}

			b2PolygonShape polygon;
			polygon.Set(b2verts.data(), static_cast<int32>(b2verts.size()));

			b2MassData mass_data;
			polygon.ComputeMass(&mass_data, density);
			total_mass += mass_data.mass;
		};

		if (const auto& shape = caches.at(sprite.image_id).non_standard_shape; !shape.empty()) {
			shape.for_each_convex([&](const auto& convex) {
				if (convex.size() <= b2_maxPolygonVertices) {
					add_polygon(convex);
				}
			});
		}
		else {
			auto size = vec2(sprite.get_size());
			size.x = std::max(1.f, size.x);
			size.y = std::max(1.f, size.y);

			const auto h = size / 2;

			add_polygon(std::array<vec2, 4> {
				vec2(-h.x, -h.y),
				vec2(h.x, -h.y),
				vec2(h.x, h.y),
				vec2(-h.x, h.y)
			});
		}

		return total_mass;
	}

	/*
		The mass of what is loaded into the gun's slots and attached to it as fixtures -
		the magazine, unless hidden, and the charge in the chamber of e.g. a rocket launcher.
		Hidden slots deactivate the bodies of their items, so these weigh nothing.
	*/

	template <class M>
	real32 calc_loaded_mass(const M& meta, const all_entity_flavours& flavours, const loaded_image_caches_map& caches) {
		const auto& slots = meta.template get<invariants::container>().slots;

		auto loaded_mass_in = [&]<class FlavourEnum>(const slot_function slot_type, const FlavourEnum) {
			const auto slot = mapped_or_nullptr(slots, slot_type);

			if (slot == nullptr
				|| !slot->only_allow_flavour.is_set()
				|| slot->physical_behaviour != slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY
			) {
				return 0.f;
			}

			using loaded_type = test_flavours_map::at<FlavourEnum>;
			const auto& loaded = flavours.template get_for<loaded_type>()[slot->only_allow_flavour.raw];
			const auto density = loaded.template get<invariants::fixtures>().density * slot->attachment_density_multiplier;

			return calc_attached_mass(loaded, density, caches);
		};

		return
			loaded_mass_in(slot_function::GUN_DETACHABLE_MAGAZINE, test_container_items())
			+ loaded_mass_in(slot_function::GUN_CHAMBER, test_shootable_charges())
		;
	}

	struct sprint_speed_input {
		const all_entity_flavours& flavours;
		const loaded_image_caches_map& caches;
	};

	inline real32 calc_character_mass(const sprint_speed_input in) {
		const auto& character = get_test_flavour(in.flavours, test_controlled_characters::METROPOLIS_SOLDIER);
		return calc_attached_mass(character, character.template get<invariants::fixtures>().density, in.caches);
	}

	/*
		Arguments for calc_top_movement_speed and its inverse when sprinting.
	*/

	template <class F>
	decltype(auto) with_sprint_movement(const sprint_speed_input in, F&& callback) {
		const auto& character = get_test_flavour(in.flavours, test_controlled_characters::METROPOLIS_SOLDIER);
		const auto& movement_def = character.template get<invariants::movement>();
		return callback(movement_def.acceleration_length, BALANCE_ELECTRIC_ARMOR_MOVEMENT_SPEED_MULT, movement_def.standard_linear_damping);
	}

	/*
		How fast a character holding the gun (loaded, nothing else) sprints - in logic pixels per second.
	*/

	template <class M>
	real32 calc_sprint_speed_with(const M& gun_meta, const sprint_speed_input in) {
		const auto density = gun_meta.template get<invariants::fixtures>().density;

		const auto total_mass =
			calc_character_mass(in)
			+ calc_attached_mass(gun_meta, density, in.caches)
			+ calc_loaded_mass(gun_meta, in.flavours, in.caches)
		;

		return with_sprint_movement(in, [&](const auto acceleration_length, const auto speed_mult, const auto damping) {
			return ::calc_top_movement_speed(acceleration_length, speed_mult, damping, total_mass, true);
		});
	}

	/*
		The inverse: the gun's density with which the character sprints at this speed.
	*/

	template <class M>
	real32 calc_density_for_sprint_speed(const M& gun_meta, const real32 sprint_speed, const sprint_speed_input in) {
		const auto total_mass = with_sprint_movement(in, [&](const auto acceleration_length, const auto speed_mult, const auto damping) {
			return ::calc_body_mass_for_top_movement_speed(acceleration_length, speed_mult, damping, sprint_speed, true);
		});

		const auto gun_mass = total_mass - calc_character_mass(in) - calc_loaded_mass(gun_meta, in.flavours, in.caches);

		/*
			Not positive when the speed is at least that of an empty-handed sprint.
		*/

		ensure_greater(gun_mass, 0.f);

		return gun_mass / calc_attached_mass(gun_meta, 1.f, in.caches);
	}
}
