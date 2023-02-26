#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"

template <class N, class R, class H, class A>
void setup_entity_from_node(
	const sorting_order_type total_order,
	const editor_layer& layer,
	const N& node, 
	const R& resource,
	H& handle, 
	A& agg
) {
	using Editable = decltype(node.editable);
	auto& editable = node.editable;

	if (auto sorting_order = agg.template find<components::sorting_order>()) {
		sorting_order->order = total_order;
	}
	
	ensure(agg.when_born.step == 1);

	if (!layer.editable.selectable_on_scene) {
		agg.when_born.step = 2;
	}

	if constexpr(std::is_same_v<N, editor_sprite_node>) {
		auto& sprite = agg.template get<components::sprite>();
		sprite.colorize = editable.colorize;
		sprite.colorize *= layer.editable.tint;

		const auto opacity = layer.editable.opacity;

		if (opacity != 1.0f) {
			sprite.colorize.mult_alpha(opacity);
		}
	}
	else if constexpr(std::is_same_v<N, editor_light_node>) {
		auto set_attn_from_falloff = [&](auto& attn, const auto& foff) {
			const auto mult = foff.calc_attenuation_mult_for_requested_radius();

			attn.constant = foff.constant * mult * CONST_MULT;
			attn.linear = foff.linear * mult * LINEAR_MULT;
			attn.quadratic = foff.quadratic * mult * QUADRATIC_MULT;

			attn.trim_alpha = foff.cutoff_alpha;
			// LOG_NVPS(foff.radius, attn.calc_reach(), attn.calc_reach_trimmed());
		};

		auto& light = agg.template get<components::light>();
		light.color *= editable.colorize;

		set_attn_from_falloff(light.attenuation, node.editable.falloff);

		{
			auto& wall_foff = node.editable.wall_falloff;

			if (wall_foff.is_enabled) {
				set_attn_from_falloff(light.wall_attenuation, wall_foff.value);
			}
			else {
				light.wall_attenuation = light.attenuation;
			}
		}

		light.wall_variation = light.variation;

		{
			const float positional_vibration = node.editable.positional_vibration;
			const float intensity_vibration = node.editable.intensity_vibration;

			light.variation.is_enabled = intensity_vibration > 0.0f;
			light.wall_variation.is_enabled = intensity_vibration > 0.0f;

			light.position_variations.is_enabled = positional_vibration > 0.0f;

			for (auto& pv : light.position_variations.value) {
				pv *= positional_vibration;
			}

			light.variation.value *= intensity_vibration;
			light.wall_variation.value *= intensity_vibration;
		}

	}
	else if constexpr(std::is_same_v<N, editor_wandering_pixels_node>) {
		auto& wandering_pixels = agg.template get<components::wandering_pixels>();
		wandering_pixels = static_cast<const components::wandering_pixels&>(editable);
	}
	else if constexpr(is_one_of_v<N, editor_point_marker_node, editor_area_marker_node>) {
		auto& marker = agg.template get<components::marker>();
		marker.faction = editable.faction;
		// TODO: add letter to the fields once we can freely modify cosmos ABI
	}

	if (auto geo = agg.template find<components::overridden_geo>()) {
		if constexpr(has_size_v<Editable>) {
			geo->size = editable.size;
		}

		if constexpr(has_flip_v<Editable>) {
			geo->flip.horizontally = editable.flip_horizontally;
			geo->flip.vertically = editable.flip_vertically;
		}
	}

	handle.set_logic_transform(node.get_transform());
	(void)resource;
}

