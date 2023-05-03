#pragma once
#include "augs/templates/introspection_utils/find_object_in_object.h"
#include "augs/templates/introspection_utils/types_in.h"
#include "game/cosmos/cosmos.h"
#include "view/viewables/all_viewables_defs.h"
#include "application/predefined_rulesets.h"

template <class T>
struct ignore_in_common : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

template <class object_type, class F>
void find_flavours_that_use(
	const object_type& id,
	const cosmos& cosm,
	F location_callback
) {
	static constexpr bool allow_conversion = is_typed_flavour_id_v<object_type>;
	using contains = detail_same_or_convertible<allow_conversion>;

	using finder = object_in_object<always_false, allow_conversion>;

	/* Scan all flavours. */

	(void)id;
	(void)cosm;
	(void)location_callback;

	for_each_entity_type([&](auto e) { 
		using E = decltype(e);
		using Fl = entity_flavour<E>;

		if constexpr(contains::template value<Fl, object_type>) {
			cosm.for_each_id_and_flavour<E>([&](typed_entity_flavour_id<E>, const Fl& flavour) {
				auto finder = [&](const auto& c) {
					const auto& name = flavour.template get<invariants::text_details>().name;

					auto report_in_flavour = [&](const std::string& location) {
						const auto struct_name = format_struct_name(c);
						const auto full_location = typesafe_sprintf("Flavour: %x (%x.%x)", name, struct_name, location);

						location_callback(full_location);
					};

					finder::find(id, c, report_in_flavour);
				};

				auto for_each_through = [&](const auto& where) {
					for_each_through_std_get(
						where,
						[&](const auto& c) {
							using C = remove_cref<decltype(c)>;

							if constexpr(contains::template value<C, object_type>) {
								finder(c);
							}
							else {
								(void)c;
							}
						}
					);
				};

				for_each_through(flavour.initial_components);
				for_each_through(flavour.invariant_state);
			});
		}
	});
}

struct candidate_id_locations {
	const cosmos& cosm;
	const all_viewables_defs& viewables;
	const predefined_rulesets& rulesets;
};

template <class object_type, class F>
void find_locations_that_use(
	const object_type& id,
	const candidate_id_locations l,
	F location_callback
) {
	/* For typed entity flavour ids, allow conversion to constrained entity ids. */
	static constexpr bool allow_conversion = is_typed_flavour_id_v<object_type>;

	auto& cosm = l.cosm;

	using contains = detail_same_or_convertible<allow_conversion>;

	if constexpr(contains::template value<cosmos_common_significant, object_type>) {
		/* 
			Scan the entire common state, 
			except for flavours and assets.
		*/

		const auto& common = cosm.get_common_significant();

		using finder = object_in_object<ignore_in_common, allow_conversion>;

		finder::find(id, common, [&](const std::string& location) {
			location_callback("Common: " + location);
		});
	}
	
	find_flavours_that_use(id, cosm, location_callback);

	using finder = object_in_object<always_false, allow_conversion>;

	/* Scan all assets. */

	const auto& logicals = cosm.get_logical_assets();

	auto traverse_assets = [&](const auto preffix, const auto& p) {
		(void)preffix;
		(void)p;

		using SearchedObject = typename remove_cref<decltype(p)>::value_type;

		if constexpr(contains::template value<SearchedObject, object_type>) {
			for_each_id_and_object(
				p, 
				[&](const auto&, const auto& asset) {
					auto report_in_asset = [&](const std::string& location) {
						const auto asset_name = get_displayed_name(asset, l.viewables.image_definitions);
						const auto full_location = preffix + asset_name + " (" + location + ")";

						location_callback(full_location);
					};

					finder::find(id, asset, report_in_asset);
				}
			);
		}
	};

	traverse_assets("Particle effect: ", l.viewables.particle_effects);
	traverse_assets("Animation: ", logicals.plain_animations);
	traverse_assets("Physical material: ", logicals.physical_materials);

	{
		const auto preffix = "Ruleset: ";

		l.rulesets.all.for_each_container(
			[&](const auto& rs_container) {
				using SearchedObject = remove_cref<decltype(rs_container)>;

				if constexpr(contains::template value<SearchedObject, object_type>) {
					for (const auto& it : rs_container) {
						const auto& ruleset = it.second;

						auto report_in_ruleset = [&](const std::string& location) {
							const auto full_location = preffix + ruleset.name + " (" + location + ")";

							location_callback(full_location);
						};

						finder::find(id, ruleset, report_in_ruleset);
					}
				}
			}
		);
	}
}

template <class E, class F>
void find_locations_that_use_flavour(
	const typed_entity_flavour_id<E> id,
	const candidate_id_locations l,
	F&& location_callback
) {
	int num_entities = 0;

	l.cosm.for_each_entity(
		[&]<typename H>(const H& h) {
			if constexpr(std::is_same_v<E, typename H::used_entity_type>) {
				if (h.get_flavour_id() == id) {
					++num_entities;
				}
			}
		}
	);

	if (num_entities > 0) {
		location_callback(typesafe_sprintf("%x Entities of this flavour", num_entities));
	}

	find_locations_that_use(id, l, std::forward<F>(location_callback));
}

