#pragma once
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/packaged_official_content.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/typed_entity_handle.h"
#include "game/cosmos/for_each_entity.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/create_name_to_id_map.hpp"
#include "game/enums/filters.h"
#include "application/setups/editor/editor_official_resource_map.hpp"
#include "application/setups/editor/defaults/editor_node_defaults.h"
#include "application/setups/editor/defaults/editor_resource_defaults.h"

#include "augs/string/string_templates.h"
#include "augs/string/string_to_enum.h"
//#include "application/setups/debugger/detail/find_locations_that_use.h"

inline auto to_test_scene_physical_material_id(const assets::physical_material_id id) {
	return static_cast<test_scene_physical_material_id>(id.indirection_index);
}

void export_intercosm_to_json(const debugger_command_input cmd_in) {
	const auto paths = cmd_in.folder.get_paths();
	const auto output_path = paths.json_export_file;

	const auto& folder = cmd_in.folder;

	const auto& csm = folder.commanded->work.world;
	const auto& viewables = folder.commanded->work.viewables;

	const auto official = packaged_official_content(cmd_in.lua);

	editor_project project;

	project.settings.ambient_light_color = csm.get_common_significant().light.ambient_color;

	std::unordered_map<entity_flavour_id, editor_typed_resource_id<editor_sprite_resource>> sprites;
	std::unordered_map<entity_flavour_id, editor_typed_resource_id<editor_sound_resource>> sounds;

	const auto id_to_name_official = official.resource_map.create_name_to_id_map();

#if 0
	const auto in_locations = candidate_id_locations {
		folder.commanded->work.world, 
		folder.commanded->work.viewables, 
		folder.commanded->rulesets
	};
#endif

	auto is_used = [&](const auto) {
		return true;
	};

	{
		auto migrate_sprite = [&](auto& to, const auto& from) {
			to.color = from.color;
			to.neon_color = from.neon_color;
			to.size = from.size;
			to.color_wave_speed.value = from.effect_speed_multiplier;
			to.rotate_continuously_degrees_per_sec.value = from.effect_speed_multiplier;
			to.stretch_when_resized = !from.tile_excess_size;

			if (from.has_color_wave()) {
				to.color_wave_speed.is_enabled = true;
			}

			if (from.effect == augs::sprite_special_effect::CONTINUOUS_ROTATION) {
				to.rotate_continuously_degrees_per_sec.is_enabled = true;
			}

			to.neon_alpha_vibration = from.neon_alpha_vibration;
			to.stretch_when_resized = !from.tile_excess_size;
		};

		auto migrate_render = [&](auto& to, const auto& from) {
			if (from.layer == render_layer::FOREGROUND_GLOWS) {
				to.as_nonphysical.full_illumination = true;
			}

			if (from.layer < render_layer::SENTIENCES) {
				to.domain = editor_sprite_domain::BACKGROUND;
			}
			else {
				to.domain = editor_sprite_domain::FOREGROUND;
			}
		};

		auto migrate_ground = [&](auto& to, const auto& from) {
			if (from.movement_speed_mult != 1.0f || from.footstep_effect.is_enabled) {
				to.custom_footstep.is_enabled = true;
				to.custom_footstep.value.walking_speed = from.movement_speed_mult;

				to.custom_footstep.value.sound.gain = from.footstep_effect.value.sound.modifier.gain;
				to.custom_footstep.value.sound.pitch = from.footstep_effect.value.sound.modifier.pitch;
			}
		};

		auto migrate_rigid_body = [&](auto& to, const auto& from) {
			to.linear_damping = from.damping.linear;
			to.angular_damping = from.damping.angular;

			if (from.body_type == rigid_body_type::ALWAYS_STATIC || from.body_type == rigid_body_type::STATIC) {
				to.is_static = true;
			}
		};

		auto migrate_fixtures = [&](auto& to, const auto& from) {
			to.density = from.density;
			to.friction = from.friction;
			to.bounciness = from.restitution;

			if (from.filter == filters[predefined_filter_type::GLASS_OBSTACLE]) {
				to.is_see_through = true;
			}

			{
				auto wall = filters[predefined_filter_type::WALL];
				wall.maskBits &= ~(1 << int(filter_category::FLYING));

				if (wall == from.filter) {
					to.is_throw_through = true;
				}

				auto glass = filters[predefined_filter_type::GLASS_OBSTACLE];
				glass.maskBits &= ~(1 << int(filter_category::FLYING));

				if (glass == from.filter) {
					to.is_throw_through = true;
				}
			}

			to.is_shoot_through = from.bullets_fly_through;
			to.collision_sound_strength_mult = from.collision_sound_strength_mult;

			if (const auto found = mapped_or_nullptr(official.resource_map.materials, to_test_scene_physical_material_id(from.material))) {
				to.material = *found;
			}
		};

		auto do_full_modifier = [&](auto& to, auto& from) {
			to.gain = from.gain;
			to.pitch = from.pitch;

			if (from.max_distance != -1) {
				to.max_distance.emplace(from.max_distance);
			}

			if (from.reference_distance != -1) {
				to.reference_distance.emplace(from.reference_distance);
			}

			if (from.distance_model != augs::distance_model::NONE) {
				to.distance_model.emplace(from.distance_model);
			}

			to.spatialize = !from.always_direct_listener;

			if (from.repetitions == -1) {
				to.loop = true;
			}
			else {
				to.play_times = from.repetitions;
			}
		};

		csm.for_each_flavour_and_id<sound_decoration>(
			[&](const auto& flavour_id, const auto& flavour) {
				if (!is_used(flavour_id)){ 
					return;
				}

				const auto& sound = flavour.template get<invariants::continuous_sound>();

				const auto& viewable = viewables.sounds[sound.effect.id];
				const auto path = viewable.source_sound;

				LOG("sound_decoration %x %x", flavour.get_name(), path.path);

				if (path.is_official) {
					if (const auto found_enum = mapped_or_nullptr(augs::get_string_to_enum_map<test_sound_decorations>(), to_uppercase(path.path.stem()))) {
						sounds[flavour_id] = official.resource_map[*found_enum];
						LOG("Found official");
						return;
					}
				}

				const auto pathed = editor_pathed_resource(path.path, "", {});

				auto resource = editor_sound_resource(pathed);
				auto& editable = resource.editable;
				::setup_resource_defaults(editable, official.resource_map);

				do_full_modifier(editable, sound.effect.modifier);

				auto new_id = project.resources.pools.get_for<editor_sound_resource>().allocate(resource).key;
				auto new_typed_id = editor_typed_resource_id<editor_sound_resource>::from_raw(new_id, false);

				sounds[flavour_id] = new_typed_id;
			}
		);

		csm.for_each_flavour_and_id<static_decoration>(
			[&](const auto& flavour_id, const auto& flavour) {
				if (!is_used(flavour_id)){ 
					return;
				}
				const auto& sprite = flavour.template get<invariants::sprite>();

				const auto& viewable = viewables.image_definitions[sprite.image_id];
				const auto path = viewable.source_image;

				LOG("static_decoration %x %x", flavour.get_name(), path.path);

				if (flavour.get_name().empty()) {
					return;
				}

				if (path.is_official) {
					if (const auto found_enum = mapped_or_nullptr(augs::get_string_to_enum_map<test_static_decorations>(), to_uppercase(path.path.stem()))) {
						sprites[flavour_id] = official.resource_map[*found_enum];
						LOG("Found official");
						return;
					}
				}


				const auto pathed = editor_pathed_resource(path.path, "", {});

				auto resource = editor_sprite_resource(pathed);
				auto& editable = resource.editable;
				::setup_resource_defaults(editable, official.resource_map);
				editable.neon_map = viewable.meta.extra_loadables.generate_neon_map;

				const auto& ground = flavour.template get<invariants::ground>();
				const auto& render = flavour.template get<invariants::render>();

				editable.as_nonphysical.illuminate_like_wall = render.special_functions.test(special_render_function::ILLUMINATE_AS_WALL);
				editable.as_nonphysical.cover_background_neons = render.special_functions.test(special_render_function::COVER_GROUND_NEONS);

				migrate_render(editable, render);
				migrate_sprite(editable, sprite);
				migrate_ground(editable.as_nonphysical, ground);

				auto new_id = project.resources.pools.get_for<editor_sprite_resource>().allocate(resource).key;
				auto new_typed_id = editor_typed_resource_id<editor_sprite_resource>::from_raw(new_id, false);

				sprites[flavour_id] = new_typed_id;
			}
		);

		csm.for_each_flavour_and_id<plain_sprited_body>(
			[&](const auto& flavour_id, const auto& flavour) {
				if (!is_used(flavour_id)){ 
					return;
				}
				const auto& sprite = flavour.template get<invariants::sprite>();

				const auto& viewable = viewables.image_definitions[sprite.image_id];
				const auto path = viewable.source_image;

				LOG("plain_sprited_body %x %x", flavour.get_name(), path.path);

				if (flavour.get_name().empty()) {
					return;
				}

				if (path.is_official) {
					if (const auto found_enum = mapped_or_nullptr(augs::get_string_to_enum_map<test_static_decorations>(), to_uppercase(path.path.stem()))) {
						sprites[flavour_id] = official.resource_map[*found_enum];
						LOG("Found official");
						return;
					}
				}

				const auto pathed = editor_pathed_resource(path.path, "", {});

				auto resource = editor_sprite_resource(pathed);
				auto& editable = resource.editable;
				::setup_resource_defaults(editable, official.resource_map);
				editable.neon_map = viewable.meta.extra_loadables.generate_neon_map;
				editable.as_physical.custom_shape = viewable.meta.offsets.non_standard_shape;

				const auto& rigid_body = flavour.template get<invariants::rigid_body>();
				const auto& render = flavour.template get<invariants::render>();
				const auto& fixtures = flavour.template get<invariants::fixtures>();

				editable.as_physical.illuminate_like_wall = render.special_functions.test(special_render_function::ILLUMINATE_AS_WALL);
				editable.as_physical.cover_background_neons = render.special_functions.test(special_render_function::COVER_GROUND_NEONS);

				migrate_rigid_body(editable.as_physical, rigid_body);
				migrate_fixtures(editable.as_physical, fixtures);
				migrate_sprite(editable, sprite);
				migrate_render(editable, render);

				editable.domain = editor_sprite_domain::PHYSICAL;

				auto new_id = project.resources.pools.get_for<editor_sprite_resource>().allocate(resource).key;
				auto new_typed_id = editor_typed_resource_id<editor_sprite_resource>::from_raw(new_id, false);

				sprites[flavour_id] = new_typed_id;
			}
		);
	}

	const auto fallback_sprite_flav = official.resource_map[test_static_decorations::AQUARIUM_SAND_1];
	const auto fallback_sound_flav = official.resource_map[test_sound_decorations::FOOTSTEP_FLOOR];

	std::unordered_map<std::string, int> names;

	csm.for_each_entity(
		[&]<typename H>(const H& h) {
			using E = typename H::used_entity_type;

			const auto transform = h.get_logic_transform();
			const auto flavour_id = h.get_flavour_id();

			auto orig_name = h.get_name();
			auto name = orig_name;

			if (names[orig_name] > 0) {
				name = orig_name + typesafe_sprintf(" (%x)", names[orig_name]);
			}

			names[orig_name]++;

			if constexpr(is_one_of_v<E, static_decoration, plain_sprited_body, dynamic_decoration>) {
				editor_sprite_node node;
				node.resource_id = sprites[flavour_id];

				if (!node.resource_id.is_set()) {
					node.resource_id = fallback_sprite_flav;
				}

				auto& to = node.editable;

				{
					if (const auto geo = h.get().template find<components::overridden_geo>()) {

						if (geo->size.is_enabled) {
							to.size = geo->size.value;
						}

						to.flip_horizontally = geo->flip.horizontally;
						to.flip_vertically = geo->flip.vertically;
					}
				}

				{
					auto sprite = h.template get<components::sprite>();

					to.color = sprite.colorize;
					to.neon_color = sprite.colorize_neon;
				}

				to.pos = transform.pos;
				to.rotation = transform.rotation;

				const auto& sorting_order = h.template get<components::sorting_order>();
				LOG_NVPS(name, sorting_order.order);
				const auto considered_order = uint32_t(h.template get<invariants::render>().layer) * 100000 + sorting_order.order;
				node.chronological_order = std::numeric_limits<uint32_t>::max() - considered_order;

				node.unique_name = name;
				project.nodes.pools.get_for<editor_sprite_node>().allocate(node);
			}
			else if constexpr(std::is_same_v<E, sound_decoration>) {
				editor_sound_node node;
				node.resource_id = sounds[flavour_id];

				if (!node.resource_id.is_set()) {
					node.resource_id = fallback_sound_flav;
				}

				auto& to = node.editable;

				to.pos = transform.pos;

				node.unique_name = name;
				project.nodes.pools.get_for<editor_sound_node>().allocate(node);
			}
			else if constexpr(std::is_same_v<E, static_light>) {
				editor_light_node node;
				node.resource_id = official.resource_map[test_static_lights::POINT_LIGHT];

				auto& to = node.editable;

				const auto& light = h.template get<components::light>();

				to.pos = transform.pos;
				to.color = light.color;
				to.falloff.radius = light.calc_reach_trimmed().bigger_side() / 10;

				node.unique_name = name;
				project.nodes.pools.get_for<editor_light_node>().allocate(node);
			}
			else {
				editor_sprite_node node;
				node.resource_id = fallback_sprite_flav;

				auto& to = node.editable;

				to.pos = transform.pos;
				to.size = h.get_logical_size();
				to.color = white;
				to.rotation = transform.rotation;

				node.unique_name = name;
				project.nodes.pools.get_for<editor_sprite_node>().allocate(node);
			}

			return callback_result::CONTINUE;
		}
	);

	{

		editor_project_readwrite::write_project_json(
			output_path,
			project,
			official.resources,
			official.resource_map
		);
	}
}
