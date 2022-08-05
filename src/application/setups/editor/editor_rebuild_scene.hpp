#pragma once
#include "application/setups/editor/resources/can_be_instantiated.h"

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

void editor_setup::rebuild_scene() {
	scene.clear();

	for (auto& s : scene_entity_to_node) {
		s.clear();
	}

	const auto mutable_access = cosmos_common_significant_access();
	auto& common = scene.world.get_common_significant(mutable_access);
	common.light.ambient_color = rgba(180, 180, 180, 255);
	(void)common;

	/* Create resources */

	auto resource_pool_handler = [&]<typename P>(const P& pool, const bool is_official) {
		using resource_type = typename P::value_type;

		auto& viewables = scene.viewables;

		for (const auto& resource : pool) {
			if constexpr(std::is_same_v<editor_sprite_resource, resource_type>) {
				auto create_as = [&]<typename entity_type>(entity_type) {
					auto& flavour_pool = scene.world.get_flavours<entity_type>(mutable_access);
					auto& definitions = viewables.image_definitions;
					
					const auto [new_image_id, new_definition] = definitions.allocate();

					{
						/* 
							Note that this is later resolved with get_unofficial_content_dir.
							get_unofficial_content_dir could really be depreciated,
							it existed because an intercosm always had relative paths since it was saved.
							Now we won't need it because an intercosm will always exist in memory only.
							Even if we cache things in some .cache folder per-project,
							I think we'd be better off just caching binary representation of the project data instead of intercosms.
						*/

						new_definition.source_image.path = resource.external_file.path_in_project;
						new_definition.source_image.is_official = is_official;

						auto& meta = new_definition.meta;
						(void)meta;
					}

					const auto [new_raw_flavour_id, new_flavour] = flavour_pool.allocate();
					const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

					const auto& editable = resource.editable;

					{
						auto& render = new_flavour.template get<invariants::render>();

						render.layer = [&]() {
							switch (editable.domain) {
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
						auto& sprite = new_flavour.template get<invariants::sprite>();
						sprite.set(new_image_id, editable.size, editable.color);
						sprite.tile_excess_size = !editable.stretch_when_resized;
					}

					/* Cache for quick and easy mapping */
					resource.scene_flavour_id = new_flavour_id;
				};

				// for now the only supported one
				create_as(static_decoration());
			}
			else if constexpr(std::is_same_v<editor_sound_resource, resource_type>) {
				auto create_as = [&]<typename entity_type>(entity_type) {
					auto& flavour_pool = scene.world.get_flavours<entity_type>(mutable_access);
					auto& definitions = viewables.sounds;
					
					const auto [new_sound_id, new_definition] = definitions.allocate();

					{
						new_definition.source_sound.path = resource.external_file.path_in_project;
						new_definition.source_sound.is_official = is_official;

						auto& meta = new_definition.meta;
						(void)meta;
					}

					const auto [new_raw_flavour_id, new_flavour] = flavour_pool.allocate();
					const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

					const auto& editable = resource.editable;

					{
						auto& sound = new_flavour.template get<invariants::continuous_sound>();
						sound.effect.id = new_sound_id;
						sound.effect.modifier = static_cast<sound_effect_modifier>(editable);
					}

					/* Cache for quick and easy mapping */
					resource.scene_flavour_id = new_flavour_id;
				};

				create_as(sound_decoration());
			}
			else if constexpr(std::is_same_v<editor_light_resource, resource_type>) {

			}
			else {
				//static_assert(always_false_v<P>, "Non-exhaustive if constexpr");
			}
		}
	};

	project.resources .for_each([&](const auto& pool) { resource_pool_handler(pool, false); } );
	official_resources.for_each([&](const auto& pool) { resource_pool_handler(pool, true); } );

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

			if constexpr(
				std::is_same_v<editor_sprite_node, node_type> ||
				std::is_same_v<editor_sound_node, node_type>
			) {
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
			}
			else {
				//static_assert(always_false_v<P>, "Non-exhaustive if constexpr");
			}
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
