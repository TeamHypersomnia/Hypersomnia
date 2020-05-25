#include "game/common_state/entity_flavours.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scene_animations.h"
#include "test_scenes/test_scene_images.h"

#include "augs/templates/enum_introspect.h"
#include "augs/string/format_enum.h"
#include "game/detail/inventory/inventory_utils.h"

void populate_test_scene_flavours(const populate_flavours_input in) {
	test_flavours::populate_grenade_flavours(in);
	test_flavours::populate_character_flavours(in);
	test_flavours::populate_gun_flavours(in);
	test_flavours::populate_other_flavours(in);
	test_flavours::populate_car_flavours(in);
	test_flavours::populate_crate_flavours(in);
	test_flavours::populate_decoration_flavours(in);
	test_flavours::populate_melee_flavours(in);
	test_flavours::populate_backpack_flavours(in);
}

namespace test_flavours {
	void populate_other_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto flavour_with_sprite = in.flavour_with_sprite_maker();

		auto flavour_with_tiled_sprite = [&](auto&&... args) -> auto& {
			auto& meta = flavour_with_sprite(std::forward<decltype(args)>(args)...);
			meta.template get<invariants::sprite>().tile_excess_size = true;
			return meta;
		};

		{
			auto& meta = get_test_flavour(flavours, test_static_lights::STRONG_LAMP);

			components::light light; 
			meta.set(light);
		}

		{
			auto& meta = get_test_flavour(flavours, test_static_lights::AQUARIUM_LAMP);

			components::light light; 
			light.attenuation.constant = 75;
			light.attenuation.quadratic = 631;
			meta.set(light);
		}

		{
			auto& meta = get_test_flavour(flavours, test_wandering_pixels_decorations::WANDERING_PIXELS);

			invariants::wandering_pixels wandering;
			wandering.animation_id = to_animation_id(test_scene_plain_animation_id::WANDERING_PIXELS_ANIMATION);
			meta.set(wandering);

			components::wandering_pixels initial_wandering;
			initial_wandering.illuminate = true;
			meta.set(initial_wandering);
		}

		{
			auto& meta = get_test_flavour(flavours, test_wandering_pixels_decorations::AQUARIUM_PIXELS_LIGHT);

			{
				invariants::wandering_pixels wandering;
				wandering.animation_id = to_animation_id(test_scene_plain_animation_id::WANDERING_PIXELS_ANIMATION);
				wandering.max_direction_ms = 2000;
				wandering.max_direction_ms = 700;
				wandering.base_velocity = { 2, 20 };
				meta.set(wandering);
			}

			{
				components::wandering_pixels wandering;
				wandering.keep_particles_within_bounds = true;
				wandering.colorize = { 234, 228, 201, 255 };
				wandering.particles_count = 15;
				wandering.illuminate = true;
				meta.set(wandering);

				components::overridden_geo s;
				s.size.emplace(vec2(750, 750));
				meta.set(s);
			}
		}

		{
			auto& meta = get_test_flavour(flavours, test_wandering_pixels_decorations::AQUARIUM_PIXELS_DIM);

			{
				invariants::wandering_pixels wandering;
				wandering.animation_id = to_animation_id(test_scene_plain_animation_id::WANDERING_PIXELS_ANIMATION);
				wandering.max_direction_ms = 500;
				wandering.direction_interp_ms = 300;
				wandering.base_velocity = { 40, 120 };
				meta.set(wandering);
			}

			{
				components::wandering_pixels wandering;
				wandering.keep_particles_within_bounds = true;
				wandering.colorize = { 234, 228, 201, 255 };
				wandering.particles_count = 500;
				meta.set(wandering);

				components::overridden_geo s;
				s.size.emplace(vec2(750, 750));
				meta.set(s);
			}
		}

		{
			auto& meta = flavour_with_tiled_sprite(
				test_sprite_decorations::SOIL,
				test_scene_image_id::SOIL,
				test_ground_order::SOIL,
				gray1
			);

			invariants::ground ground_def;

			footstep_effect_input dirt;
			dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_DIRT);
			dirt.sound.modifier.gain = .35f;
			dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

