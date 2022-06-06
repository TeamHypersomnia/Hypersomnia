#pragma once
#include "augs/log.h"
#include "test_scenes/test_scene_settings.h"
#include "game/cosmos/change_common_significant.hpp"
#include "augs/templates/introspection_utils/on_dynamic_content.h"
#include "augs/misc/pool/pool.h"
#include "augs/misc/pool/pool_allocate.h"
#include "application/setups/debugger/detail/find_asset_id.h"

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
struct flavour_remap_entry {
	typed_entity_flavour_id<T> id;
	bool is_content_mapped_to_official = false;
};

template <class T>
using make_flavour_ids_vector = std::vector<flavour_remap_entry<T>>;

struct update_official_content_settings {
	bool overwrite_recoils = true;
	bool overwrite_physical_materials = true;
	bool overwrite_common_assets = true;
	bool overwrite_non_decoration_flavours = true;
	bool overwrite_economy_vars = false;
	bool overwrite_whole_ruleset = false;
	bool overwrite_meters = true;
	bool overwrite_spells = true;
	bool overwrite_perks = true;
};

inline void update_official_content(const editor_command_input cmd_in, update_official_content_settings settings) {
	auto& folder = cmd_in.folder;

	auto test = std::make_unique<intercosm>();

	test_mode_ruleset offi_tt;
	bomb_defusal_ruleset offi_bt;

	test->make_test_scene(cmd_in.lua, test_scene_settings(), offi_tt, &offi_bt);

	std::vector<std::string> added_names;

	const auto& offi_cosm = test->world;
	auto& cust_cosm = folder.commanded->work.world;
	auto& cust_viewables = folder.commanded->work.viewables;
	const auto& cust_logicals = cust_cosm.get_logical_assets();
	const auto& offi_viewables = test->viewables;
	const auto& offi_image_defs = offi_viewables.image_definitions;
	const auto& cust_image_defs = cust_viewables.image_definitions;
	const auto& offi_logicals = offi_cosm.get_logical_assets();

	const auto old_cust_viewables = std::make_unique<all_viewables_defs>(cust_viewables);
	const auto old_cust_logicals = std::make_unique<all_logical_assets>(cust_logicals);
	const auto old_cust_flavours = std::make_unique<all_entity_flavours>(cust_cosm.get_common_significant().flavours);

	auto find_and_remap_by_name = [&](const auto& reference_pool, const auto& refere_image_defs, const auto& target_pool, auto& field) {
		using F = decltype(field);

		if constexpr(!is_const_ref_v<F>) {
			if (const auto found = mapped_or_nullptr(reference_pool, field)) {
				const auto source_name = get_displayed_name(*found, refere_image_defs);

				if (const auto found_in_new = ::find_asset_id_by_name(source_name, target_pool, cust_image_defs)) {
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

	auto find_and_remap_by_path = [&](const auto& reference_pool, const auto& target_pool, auto& field) {
		if (const auto found = mapped_or_nullptr(reference_pool, field)) {
			const auto source_path = found->get_source_path();

			if (const auto found_in_new = ::find_asset_id_by_path(source_path, target_pool)) {
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

	auto remap_ids = [&](auto& field, const bool remapping_official) {
		using F = decltype(field);
		using T = remove_cref<F>;

		const auto& reference_viewables = remapping_official ? offi_viewables : *old_cust_viewables;
		const auto& reference_logicals = remapping_official ? offi_logicals : *old_cust_logicals;
		const auto& refere_image_defs = reference_viewables.image_definitions;

		if constexpr(is_pathed_asset<T>) {
			const auto& refere_pool = get_viewable_pool<T>(reference_viewables);
			const auto& target_pool = get_viewable_pool<T>(cust_viewables);

			find_and_remap_by_path(refere_pool, target_pool, field);
		}
		else if constexpr(is_unpathed_asset<T>) {
			const auto& refere_pool = get_asset_pool<T>(reference_viewables, reference_logicals);
			const auto& target_pool = get_asset_pool<T>(cust_viewables, cust_logicals);

			find_and_remap_by_name(refere_pool, refere_image_defs, target_pool, field);
		}
		else if constexpr(is_typed_flavour_id_v<T>) {
			using E = entity_type_of<T>;

			const auto& refere_flavours = remapping_official ? offi_cosm.get_flavours<E>() : old_cust_flavours->get_for<E>();
			const auto& target_flavours = cust_cosm.get_flavours<E>();

			find_and_remap_by_name(refere_flavours, refere_image_defs, target_flavours, field);
		}
		else if constexpr(is_constrained_flavour_id_v<T>) {
			if (field.is_set()) {
				field.dispatch(
					[&](auto typed_id) {
						using E = entity_type_of<decltype(typed_id)>;

						const auto& refere_flavours = remapping_official ? offi_cosm.get_flavours<E>() : old_cust_flavours->get_for<E>();
						const auto& target_flavours = cust_cosm.get_flavours<E>();

						find_and_remap_by_name(refere_flavours, refere_image_defs, target_flavours, typed_id.raw);
						field = typed_id;
					}
				);
			}
		}
	};

	auto remap_ids_in = [&](auto& new_object, const bool is_official) {
		::on_each_object_in_object(new_object, [&](auto& field) { remap_ids(field, is_official); });
	};

	auto add_pathed_assets = [&](auto& custom, const auto& official, const std::string& type_name) {
		for (const auto& o : official) {
			const auto source_path = o.get_source_path();
			const auto existing_custom_asset = find_asset_id_by_path(source_path, custom);

			if (existing_custom_asset != std::nullopt) {
				auto adjusted_asset = o;
				remap_ids_in(adjusted_asset, true);
				custom[*existing_custom_asset] = adjusted_asset;
			}
			else {
				{
					auto new_asset = o;
					remap_ids_in(new_asset, true);
					custom.allocate(new_asset);
				}

				const auto added_name = ::get_displayed_name(o) + " (" + type_name + ")";
				added_names.push_back(added_name);
			}
		}
	};

	auto add_unpathed_assets = [&](auto& custom, const auto& official, const std::string& type_name) {
		for (const auto& o : official) {
			const auto source_name = ::get_displayed_name(o, offi_image_defs);
			const auto existing_custom_asset = find_asset_id_by_name(source_name, custom, cust_image_defs);

			if (existing_custom_asset != std::nullopt) {
				auto adjusted_asset = o;
				remap_ids_in(adjusted_asset, true);
				custom[*existing_custom_asset] = adjusted_asset;
			}
			else {
				{
					auto new_asset = o;
					remap_ids_in(new_asset, true);
					custom.allocate(new_asset);
				}

				const auto added_name = source_name + " (" + type_name + ")";
				added_names.push_back(added_name);
			}
		}
	};

	add_pathed_assets(cust_viewables.image_definitions, offi_viewables.image_definitions, "Image");
	add_pathed_assets(cust_viewables.sounds, offi_viewables.sounds, "Sound");
	add_unpathed_assets(cust_viewables.particle_effects, offi_viewables.particle_effects, "Particle effect");

	using flavours_ids_type = per_entity_type_container<make_flavour_ids_vector>;

	folder.commanded->work.world.change_common_significant(
		[&](auto& comm) {
			auto& mut_cust_logicals = comm.logical_assets;

			if (settings.overwrite_physical_materials) {
				mut_cust_logicals.physical_materials = offi_logicals.physical_materials;
			}

			if (settings.overwrite_recoils) {
				mut_cust_logicals.recoils = offi_logicals.recoils;
			}

			if (settings.overwrite_common_assets) {
				comm.assets = offi_cosm.get_common_assets();
			}

			if (settings.overwrite_spells) {
				comm.spells = offi_cosm.get_common_significant().spells;
			}

			if (settings.overwrite_perks) {
				comm.perks = offi_cosm.get_common_significant().perks;
			}

			if (settings.overwrite_meters) {
				comm.meters = offi_cosm.get_common_significant().meters;
			}

			add_unpathed_assets(mut_cust_logicals.plain_animations, offi_logicals.plain_animations, "Animation");

			flavours_ids_type flavours_to_remap;


			auto import_official_flavours = [&](const auto& offi_flavours) {
				using V = typename remove_cref<decltype(offi_flavours)>::mapped_type;
				using E = entity_type_of<V>;

				const auto type_name = get_type_name<V>();

				for (const auto& f : offi_flavours) {
					const auto source_name = f.get_name();
					auto& cust_flavours = comm.flavours.template get_for<E>();

					LOG("Processing flavour: %x", source_name);

					const auto existing_custom_flavour = find_asset_id_by_name(source_name, std::as_const(cust_flavours), cust_image_defs);

					if (existing_custom_flavour == std::nullopt) {
						LOG("It's new");
						{
							const auto result = cust_flavours.allocate(f);
							flavours_to_remap.get_for<E>().push_back({ typed_entity_flavour_id<E>(result.key), true });
						}

						{
							const auto added_name = f.get_name() + " (" + type_name + ")";
							added_names.push_back(added_name);
						}
					}
					else {
						LOG("Exists already");

						bool will_remapped_be_official = false;

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
								LOG("Non-decoration, overwriting");
								cust_flavours[*existing_custom_flavour] = f;
								will_remapped_be_official = true;
							}
						}

						flavours_to_remap.get_for<E>().push_back({ typed_entity_flavour_id<E>(*existing_custom_flavour), will_remapped_be_official });
					}
				}
			};

			const auto& offi_flavours = offi_cosm.get_common_significant().flavours;
			offi_flavours.for_each_container(import_official_flavours);

			auto remap_custom_flavours = [&](auto& cust_flavours) {
				using V = typename remove_cref<decltype(cust_flavours)>::mapped_type;
				using E = entity_type_of<V>;

				const auto& typed_offi_flavours = offi_flavours.template get_for<E>();

				for (auto& f : cust_flavours) {
					const auto source_name = f.get_name();

					{
						const auto existing_official_flavour = find_asset_id_by_name(source_name, typed_offi_flavours, offi_image_defs);

						if (existing_official_flavour) {
							continue;
						}
					}

					remap_ids_in(f, false);
				}
			};

			comm.flavours.for_each_container(remap_custom_flavours);

			flavours_to_remap.for_each([&](const auto& remapped_entry) {
				comm.on_flavour(remapped_entry.id, [&](auto& new_flavour) {
					remap_ids_in(new_flavour, remapped_entry.is_content_mapped_to_official);
				});
			});

			for (auto& p : mut_cust_logicals.physical_materials) {
				remap_ids_in(p, settings.overwrite_physical_materials);
			}

			for (auto& p : mut_cust_logicals.recoils) {
				remap_ids_in(p, settings.overwrite_recoils);
			}

			remap_ids_in(comm.assets, settings.overwrite_common_assets);

			remap_ids_in(comm.spells, settings.overwrite_spells);
			remap_ids_in(comm.perks, settings.overwrite_perks);
			remap_ids_in(comm.meters, settings.overwrite_meters);

			return changer_callback_result::DONT_REFRESH;
		}
	);

	{
		auto& cust_economy = (*folder.commanded->rulesets.all.get_for<bomb_defusal>().begin()).second.economy;

		if (settings.overwrite_economy_vars) {
			cust_economy = offi_bt.economy;
		}

		remap_ids_in(cust_economy, settings.overwrite_economy_vars);
	}


	{
		auto& cust_rulesets = (*folder.commanded->rulesets.all.get_for<bomb_defusal>().begin()).second;

		if (settings.overwrite_whole_ruleset) {
			const auto previous = cust_rulesets;

			cust_rulesets = offi_bt;

			cust_rulesets.bot_quota = previous.bot_quota;
			cust_rulesets.max_players_per_team = previous.max_players_per_team;
			cust_rulesets.freeze_secs = previous.freeze_secs;
			cust_rulesets.warmup_secs = previous.warmup_secs;
			cust_rulesets.max_rounds = previous.max_rounds;
			cust_rulesets.round_end_secs = previous.round_end_secs;
		}

		remap_ids_in(cust_rulesets, settings.overwrite_whole_ruleset);
	}

	folder.commanded->work.post_load_state_correction();

	LOG("Imported content: ");

	for (const auto& n : added_names) {
		LOG(n);
	}
}