template <class N, class R, class H>
void setup_entity_from_node_post_construct(
	const N& node, 
	const R& resource,
	H& handle
) {
	(void)resource;

	if constexpr(std::is_same_v<N, editor_sprite_node>) {
		if (auto anim = handle.template find<components::animation>()) {
			if (!node.editable.randomize_starting_animation_frame) {
				anim->state.frame_num = 0;
			}

			anim->speed_factor *= node.editable.animation_speed_factor;
		}
	}
}

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
		using entity_type = box_marker;

		auto& flavour_pool = common.flavours.get_for<box_marker>();
		resource.scene_flavour_id = typed_entity_flavour_id<entity_type>(flavour_pool.allocate().key);
	}
	else if constexpr(std::is_same_v<editor_firearm_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_ammunition_resource, R>) {
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

template <class A, class R, class S>
void setup_scene_object_from_resource(
	A get_asset_id_of,
	const R& resource,
	S& scene
) {
	const auto& editable = resource.editable;

	auto to_game_effect = [&]<typename T>(const T& in) {
		if constexpr(std::is_same_v<T, editor_sound_effect>) {
			sound_effect_input out;
			out.modifier = static_cast<sound_effect_modifier>(in);
			out.id = get_asset_id_of(in.resource_id);

			return out;
		}
		else if constexpr(std::is_same_v<T, editor_particle_effect>) {
			particle_effect_input out;
			out.modifier = static_cast<particle_effect_modifier>(in);
			out.id = get_asset_id_of(in.resource_id);

			return out;
		}
	};

	if constexpr(std::is_same_v<editor_material_resource, R>) {
		scene.unit_effect_damage = editable.unit_effect_damage;
		scene.standard_damage_sound = to_game_effect(editable.standard_damage_sound);
	}
	else if constexpr(std::is_same_v<editor_particles_resource, R>) {
		if constexpr(std::is_same_v<R, particle_effect>) {
			// TODO: we'll have to call introspective assign here

			scene.emissions = editable.emissions;
			scene.name = resource.get_display_name();
		}
		else {
			auto& particles = scene.template get<invariants::continuous_particles>();
			particles.effect.id = resource.scene_asset_id;
			particles.effect.modifier = static_cast<particle_effect_modifier>(editable);
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
		auto& marker = scene.template get<invariants::box_marker>();
		marker.type = resource.editable.type;
	}
	else if constexpr(std::is_same_v<editor_firearm_resource, R>) {
		ensure(false && "not implemented");
	}
	else if constexpr(std::is_same_v<editor_ammunition_resource, R>) {
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
						if (editable.foreground_glow) {
							return render_layer::FOREGROUND_GLOWS;
						}

						return render_layer::FOREGROUND;
					case editor_sprite_domain::PHYSICAL:
						return render_layer::SOLID_OBSTACLES;

					default:
						return render_layer::GROUND;

				}
			}();

			on_domain_specific([&](auto& specific) {
				render.special_functions.set(special_render_function::ILLUMINATE_AS_WALL, specific.illuminate_as_wall);
				render.special_functions.set(special_render_function::COVER_GROUND_NEONS, specific.cover_ground_neons);
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
		}

		if (auto animation = scene.template find<invariants::animation>()) {
			animation->id = resource.scene_animation_id; 
		}

		auto& physical = editable.as_physical;

		if (auto rigid_body = scene.template find<invariants::rigid_body>()) {
			rigid_body->damping.linear = physical.linear_damping;
			rigid_body->damping.angular = physical.angular_damping;

			if (physical.is_static) {
				rigid_body->body_type = rigid_body_type::ALWAYS_STATIC;
			}
		}

		if (auto fixtures = scene.template find<invariants::fixtures>()) {
			fixtures->density = physical.density;
			fixtures->friction = physical.friction;
			fixtures->restitution = physical.bounciness;

			if (physical.is_see_through) {
				fixtures->filter = filters[predefined_filter_type::GLASS_OBSTACLE];
			}
			else {
				fixtures->filter = filters[predefined_filter_type::WALL];
			}

			if (physical.is_throw_through) {
				fixtures->filter.maskBits &= ~(1 << int(filter_category::FLYING));
			}
		}
	}
	else if constexpr(std::is_same_v<editor_sound_resource, R>) {
		auto& sound = scene.template get<invariants::continuous_sound>();
		sound.effect.id = resource.scene_asset_id;
		sound.effect.modifier = static_cast<sound_effect_modifier>(editable);
	}
	else if constexpr(std::is_same_v<editor_light_resource, R>) {
		auto& light = scene.template get<components::light>();
		light = editable;

		//scene.template get<invariants::light>().is_new_light_attenuation = 1;
	}
	else {
		static_assert(always_false_v<R>, "Non-exhaustive if constexpr");
	}
}

void editor_setup::rebuild_scene() {
	scene = initial_scene;

	for (auto& s : scene_entity_to_node) {
		s.clear();
	}

	const auto mutable_access = cosmos_common_significant_access();
	auto& common = scene.world.get_common_significant(mutable_access);
	//common.light.ambient_color = rgba(180, 180, 180, 255);

	auto for_each_manually_specified_official_resource_pool = [&](auto lbd) {
		lbd(official_resources.get_pool_for<editor_point_marker_resource>());
		lbd(official_resources.get_pool_for<editor_area_marker_resource>());
	};

	/* 
		Establish identities. 
		First we have to create all flavour and asset ids so that we can properly setup references later.
	*/

	{
		auto allocate_flavours_and_assets = [&]<typename P>(const P& pool, const bool is_official) {
			for (const auto& resource : pool) {
				::allocate_flavours_and_assets_for_resource(
					resource,
					is_official,
					scene.viewables,
					common,
					[this](const auto& path) { return resolve_project_path(path); }
				);
			}
		};

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
		official_resources.for_each([&](const auto& pool) { allocate_flavours_and_assets(pool, true); } );
#else
		for_each_manually_specified_official_resource_pool([&](const auto& pool) { allocate_flavours_and_assets(pool, true); } );
#endif
		project.resources .for_each([&](const auto& pool) { allocate_flavours_and_assets(pool, false); } );
	}

	/* Create resources */

	auto setup_flavours_and_assets = [&]<typename P>(const P& pool) {
		auto get_asset_id_of = [&]<typename R>(const editor_typed_resource_id<R>& resource_id) {
			using asset_type = decltype(R::scene_asset_id);

			if (const auto resource = find_resource(resource_id)) {
				return resource->scene_asset_id;
			}

			return asset_type();
		};

		using R = typename P::value_type;

		for (const R& resource : pool) {
			auto setup_scene_object = [&](auto& scene_object) {
				::setup_scene_object_from_resource(
					get_asset_id_of,
					resource,
					scene_object
				);
			};

			if constexpr(std::is_same_v<R, editor_material_resource>) {
				auto& material = common.logical_assets.physical_materials.get(resource.scene_asset_id);
				setup_scene_object(material);
			}
			else {
				std::visit(
					[&]<typename E>(const typed_entity_flavour_id<E>& typed_flavour_id) {
						auto& flavour_pool = common.flavours.get_for<E>();
						auto& flavour = flavour_pool.get(typed_flavour_id.raw);

						setup_scene_object(flavour);
					},
					resource.scene_flavour_id
				);
			}
		}
	};


#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	official_resources.for_each(setup_flavours_and_assets);
#else
	for_each_manually_specified_official_resource_pool(setup_flavours_and_assets);
#endif
	project.resources .for_each(setup_flavours_and_assets);

	/* Create nodes */

	auto total_order = sorting_order_type(0);

	auto populate = [&](const logic_step step) {
		for (const auto layer_id : reverse(project.layers.order)) {
			auto layer = find_layer(layer_id);
			ensure(layer != nullptr);

			auto node_handler = [&]<typename node_type>(const node_type& typed_node, const auto node_id) {
				typed_node.scene_entity_id.unset();

				if (!typed_node.visible || !layer->visible) {
					return;
				}

				const auto resource = find_resource(typed_node.resource_id);

				if (resource == nullptr) {
					return;
				}

				auto setup_node_entity_mapping = [&](const auto new_entity_id) {
					const auto mapping_index = new_entity_id.get_type_id().get_index();
					auto& mapping = scene_entity_to_node[mapping_index];

					ensure_eq(decltype(new_entity_id.raw.version)(1), new_entity_id.raw.version);

					while (mapping.size() <= std::size_t(new_entity_id.raw.indirection_index)) {
						mapping.emplace_back();
					}

					mapping.back() = node_id.operator editor_node_id();
					typed_node.scene_entity_id = new_entity_id;
				};

				if constexpr(std::is_same_v<node_type, editor_firearm_node>) {
					std::visit(
						[&](const auto& typed) {
							requested_equipment r;
							r.weapon = typed;
							r.num_given_ammo_pieces = 1;

							auto new_id = r.generate_for(typed_node.get_transform(), step);
							ensure(scene.world[new_id].alive());
							setup_node_entity_mapping(new_id);
						},

						resource->scene_flavour_id
					);
				}
				else if constexpr(is_one_of_v<node_type, editor_melee_node, editor_explosive_node>) {
					std::visit(
						[&](const auto& typed) {
							requested_equipment r;
							r.weapon = typed;

							auto new_id = r.generate_for(typed_node.get_transform(), step);
							ensure(scene.world[new_id].alive());
							setup_node_entity_mapping(new_id);
						},

						resource->scene_flavour_id
					);
				}
				else if constexpr(std::is_same_v<node_type, editor_ammunition_node>) {
					std::visit(
						[&](const auto& typed) {
							requested_equipment r;
							r.non_standard_mag = typed;
							r.num_given_ammo_pieces = 1;

							auto new_id = r.generate_for(typed_node.get_transform(), step);
							ensure(scene.world[new_id].alive());
							setup_node_entity_mapping(new_id);
						},

						resource->scene_flavour_id
					);
				}
				else {
					auto entity_from_node = [&]<typename H>(const H& handle, auto& agg) {
						::setup_entity_from_node(
							total_order++, 
							*layer,
							typed_node, 
							resource,
							handle, 
							agg
						);
					};

					auto entity_from_node_post_construct = [&]<typename H>(const H& handle) {
						::setup_entity_from_node_post_construct(
							typed_node, 
							resource,
							handle
						);
					};

					std::visit(
						[&](const auto& typed_flavour_id) {
							const auto new_handle = cosmic::specific_create_entity(
								scene.world,
								typed_flavour_id,
								entity_from_node
							);

							entity_from_node_post_construct(new_handle);

							setup_node_entity_mapping(new_handle.get_id());
						},
						resource->scene_flavour_id
					);
				}
			};

			for (const auto node_id : reverse(layer->hierarchy.nodes)) {
				on_node(node_id, node_handler);
			}
		}
	};

	/* 
		We need to do this because ids stored in selection state
		might point to other entities after toggling visibility.
	*/

	/*
		Some node-generating funcitons need a logic_step.
		Therefore we have no choice but to fully step the cosmos with a standard solver
		so that we have a logic_step to be passed.
	*/

	auto entropy = cosmic_entropy();
	auto step_input = logic_step_input { scene.world, entropy, solve_settings() };
	auto solver = standard_solver();

	solver(
		step_input,
		solver_callbacks(
			[&](const logic_step step) { populate(step); }
		)
	);

	inspected_to_entity_selector_state();
}

inline real32 editor_light_falloff::calc_attenuation_mult_for_requested_radius() const {
	/*
		Taking component defaults is kinda stupid but we have to bear with it until we can remove legacy maps
		Ultimately vibration might scale with the values itself
		Or we'll just have hardcoded percentage like 10% and it will scale that

		Update:
		Let's actually not consider vibration in light range calculations
		vibration is meant to be minor
		also it should vibrate to the smaller side (so more attenuation)

		(void)vibration;
	*/

	const auto atten_at_edge = 
		constant +
		linear * radius +
		quadratic * radius * radius
	;

	if (atten_at_edge == 0.0f) {
		return 1.0f;
	}

	return 255.f / (atten_at_edge * float(std::max(rgba_channel(1), cutoff_alpha)));
}