			ground_def.footstep_effect.emplace(dirt);
			ground_def.movement_speed_mult = 0.95f;

			meta.set(ground_def);
		}

		{
			auto& meta = flavour_with_sprite(
				test_sprite_decorations::ROAD_DIRT,
				test_scene_image_id::ROAD_FRONT_DIRT,
				test_ground_order::FLOOR_AND_ROAD
			);

			invariants::ground ground_def;

			footstep_effect_input road;
			road.sound.id = to_sound_id(test_scene_sound_id::STANDARD_FOOTSTEP);
			road.sound.modifier.gain = .35f;
			road.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

			ground_def.footstep_effect.emplace(road);

			meta.set(ground_def);
		}

		{
			auto& meta = flavour_with_tiled_sprite(
				test_sprite_decorations::ROAD,
				test_scene_image_id::ROAD,
				test_ground_order::FLOOR_AND_ROAD
			);

			invariants::ground ground_def;

			footstep_effect_input road;
			road.sound.id = to_sound_id(test_scene_sound_id::STANDARD_FOOTSTEP);
			road.sound.modifier.gain = .35f;
			road.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

			ground_def.footstep_effect.emplace(road);

			meta.set(ground_def);
		}

		{
			auto& meta = flavour_with_tiled_sprite(
				test_sprite_decorations::FLOOR,
				test_scene_image_id::FLOOR,
				test_ground_order::FLOOR_AND_ROAD
			);

			invariants::ground ground_def;

			footstep_effect_input dirt;
			dirt.sound.id = to_sound_id(test_scene_sound_id::FOOTSTEP_FLOOR);
			dirt.sound.modifier.gain = .6f;
			dirt.sound.modifier.pitch = .9f;
			dirt.particles.id = to_particle_effect_id(test_scene_particle_effect_id::FOOTSTEP_SMOKE);

			ground_def.footstep_effect.emplace(dirt);

			meta.set(ground_def);

			{
				auto& meta = flavour_with_tiled_sprite(
					test_sprite_decorations::WATER_ROOM_FLOOR,
					test_scene_image_id::WATER_ROOM_FLOOR,
					test_ground_order::FLOOR_AND_ROAD
				);

				meta.get<invariants::sprite>().color = { 132, 132, 132, 255 };
				meta.get<invariants::sprite>().neon_color = { 255, 255, 255, 106 };
				meta.set(ground_def);
			}
		}

		flavour_with_sprite(
			test_sprite_decorations::HAVE_A_PLEASANT,
			test_scene_image_id::HAVE_A_PLEASANT,
			render_layer::FOREGROUND_GLOWS
		);

		flavour_with_sprite(
			test_sprite_decorations::AWAKENING,
			test_scene_image_id::AWAKENING,
			render_layer::FOREGROUND_GLOWS,
			white,
			augs::sprite_special_effect::COLOR_WAVE
		);

		flavour_with_sprite(
			test_sprite_decorations::METROPOLIS,
			test_scene_image_id::METROPOLIS,
			render_layer::FOREGROUND_GLOWS
		);

		flavour_with_sprite(
			test_sprite_decorations::SNACKBAR_CAPTION,
			test_scene_image_id::SNACKBAR_CAPTION,
			render_layer::FOREGROUND_GLOWS,
			white,
			augs::sprite_special_effect::COLOR_WAVE
		);

		{
			auto& meta = get_test_flavour(flavours, test_point_markers::FFA_SPAWN);
			invariants::point_marker marker;
			marker.type = point_marker_type::FFA_SPAWN;
			meta.set(marker);
		}

		{
			auto& meta = get_test_flavour(flavours, test_point_markers::BOMB_DEFUSAL_SPAWN);
			invariants::point_marker marker;
			marker.type = point_marker_type::TEAM_SPAWN;

			components::marker marker_meta;
			marker_meta.associated_faction = faction_type::METROPOLIS;

			meta.set(marker);
			meta.set(marker_meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::BOMBSITE_A);
			invariants::box_marker marker;
			marker.type = area_marker_type::BOMBSITE_A;

			components::marker marker_meta;
			marker_meta.associated_faction = faction_type::RESISTANCE;

			meta.set(marker);
			meta.set(marker_meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::BOMBSITE_B);
			invariants::box_marker marker;
			marker.type = area_marker_type::BOMBSITE_B;

			components::marker marker_meta;
			marker_meta.associated_faction = faction_type::RESISTANCE;

			meta.set(marker);
			meta.set(marker_meta);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::BUY_AREA);

			invariants::box_marker marker;
			marker.type = area_marker_type::BUY_AREA;
			meta.set(marker);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::T_SPAWN);

			invariants::box_marker marker;
			marker.type = area_marker_type::CALLOUT;
			meta.set(marker);
			meta.get<invariants::sorting_order>().order = static_cast<sorting_order_type>(test_marker_order::CALLOUT_NESTED);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::CT_SPAWN);

			invariants::box_marker marker;
			marker.type = area_marker_type::CALLOUT;
			meta.set(marker);
			meta.get<invariants::sorting_order>().order = static_cast<sorting_order_type>(test_marker_order::CALLOUT_NESTED);
		}

		{
			auto& meta = get_test_flavour(flavours, test_box_markers::ROOM);

			invariants::box_marker marker;
			marker.type = area_marker_type::CALLOUT;
			meta.set(marker);
		}


		{
			auto& meta = get_test_flavour(flavours, test_tool_items::ELECTRIC_ARMOR);

			test_flavours::add_sprite(meta, in.caches, test_scene_image_id::ELECTRIC_ARMOR);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("5");
			item.categories_for_slot_compatibility = { item_category::GENERAL, item_category::TORSO_ARMOR };
			item.standard_price = static_cast<money_type>(1800);

			meta.template get<invariants::fixtures>().density *= 2.f;
			meta.template get<invariants::fixtures>().restitution = 0.7f;

			meta.set(item);

			invariants::tool tool;
			tool.pe_absorption.hp = 0.3f;
			tool.pe_absorption.cp = 6.f;
			tool.glow_color = turquoise;
			tool.glow_color.a = 210;
			tool.movement_speed_mult = 0.85f;
			meta.set(tool);
		}

		{
			auto& meta = get_test_flavour(flavours, test_tool_items::DEFUSE_KIT);

			test_flavours::add_sprite(meta, in.caches, test_scene_image_id::DEFUSE_KIT);
			test_flavours::add_lying_item_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("30");
			item.categories_for_slot_compatibility = { item_category::GENERAL, item_category::BELT_WEARABLE };
			item.specific_to = faction_type::METROPOLIS;
			item.standard_price = static_cast<money_type>(2800);

			meta.template get<invariants::fixtures>().density /= 5.f;
			meta.template get<invariants::fixtures>().restitution = 0.7f;

			meta.set(item);

			invariants::tool tool;
			tool.defusing_speed_mult = 2.f;
			meta.set(tool);
		}

		{
			auto& meta = flavour_with_sprite(
				test_plain_sprited_bodies::DETACHED_RESISTANCE_HEAD,
				test_scene_image_id::RESISTANCE_HEAD,
				render_layer::ITEMS_ON_GROUND
			);

			auto& f = test_flavours::add_lying_item_dynamic_body(meta);
			f.density *= 2.5;
			f.restitution *= 2.;
		}

		{
			auto& meta = flavour_with_sprite(
				test_plain_sprited_bodies::DETACHED_METROPOLIS_HEAD,
				test_scene_image_id::METROPOLIS_HEAD,
				render_layer::ITEMS_ON_GROUND
			);

			auto& f = test_flavours::add_lying_item_dynamic_body(meta);
			f.density *= 2.5;
			f.restitution *= 2.;
		}
	}
}
