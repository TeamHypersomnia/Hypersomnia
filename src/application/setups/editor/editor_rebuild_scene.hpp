#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"

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

	if constexpr(std::is_same_v<N, editor_sprite_node>) {
		auto& sprite = agg.template get<components::sprite>();
		sprite.colorize = editable.colorize;

		const auto alpha_mult = layer.editable.alpha_mult;

		if (alpha_mult != 1.0f) {
			sprite.colorize.mult_alpha(alpha_mult);
		}
	}

	if constexpr(std::is_same_v<N, editor_light_node>) {
		auto& light = agg.template get<components::light>();
		light.color *= editable.colorize;
	}

	if (auto geo = agg.template find<components::overridden_geo>()) {
		if constexpr(has_size_v<Editable>) {
			if (bool(editable.size)) {
				geo->size.emplace(editable.size.value());
			}
		}

		if constexpr(has_flip_v<Editable>) {
			geo->flip.horizontally = editable.flip_horizontally;
			geo->flip.vertically = editable.flip_vertically;
		}
	}

	handle.set_logic_transform(node.get_transform());
	(void)resource;
}

template <class R>
void allocate_flavours_and_assets_for_resource(
	const R& resource,
	const bool is_official,
	all_viewables_defs& viewables,
	cosmos_common_significant& common
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
	else if constexpr(std::is_same_v<editor_sprite_resource, R>) {
		const auto domain = editable.domain;

		auto create_as = [&]<typename entity_type>(entity_type) {
			auto& flavour_pool = common.flavours.get_for<entity_type>();
			auto& definitions = viewables.image_definitions;
			
			const auto [new_image_id, new_definition] = definitions.allocate();

			{
				/* 
					Note that this is later resolved with get_unofficial_content_dir.
					get_unofficial_content_dir could really be depreciated,
						it existed because an intercosm always had relative paths since it was saved.
						Now we won't need it because an intercosm will always exist in memory only.
						So we could store absolute paths in the intercosmos.

					Even if we cache things in some .cache folder per-project,
					I think we'd be better off just caching binary representation of the project data instead of intercosms.
				*/

				new_definition.source_image.path = resource.external_file.path_in_project;
				new_definition.source_image.is_official = is_official;

				auto& meta = new_definition.meta;
				(void)meta;
			}

			const auto new_raw_flavour_id = flavour_pool.allocate().key;
			const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

			resource.scene_flavour_id = new_flavour_id;
			resource.scene_asset_id = new_image_id;
		};

		if (domain == editor_sprite_domain::PHYSICAL) {
			create_as(plain_sprited_body());
		}
		else {
			create_as(static_decoration());
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
	else if constexpr(std::is_same_v<editor_sprite_resource, R>) {
		const auto domain = editable.domain;

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
		}

		{
			auto& sprite = scene.template get<invariants::sprite>();
			sprite.set(resource.scene_asset_id, editable.size, editable.color);
			sprite.tile_excess_size = !editable.stretch_when_resized;
		}

		if (auto rigid_body = scene.template find<invariants::rigid_body>()) {
			rigid_body->damping.linear = editable.linear_damping;
			rigid_body->damping.angular = editable.angular_damping;

			if (editable.is_static) {
				rigid_body->body_type = rigid_body_type::ALWAYS_STATIC;
			}
		}

		if (auto fixtures = scene.template find<invariants::fixtures>()) {
			fixtures->density = editable.density;
			fixtures->friction = editable.friction;
			fixtures->restitution = editable.restitution;

			if (editable.is_see_through) {
				fixtures->filter = filters[predefined_filter_type::GLASS_OBSTACLE];
			}
			else {
				fixtures->filter = filters[predefined_filter_type::WALL];
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
					common
				);
			}
		};

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
		official_resources.for_each([&](const auto& pool) { allocate_flavours_and_assets(pool, true); } );
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
#endif
	project.resources .for_each(setup_flavours_and_assets);

	/* Create nodes */

	auto total_order = sorting_order_type(0);

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

			auto entity_from_node = [&]<typename H>(const H& handle, auto& agg) {
				::setup_entity_from_node(
					total_order, 
					*layer,
					typed_node, 
					resource,
					handle, 
					agg
				);
			};

			std::visit(
				[&](const auto& typed_flavour_id) {
					const auto new_id = cosmic::specific_create_entity(
						scene.world,
						typed_flavour_id,
						entity_from_node
					).get_id();

					const auto mapping_index = new_id.get_type_id().get_index();
					auto& mapping = scene_entity_to_node[mapping_index];

					ensure_eq(mapping.size(), std::size_t(new_id.raw.indirection_index));
					ensure_eq(decltype(new_id.raw.version)(1), new_id.raw.version);

					mapping.emplace_back(node_id.operator editor_node_id());
					typed_node.scene_entity_id = new_id;
				},
				resource->scene_flavour_id
			);
		};

		for (const auto node_id : reverse(layer->hierarchy.nodes)) {
			on_node(node_id, node_handler);
		}
	}

	/* 
		We need to do this because ids stored in selection state
		might point to other entities after toggling visibility.
	*/

	inspected_to_entity_selector_state();
}
