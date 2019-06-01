#pragma once
#include "test_scenes/test_scene_settings.h"
#include "game/cosmos/change_common_significant.hpp"
#include "augs/templates/introspection_utils/on_dynamic_content.h"
#include "augs/misc/pool/pool.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

#include "application/setups/editor/detail/find_asset_id.h"

template <class T, class F>
void on_each_object_in_object(T& object, F&& callback) {
	augs::introspect(
		[&callback](const auto&, auto& member) {
			using Field = remove_cref<decltype(member)>;

			if constexpr(augs::has_dynamic_content_v<Field>) {
				callback(member);

				augs::on_dynamic_content(
					[&](auto& dyn, auto...) {
						callback(dyn);

						using D = remove_cref<decltype(dyn)>;

						if constexpr(!is_introspective_leaf_v<D>) {
							on_each_object_in_object(dyn, std::forward<F>(callback));
						}
					},
					member
				);
			}
			else {
				callback(member);

				if constexpr(!is_introspective_leaf_v<Field>) {
					on_each_object_in_object(member, std::forward<F>(callback));
				}
			}
		},
		object
	);
}

template <class T>
using make_flavour_ids_vector = std::vector<typed_entity_flavour_id<T>>;

struct update_official_content_settings {
	bool overwrite_recoils = true;
	bool overwrite_physical_materials = true;
	bool overwrite_common_assets = true;
	bool overwrite_non_decoration_flavours = true;
	bool overwrite_economy_vars = true;
};

