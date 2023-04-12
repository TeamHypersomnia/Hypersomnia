#pragma once
#include "game/cosmos/per_entity_type.h"
#include "application/setups/editor/project/editor_project.hpp"
#include "application/setups/editor/editor_rebuild_prefab_nodes.hpp"
#include "application/setups/editor/editor_rebuild_node.hpp"
#include "application/setups/editor/editor_rebuild_resource.hpp"
#include "application/setups/editor/editor_rebuild_game_mode.hpp"

#include "game/detail/inventory/requested_equipment.h"
#include "test_scenes/test_scene_flavours.h"

#include "application/arena/build_arena_from_editor_project.h"

template <class A>
void build_arena_from_editor_project(A arena_handle, const build_arena_input in) {
	const auto& project = in.project;
	const auto& official = in.official;

	const auto built_default_modes = default_rulesets_tuple { 
		official.default_test_ruleset,
		official.default_bomb_ruleset
	};

	auto find_resource = project.make_find_resource_lambda(official.resources);

	auto& scene = arena_handle.scene;
	scene = official.built_content;

	if (in.scene_entity_to_node) {
		for (auto& s : *in.scene_entity_to_node) {
			s.clear();
		}
	}

	auto rebuild_game_modes = [&]() {
		const auto& pool = project.resources.get_pool_for<editor_game_mode_resource>();

		auto& rulesets = arena_handle.rulesets;

		rulesets = {};

		auto id_counter = raw_ruleset_id(0);

		using R = editor_game_mode_resource;

		pool.for_each_id_and_object(
			[&](const auto& raw_id, const R& game_mode) {
				const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, false);

				const bool pass_playtesting_overrides = in.for_playtesting;
				const editor_playtesting_settings* overrides = nullptr;

				if (pass_playtesting_overrides) {
					overrides = std::addressof(project.playtesting);
				}

				const auto rules_id = ::setup_ruleset_from_editor_mode(
					game_mode,
					find_resource,
					built_default_modes,
					id_counter,
					rulesets,
					project.settings,
					overrides
				);

				if (typed_id == project.settings.default_server_mode) {
					rulesets.meta.server_default = rules_id;
				}

				if (typed_id == project.playtesting.mode) {
					rulesets.meta.playtest_default = rules_id;
				}

				++id_counter;
			}
		);

		arena_handle.current_mode.choose(rulesets.meta.playtest_default);
	};

	rebuild_game_modes();

	cosmos_common_significant& common = scene.world.get_common_significant(cosmos_common_significant_access());
	common.light.ambient_color = project.settings.ambient_light_color;

	auto for_each_manually_specified_official_resource_pool = [&](auto lbd) {
		lbd(official.resources.get_pool_for<editor_point_marker_resource>());
		lbd(official.resources.get_pool_for<editor_area_marker_resource>());
		lbd(official.resources.get_pool_for<editor_prefab_resource>());
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
					[&](const auto& path) { return in.project_resources_parent_folder / path; }
				);
			}
		};

#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
		official.resources.for_each([&](const auto& pool) { allocate_flavours_and_assets(pool, true); } );
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
					[&find_resource](const auto typed_id) { return find_resource(typed_id); },
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
	official.resources.for_each(setup_flavours_and_assets);
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
			auto layer = project.find_layer(layer_id);
			ensure(layer != nullptr);

			auto node_handler = [&]<typename node_type, typename id_type>(
				const node_type& typed_node,
				const editor_typed_node_id<id_type> node_id,
				const bool is_prefab_child
			) {
				typed_node.scene_entity_id.unset();

				const bool force_active = !in.editor_preview && project.settings.include_disabled_nodes;

				if (!force_active) {
					if (!typed_node.active || !layer->is_active()) {
						return;
					}
				}

				const auto resource = find_resource(typed_node.resource_id);

				if (resource == nullptr) {
					return;
				}

				auto apply_layer_modifiers_on_special_entity = [&](const auto new_entity_id) {
					if (!layer->editable.selectable_on_scene) {
						if (const auto handle = scene.world[new_entity_id]) {
							handle.dispatch([&](const auto typed_handle) {
								::make_unselectable_handle(typed_handle);
							});
						}
					}
				};

				auto setup_node_entity_mapping = [&](const auto new_entity_id) {
					if (in.scene_entity_to_node == nullptr) {
						return;
					}

					const auto mapping_index = new_entity_id.get_type_id().get_index();
					auto& mapping = (*in.scene_entity_to_node)[mapping_index];

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
							apply_layer_modifiers_on_special_entity(new_id);
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
							apply_layer_modifiers_on_special_entity(new_id);
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
							apply_layer_modifiers_on_special_entity(new_id);
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
				project.on_node(node_id, [&]<typename node_type>(const node_type& typed_node, const auto node_id) {
					node_handler(typed_node, node_id, false);

					if constexpr(std::is_same_v<node_type, editor_prefab_node>) {
						::rebuild_prefab_nodes(
							typed_node,
							find_resource,
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

	if (in.target_clean_round_state) {
		*in.target_clean_round_state = scene.world.get_solvable().significant;
	}
}
