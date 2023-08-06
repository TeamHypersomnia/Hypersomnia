#pragma once
#include "application/setups/editor/to_game_effect.hpp"

template <class R, class F>
void allocate_flavours_and_assets_for_resource(
	const R& resource,
	const bool is_official,
	all_viewables_defs& viewables,
	cosmos_common_significant& common,
	F resolve_path
) {
	const auto& editable = resource.editable;

	if constexpr(std::is_same_v<editor_material_resource, R>) {
		auto& definitions = common.logical_assets.physical_materials;
		resource.scene_asset_id = definitions.allocate().key;
	}
	else if constexpr(std::is_same_v<editor_light_resource, R>) {
		using entity_type = static_light;

		auto& flavour_pool = common.flavours.get_for<entity_type>();
		resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
	}
	else if constexpr(std::is_same_v<editor_particles_resource, R>) {
		auto& definitions = viewables.particle_effects;
		resource.scene_asset_id = definitions.allocate().key;

		using entity_type = particles_decoration;

		auto& flavour_pool = common.flavours.get_for<entity_type>();
		resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
	}
	else if constexpr(std::is_same_v<editor_wandering_pixels_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_point_marker_resource, R>) {
		using entity_type = point_marker;

		auto& flavour_pool = common.flavours.get_for<point_marker>();
		resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
	}
	else if constexpr(std::is_same_v<editor_area_marker_resource, R>) {
		if (::is_portal_based(editable.type)) {

		}
		else {
			using entity_type = area_marker;

			auto& flavour_pool = common.flavours.get_for<entity_type>();
			resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
		}
	}
	else if constexpr(std::is_same_v<editor_prefab_resource, R>) {
		using entity_type = area_marker;

		auto& flavour_pool = common.flavours.get_for<area_marker>();
		resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
	}
	else if constexpr(std::is_same_v<editor_game_mode_resource, R>) {
		/* Nothing to do for game modes */
	}
	else if constexpr(std::is_same_v<editor_firearm_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_ammunition_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_tool_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_melee_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_explosive_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_sprite_resource, R>) {
		const auto domain = editable.domain;
		const int n_frames = resource.animation_frames.size();

		auto create_as = [&]<typename entity_type>(entity_type) {
			auto& flavour_pool = common.flavours.get_for<entity_type>();
			auto& image_definitions = viewables.image_definitions;
			auto& animations = common.logical_assets.plain_animations;
			
			{
				const auto new_raw_flavour_id = flavour_pool.allocate().key;
				const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

				resource.scene_flavour_id = new_flavour_id;
			}

			if (n_frames > 1) {
				const auto [new_animation_id, new_animation] = animations.allocate();
				new_animation.frames.resize(n_frames);

				{
					resource.scene_animation_id = new_animation_id;
				}

				for (int i = 0; i < n_frames; ++i) {
					const auto [new_image_id, new_definition] = image_definitions.allocate();
					new_animation.frames[i].image_id = new_image_id;
					new_animation.frames[i].duration_milliseconds = resource.animation_frames[i];

					if (!resource.scene_asset_id.is_set()) {
						resource.scene_asset_id = new_image_id;
					}

					/*
						TODO: deprecate get_unofficial_content_dir when old maps are gone!!!!
						This way we won't have to specify RESOLVED flag here.
					*/

					new_definition.source_image.path = augs::path_type(GENERATED_FILES_DIR) / resolve_path(resource.external_file.path_in_project);
					new_definition.source_image.path += typesafe_sprintf(".%x.png", i);
					new_definition.source_image.is_official = maybe_official_image_path::RESOLVED;

					new_definition.meta.extra_loadables.generate_neon_map = resource.editable.neon_map;
					new_definition.meta.offsets.non_standard_shape = resource.editable.as_physical.custom_shape;

					auto& meta = new_definition.meta;
					(void)meta;
				}
			}
			else {
				const auto [new_image_id, new_definition] = image_definitions.allocate();
				resource.scene_asset_id = new_image_id;
				resource.scene_animation_id = {};

				/* 
					Note that this is later resolved with get_unofficial_content_dir.
					get_unofficial_content_dir could really be depreciated,
						it existed because an intercosm always had relative paths since it was saved.
						Now we won't need it because an intercosm will always exist in memory only.
						So we could store absolute paths in the intercosmos.

						However for backwards compatibility with old maps we're using get_unofficial_content_dir for now.

					Even if we cache things in some .cache folder per-project,
					I think we'd be better off just caching binary representation of the project data instead of intercosms.
				*/

				new_definition.source_image.path = resource.external_file.path_in_project;
				new_definition.source_image.is_official = is_official;
				new_definition.meta.extra_loadables.generate_neon_map = resource.editable.neon_map;
				new_definition.meta.offsets.non_standard_shape = resource.editable.as_physical.custom_shape;

				auto& meta = new_definition.meta;
				(void)meta;
			}
		};

		if (domain == editor_sprite_domain::PHYSICAL) {
			create_as(plain_sprited_body());
		}
		else {
			if (n_frames > 1) {
				create_as(dynamic_decoration());
			}
			else {
				create_as(static_decoration());
			}
		}
	}
	else if constexpr(std::is_same_v<editor_sound_resource, R>) {
		auto create_as = [&]<typename entity_type>(entity_type) {
			auto& flavour_pool = common.flavours.get_for<entity_type>();
			auto& definitions = viewables.sounds;

			const auto [new_sound_id, new_definition] = definitions.allocate();

			{
				new_definition.source_sound.path = resource.external_file.path_in_project;
				new_definition.source_sound.is_official = is_official;

				auto& meta = new_definition.meta;
				(void)meta;
			}

			const auto new_raw_flavour_id = flavour_pool.allocate();
			const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

			resource.scene_flavour_id = new_flavour_id;
			resource.scene_asset_id = new_sound_id;
		};

		create_as(sound_decoration());
	}
	else {
		static_assert(always_false_v<R>, "Non-exhaustive if constexpr");
	}
}

