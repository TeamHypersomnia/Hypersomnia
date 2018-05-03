#pragma once
#include "augs/templates/introspection_utils/find_object_in_object.h"
#include "augs/templates/introspection_utils/types_in.h"
#include "application/intercosm.h"

template <class T>
struct ignore_in_common : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

template <class object_type, class F>
void find_locations_that_use(
	const object_type id,
	const intercosm& inter,
	F location_callback
) {
	const auto& cosm = inter.world;

	static constexpr bool is_flavour = is_typed_flavour_id_v<object_type>;

	/* For typed entity flavour ids, allow conversion to constrained entity ids. */
	static constexpr bool allow_conversion = is_flavour;

	using contains = detail_same_or_convertible<allow_conversion>;

	/* If it is a flavour, include information about entities. */
	if constexpr(is_flavour) {
		const auto num_entities = cosm.get_solvable_inferred().name.get_entities_by_flavour_id(id).size();

		if (num_entities > 0) {
			location_callback(typesafe_sprintf("%x Entities of this flavour", num_entities));
		}
	}

	if constexpr(contains::template value<cosmos_common_significant, object_type>) {
		/* 
			Scan the entire common state, 
			except for flavours and assets.
		*/

		const auto& common = cosm.get_common_significant();

		find_object_in_object<ignore_in_common, allow_conversion>(id, common, [&](const auto& location) {
			location_callback("Common: " + location);
		});
	}

	/* Scan all flavours. */

	for_each_entity_type([&](auto e) { 
		using E = decltype(e);
		using Fl = entity_flavour<E>;

		if constexpr(contains::template value<Fl, object_type>) {
			cosm.for_each_id_and_flavour<E>([&](const auto, const auto& flavour) {
				const auto& name = flavour.template get<invariants::name>().name;

				auto for_each_through = [&](const auto& where) {
					for_each_through_std_get(
						where,
						[&](const auto& c) {
							using C = remove_cref<decltype(c)>;

							if constexpr(contains::template value<C, object_type>) {
								find_object_in_object<always_false, allow_conversion>(id, c, [&](const auto& location) {
									location_callback("Flavour: " + name + " (" + format_struct_name(c) + "." + location + ")");
								});
							}
						}
					);
				};

				for_each_through(flavour.initial_components);
				for_each_through(flavour.invariants);
			});
		}
	});

	/* Scan all assets. */

	const auto& viewables = inter.viewables;
	const auto& logicals = cosm.get_logical_assets();

	auto traverse_assets = [&](const auto preffix, const auto& p) {
		(void)preffix;

		if constexpr(contains::template value<typename remove_cref<decltype(p)>::value_type, object_type>) {
			for_each_id_and_object(
				p, 
				[&](const auto&, const auto& asset) {
					find_object_in_object<always_false, allow_conversion>(id, asset, [&](const auto& location) {
						if constexpr(std::is_same_v<animation, remove_cref<decltype(asset)>>) {
							// TODO: animation view?
						}
						else {
							location_callback(preffix + asset.name + " (" + location + ")");
						}
					});
				}
			);
		}
	};

	traverse_assets("Particle effect: ", viewables.particle_effects);
	traverse_assets("Animation: ", logicals.animations);
	traverse_assets("Physical material: ", logicals.physical_materials);
}