inline void update_official_content(const editor_command_input cmd_in, update_official_content_settings settings) {
	auto& folder = cmd_in.folder;

	auto test = std::make_unique<intercosm>();

	test_mode_ruleset tt;
	bomb_mode_ruleset bt;

	test->make_test_scene(cmd_in.lua, test_scene_settings(), tt, &bt);

	std::vector<std::string> added_names;

	const auto& from_cosm = test->world;
	auto& to_cosm = folder.commanded->work.world;
	auto& to_viewables = folder.commanded->work.viewables;
	const auto& to_logicals = to_cosm.get_logical_assets();
	const auto& from_viewables = test->viewables;
	const auto& from_image_defs = from_viewables.image_definitions;
	const auto& to_image_defs = to_viewables.image_definitions;
	const auto& from_logicals = from_cosm.get_logical_assets();

	auto find_and_remap_by_name = [&](const auto& from_pool, const auto& to_pool, auto& field) {
		using F = decltype(field);

		if constexpr(!is_const_ref_v<F>) {
			if (const auto found = mapped_or_nullptr(from_pool, field)) {
				const auto source_name = get_displayed_name(*found, from_image_defs);

				if (const auto found_in_new = ::find_asset_id_by_name(source_name, to_pool, to_image_defs)) {
					field = *found_in_new;
				}
				else {
					field.unset();
				}
			}
			else {
				field.unset();
			}
		}
		else {
			static_assert(
				std::is_same_v<remove_cref<F>, assets::physical_material_id>, 
				"Only physical material id is allowed to be const, because it is a key in physical materials' unordered map."
				" For now we only support direct overwrite of the physical materials."
			);
		}
	};

	auto find_and_remap_by_path = [&](const auto& from_pool, const auto& to_pool, auto& field) {
		if (const auto found = mapped_or_nullptr(from_pool, field)) {
			const auto source_path = found->get_source_path();

			if (const auto found_in_new = ::find_asset_id_by_path(source_path, to_pool)) {
				field = *found_in_new;
			}
			else {
				field.unset();
			}
		}
		else {
			field.unset();
		}
	};

	auto remap_ids = [&](auto& field) {
		using F = decltype(field);
		using T = remove_cref<F>;

		if constexpr(is_pathed_asset<T>) {
			const auto& from_pool = get_viewable_pool<T>(from_viewables);
			const auto& to_pool = get_viewable_pool<T>(to_viewables);

			find_and_remap_by_path(from_pool, to_pool, field);
		}
		else if constexpr(is_unpathed_asset<T>) {
			const auto& from_pool = get_asset_pool<T>(from_viewables, from_logicals);
			const auto& to_pool = get_asset_pool<T>(to_viewables, to_logicals);

			find_and_remap_by_name(from_pool, to_pool, field);
		}
		else if constexpr(is_typed_flavour_id_v<T>) {
			using E = entity_type_of<T>;

			const auto& from_pool = from_cosm.get_flavours<E>();
			const auto& to_pool = to_cosm.get_flavours<E>();

			find_and_remap_by_name(from_pool, to_pool, field);
		}
		else if constexpr(is_constrained_flavour_id_v<T>) {
			if (field.is_set()) {
				field.dispatch(
					[&](auto typed_id) {
						using E = entity_type_of<decltype(typed_id)>;

						const auto& from_pool = from_cosm.get_flavours<E>();
						const auto& to_pool = to_cosm.get_flavours<E>();

						find_and_remap_by_name(from_pool, to_pool, typed_id.raw);
						field = typed_id;
					}
				);
			}
		}
	};

	auto remap_ids_in = [&](auto& new_object) {
		::on_each_object_in_object(new_object, remap_ids);
	};

	auto add_pathed_assets = [&](auto& to, const auto& from, const std::string& type_name) {
		for (const auto& o : from) {
			const auto source_path = o.get_source_path();
			const auto existing_asset = find_asset_id_by_path(source_path, to);

			if (existing_asset == std::nullopt) {
				{
					auto new_asset = o;
					remap_ids_in(new_asset);
					to.allocate(new_asset);
				}

				const auto added_name = ::get_displayed_name(o) + " (" + type_name + ")";
				added_names.push_back(added_name);
			}
		}
	};

	auto add_unpathed_assets = [&](auto& to, const auto& from, const std::string& type_name) {
		for (const auto& o : from) {
			const auto source_name = ::get_displayed_name(o, from_image_defs);
			const auto existing_asset = find_asset_id_by_name(source_name, to, to_image_defs);

			if (existing_asset == std::nullopt) {
				{
					auto new_asset = o;
					remap_ids_in(new_asset);
					to.allocate(new_asset);
				}

				const auto added_name = source_name + " (" + type_name + ")";
				added_names.push_back(added_name);
			}
		}
	};
	add_pathed_assets(to_viewables.image_definitions, from_viewables.image_definitions, "Image");
	add_pathed_assets(to_viewables.sounds, from_viewables.sounds, "Sound");
	add_unpathed_assets(to_viewables.particle_effects, from_viewables.particle_effects, "Particle effect");

	using flavours_ids_type = per_entity_type_container<make_flavour_ids_vector>;

	folder.commanded->work.world.change_common_significant(
		[&](auto& comm) {
			auto& mut_to_logicals = comm.logical_assets;

			if (settings.overwrite_physical_materials) {
				mut_to_logicals.physical_materials = from_logicals.physical_materials;
			}

			if (settings.overwrite_recoils) {
				mut_to_logicals.recoils = from_logicals.recoils;
			}

			if (settings.overwrite_common_assets) {
				comm.assets = test->world.get_common_assets();
			}

			add_unpathed_assets(mut_to_logicals.plain_animations, from_logicals.plain_animations, "Animation");

			flavours_ids_type flavours_to_remap;

			auto do_update = [&](const auto& from_flavours) {
				using V = typename remove_cref<decltype(from_flavours)>::mapped_type;
				using E = entity_type_of<V>;

				const auto type_name = get_type_name<V>();

				for (const auto& f : from_flavours) {
					const auto source_name = f.get_name();
					auto& to_flavours = comm.flavours.template get_for<E>();
					const auto existing_flavour = find_asset_id_by_name(source_name, std::as_const(to_flavours), to_image_defs);

					if (existing_flavour == std::nullopt) {
						{
							const auto result = to_flavours.allocate(f);
							flavours_to_remap.get_for<E>().push_back(typed_entity_flavour_id<E>(result.key));
						}

						{
							const auto added_name = f.get_name() + " (" + type_name + ")";
							added_names.push_back(added_name);
						}
					}
					else {
						if (settings.overwrite_non_decoration_flavours) {
							if constexpr(is_one_of_v<
								E,
								controlled_character,
								plain_missile,
								shootable_weapon,
								shootable_charge,
								melee_weapon,
								hand_explosive,
								finishing_trace,
								container_item,
								remnant_body,
								explosion_body,
								tool_item
							>) {
								to_flavours[*existing_flavour] = f;
								flavours_to_remap.get_for<E>().push_back(typed_entity_flavour_id<E>(*existing_flavour));
							}
						}
					}
				}
			};

			const auto& from_flavours = from_cosm.get_common_significant().flavours;
			from_flavours.for_each_container(do_update);

			flavours_to_remap.for_each([&](const auto& typed_id) {
				comm.on_flavour(typed_id, [&](auto& new_flavour) {
					remap_ids_in(new_flavour);
				});
			});

			if (settings.overwrite_physical_materials) {
				for (auto& p : mut_to_logicals.physical_materials) {
					remap_ids_in(p);
				}
			}

			if (settings.overwrite_recoils) {
				for (auto& p : mut_to_logicals.recoils) {
					remap_ids_in(p);
				}
			}

			if (settings.overwrite_common_assets) {
				remap_ids_in(comm.assets);
			}

			return changer_callback_result::DONT_REFRESH;
		}
	);

	if (settings.overwrite_economy_vars) {
		(*folder.commanded->rulesets.all.get_for<bomb_mode>().begin()).second.economy = bt.economy;
	}

	folder.commanded->work.post_load_state_correction();

	LOG("Imported content: ");

	for (const auto& n : added_names) {
		LOG(n);
	}
}