template <class A, class F, class R, class S>
void setup_scene_object_from_resource(
	A get_asset_id_of,
	F find_resource,
	const R& resource,
	S& scene
) {
	const auto& editable = resource.editable;

	auto to_game_effect = [&](const auto& in) {
		return ::convert_to_game_effect(get_asset_id_of, find_resource, in);
	};

	if constexpr(std::is_same_v<editor_material_resource, R>) {
		scene.unit_damage_for_effects = editable.unit_damage_for_effects;
		scene.standard_damage_sound = to_game_effect(editable.damage_sound);
		scene.standard_damage_particles = to_game_effect(editable.damage_particles);
		scene.silence_damager_impact_sound = editable.silence_damager_impact_sound;
		scene.silence_damager_destruction_sound = editable.silence_damager_destruction_sound;

		auto to_game_collision_def = [&to_game_effect](const auto& from) {
			collision_sound_def out;
			out.effect = to_game_effect(from.sound);
			out.particles = to_game_effect(from.particles);
			out.pitch_bound.set(from.min_pitch, from.max_pitch);
			out.collision_sound_sensitivity = from.collision_sound_sensitivity;
			out.unmute_after_ms = from.unmute_after_ms;
			out.mute_after_playing_times = from.mute_after_playing_times;
			out.min_interval_ms = from.min_interval_ms;
			out.silence_opposite_collision_sound = from.silence_opposite_collision_sound;

			return out;
		};

		scene.default_collision = to_game_collision_def(editable.default_collision);

		for (auto& entry : editable.specific_collisions) {
			const auto mapped_id = get_asset_id_of(entry.collider);

			scene.collision_sound_matrix[mapped_id] = to_game_collision_def(entry);

			if (mapped_id == to_physical_material_id(test_scene_physical_material_id::CHARACTER)) {
				/* Blank so must override */
				scene.collision_sound_matrix[mapped_id].silence_opposite_collision_sound = true;
			}
		}
	}
	else if constexpr(std::is_same_v<editor_particles_resource, R>) {
		if constexpr(std::is_same_v<R, particle_effect>) {
			// TODO: we'll have to call introspective assign here

			scene.emissions = editable.emissions;
			scene.name = resource.get_display_name();
		}
		else {
			auto& particles = scene.template get<invariants::continuous_particles>();
			particles.effect_id = resource.scene_asset_id;
			/*
				Keep invariant modifier at default.
				Why?
				The continuous_particles flavour will only be used to spawn nodes and we want to
				customize modifiers on a per-entity basis.

				(We actually removed effect modifier from the invariant.)
			*/
			//particles.effect.modifier = static_cast<particle_effect_modifier>(editable);
		}
	}
	else if constexpr(std::is_same_v<editor_wandering_pixels_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_point_marker_resource, R>) {
		auto& marker = scene.template get<invariants::point_marker>();
		marker.type = resource.editable.type;
	}
	else if constexpr(std::is_same_v<editor_area_marker_resource, R>) {
		auto& marker = scene.template get<invariants::area_marker>();
		marker.type = resource.editable.type;
	}
	else if constexpr(std::is_same_v<editor_prefab_resource, R>) {
		auto& marker = scene.template get<invariants::area_marker>();
		marker.type = area_marker_type::PREFAB;
	}
	else if constexpr(std::is_same_v<editor_game_mode_resource, R>) {
		/* Nothing to do for game modes */
	}
	else if constexpr(std::is_same_v<editor_firearm_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_ammunition_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_tool_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_melee_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_explosive_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_sprite_resource, R>) {
		const auto domain = editable.domain;

		auto on_domain_specific = [&](auto callback) {
			if (domain == editor_sprite_domain::PHYSICAL) {
				callback(editable.as_physical);
			}
			else {
				callback(editable.as_nonphysical);
			}
		};

		{
			auto& render = scene.template get<invariants::render>();

			render.layer = [&]() {
				switch (domain) {
					case editor_sprite_domain::BACKGROUND:
						return render_layer::GROUND;
					case editor_sprite_domain::FOREGROUND:
						if (editable.as_nonphysical.full_illumination) {
							return render_layer::FOREGROUND_GLOWS;
						}

						return render_layer::FOREGROUND;
					case editor_sprite_domain::PHYSICAL:
						if (editable.as_physical.is_shoot_through) {
							return render_layer::OBSTACLES_UNDER_MISSILES;
						}

						return render_layer::SOLID_OBSTACLES;

					default:
						return render_layer::GROUND;

				}
			}();

			on_domain_specific([&](auto& specific) {
				render.special_functions.set(special_render_function::ILLUMINATE_AS_WALL, specific.illuminate_like_wall);

				if (domain == editor_sprite_domain::FOREGROUND) {
					render.special_functions.set(special_render_function::COVER_GROUND_NEONS, false);
					render.special_functions.set(special_render_function::COVER_GROUND_NEONS_FOREGROUND, specific.cover_background_neons);
				}
				else {
					render.special_functions.set(special_render_function::COVER_GROUND_NEONS, specific.cover_background_neons);
					render.special_functions.set(special_render_function::COVER_GROUND_NEONS_FOREGROUND, false);
				}
			});
		}

		{
			auto& sprite = scene.template get<invariants::sprite>();
			sprite.set(resource.scene_asset_id, editable.size, editable.color);
			sprite.tile_excess_size = !editable.stretch_when_resized;
			sprite.neon_color = editable.neon_color;

			if (editable.color_wave_speed.is_enabled) {
				sprite.effect = augs::sprite_special_effect::COLOR_WAVE;
				sprite.effect_speed_multiplier = editable.color_wave_speed.value;
			}

			// TODO: currently one overrides the other, make it a boolset!

			if (editable.rotate_continuously_degrees_per_sec.is_enabled) {
				sprite.effect = augs::sprite_special_effect::CONTINUOUS_ROTATION;
				sprite.effect_speed_multiplier = editable.rotate_continuously_degrees_per_sec.value / 360.0f;
			}

			sprite.neon_alpha_vibration = editable.neon_alpha_vibration;
			sprite.vibrate_diffuse_too = editable.vibrate_diffuse_too;
		}

		if (auto animation = scene.template find<invariants::animation>()) {
			animation->id = resource.scene_animation_id; 
		}

		auto& physical = editable.as_physical;
		auto& nonphysical = editable.as_nonphysical;

		if (auto rigid_body = scene.template find<invariants::rigid_body>()) {
			rigid_body->damping.linear = physical.linear_damping;
			rigid_body->damping.angular = physical.angular_damping;

			if (physical.is_static) {
				rigid_body->body_type = rigid_body_type::ALWAYS_STATIC;
			}
		}

		if (auto fixtures = scene.template find<invariants::fixtures>()) {
			if (const auto material = find_resource(physical.material)) {
				fixtures->material = material->scene_asset_id;

				/* 
					These should be read in runtime from material but caching like is is more performant,
					since realistically materials of simple walls will never change (which is why they're specified in an INVARIANT::fixtures).
				*/

				fixtures->max_ricochet_angle = material->editable.max_ricochet_angle;
				fixtures->point_blank_ricochets = material->editable.point_blank_ricochets;
				fixtures->penetrability = material->editable.penetrability;
			}

			fixtures->penetrability *= physical.penetrability;

			fixtures->density = physical.density;
			fixtures->friction = physical.friction;
			fixtures->restitution = physical.bounciness;

			if (physical.is_see_through) {
				fixtures->filter = filters[predefined_filter_type::GLASS_OBSTACLE];
			}
			else {
				fixtures->filter = filters[predefined_filter_type::WALL];
			}

			if (physical.is_walk_through) {
				fixtures->filter.maskBits &= ~(1 << int(filter_category::CHARACTER));
				fixtures->filter.maskBits &= ~(1 << int(filter_category::CHARACTER_WEAPON));

				fixtures->filter.maskBits &= ~(1 << int(filter_category::WALL));
				fixtures->filter.maskBits &= ~(1 << int(filter_category::LYING_ITEM));
				fixtures->filter.maskBits &= ~(1 << int(filter_category::SHELL));
			}

			if (physical.is_throw_through) {
				fixtures->filter.maskBits &= ~(1 << int(filter_category::FLYING_EXPLOSIVE));
			}

			if (physical.is_melee_throw_through) {
				fixtures->filter.maskBits &= ~(1 << int(filter_category::FLYING_MELEE));
			}

			if (physical.is_shoot_through) {
				fixtures->filter.maskBits &= ~(1 << int(filter_category::FLYING_BULLET));
				fixtures->filter.maskBits &= ~(1 << int(filter_category::CHARACTER_WEAPON));
			}

			fixtures->collision_sound_sensitivity = physical.collision_sound_sensitivity;
		}

		if (domain == editor_sprite_domain::BACKGROUND) {
			if (auto ground = scene.template find<invariants::ground>()) {
				if (nonphysical.custom_footstep.is_enabled) {
					ground->footstep_effect.is_enabled = true;
					ground->footstep_effect.value.sound = to_game_effect(nonphysical.custom_footstep.value.sound);

					ground->movement_speed_mult = nonphysical.custom_footstep.value.walking_speed;
				}
			}
		}
	}
	else if constexpr(std::is_same_v<editor_sound_resource, R>) {
		auto& sound = scene.template get<invariants::continuous_sound>();
		sound.effect.id = resource.scene_asset_id;
		sound.effect.modifier = to_game_effect(static_cast<const editor_sound_effect_modifier&>(editable));
	}
	else if constexpr(std::is_same_v<editor_light_resource, R>) {
		/*
			We could theoretically setup defaults for the node here,
			but on the other hand this could be handled by setup_node_defaults

			auto& light = scene.template get<components::light>();
			light = editable;

			//scene.template get<invariants::light>().is_new_light_attenuation = 1;
		*/
	}
	else {
		static_assert(always_false_v<R>, "Non-exhaustive if constexpr");
	}
}

