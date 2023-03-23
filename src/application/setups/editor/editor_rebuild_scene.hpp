#pragma once
#include "application/setups/editor/resources/resource_traits.h"
#include "game/enums/filters.h"
#include "game/detail/inventory/requested_equipment.h"
#include "game/detail/inventory/generate_equipment.h"
#include "game/cosmos/solvers/standard_solver.h"
#include "view/audiovisual_state/systems/legacy_light_mults.h"
#include "game/cosmos/change_common_significant.hpp"

#include "application/setups/editor/editor_rebuild_prefab_nodes.hpp"
#include "application/setups/editor/editor_rebuild_node.hpp"
#include "application/setups/editor/editor_rebuild_resource.hpp"
#include "application/setups/editor/editor_rebuild_game_mode.hpp"

void editor_setup::rebuild_scene() {
	scene = initial_scene;

	for (auto& s : scene_entity_to_node) {
		s.clear();
	}

	const auto mutable_access = cosmos_common_significant_access();
	auto& common = scene.world.get_common_significant(mutable_access);
	common.light.ambient_color = project.settings.ambient_light_color;

	auto for_each_manually_specified_official_resource_pool = [&](auto lbd) {
		lbd(official_resources.get_pool_for<editor_point_marker_resource>());
		lbd(official_resources.get_pool_for<editor_area_marker_resource>());
		lbd(official_resources.get_pool_for<editor_prefab_resource>());
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
					[this](const auto typed_id) { return this->find_resource(typed_id); },
					resource,
					scene_object
				);
			};

			if constexpr(std::is_same_v<R, editor_game_mode_resource>) {

			}
			else if constexpr(std::is_same_v<R, editor_material_resource>) {
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

	/* 
		We do this instead of calling post_load_state_correction 
		which would reinfer all entities unnecessarily.
	*/

	scene.world.change_common_significant([&](cosmos_common_significant& common) {
		scene.viewables.update_relevant(common.logical_assets);

		return changer_callback_result::DONT_REFRESH;
	});

	/* Create nodes */

	auto total_order = sorting_order_type(0);

	auto populate = [&](const logic_step step) {
		for (const auto layer_id : reverse(project.layers.order)) {
			auto layer = find_layer(layer_id);
			ensure(layer != nullptr);

			auto node_handler = [&]<typename node_type, typename id_type>(
				const node_type& typed_node,
				const editor_typed_node_id<id_type> node_id,
				const bool is_prefab_child
			) {
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
							*resource,
							handle, 
							agg
						);

						if (is_prefab_child) {
							::make_unselectable(agg);
						}
					};

					auto entity_from_node_post_construct = [&]<typename H>(const H& handle) {
						::setup_entity_from_node_post_construct(
							typed_node, 
							*resource,
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
				on_node(node_id, [&]<typename node_type>(const node_type& typed_node, const auto node_id) {
					node_handler(typed_node, node_id, false);

					if constexpr(std::is_same_v<node_type, editor_prefab_node>) {
						rebuild_prefab_nodes(
							node_id,
							[&](const auto& child_node) {
								node_handler(child_node, node_id, true);
							}
						);
					}
				});
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

	/*
		Strength is just another name for cutoff alpha.
	*/

	const auto cutoff_alpha = strength;
	return 255.f / (atten_at_edge * float(std::max(rgba_channel(1), cutoff_alpha)));
}