template <class A, class F, class R, class N>
void setup_per_node_flavour(
	A get_asset_id_of,
	F find_resource,
	const R& resource,
	const N& node,
	cosmos_common_significant& common
) {
	const auto& editable = node.editable;

	auto to_game_effect = [&](const auto& in) {
		return ::convert_to_game_effect(get_asset_id_of, find_resource, in);
	};

	if constexpr(std::is_same_v<editor_area_marker_node, N>) {
		if (is_portal_based(resource.editable.type)) {
			using entity_type = area_sensor;

			auto& flavour_pool = common.flavours.get_for<entity_type>();
			auto allocated = flavour_pool.allocate();

			node.custom_scene_flavour_id = typed_entity_flavour_id<entity_type>(allocated.key);

			auto& scene = allocated.object;

			const auto& from = editable.as_portal;

			const auto radius = float(editable.size.smaller_side()) / 2;

			if (!from.disable_all_entry_effects) {
				if (const auto particles = scene.template find<invariants::continuous_particles>()) {
					particles->effect_id = get_asset_id_of(from.ambience_particles.id);
				}

				if (auto sound = scene.template find<invariants::continuous_sound>()) {
					sound->effect = to_game_effect(from.ambience_sound);
					sound->effect.modifier.repetitions = -1;
					sound->effect.modifier.max_distance = radius * from.ambience_sound_distance_mult;
					sound->effect.modifier.reference_distance = sound->effect.modifier.max_distance / 1.5f;
				}
			}
			
			auto& marker = scene.template get<invariants::area_marker>();
			marker.type = resource.editable.type;

			if (auto fixtures = scene.template find<invariants::fixtures>()) {
				/*
					It's an area sensor.
				*/

				fixtures->filter = filters[predefined_filter_type::PORTAL];
				fixtures->disable_standard_collision_resolution = true;
			}

			if (auto rigid_body = scene.template find<invariants::rigid_body>()) {
				rigid_body->body_type = rigid_body_type::ALWAYS_STATIC;
			}

			if (from.context_tip.is_enabled) {
				scene.template get<invariants::text_details>().description = from.context_tip.value;
			}
		}
	}
}
