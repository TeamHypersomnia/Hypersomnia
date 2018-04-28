#pragma once
#include "augs/templates/introspection_utils/find_object_in_object.h"
#include "augs/templates/introspection_utils/types_in.h"
#include "application/intercosm.h"

template <class T>
struct ignore_while_looking_for_id : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

template <class asset_id_type, class F>
void find_locations_that_use(
	const asset_id_type id,
	const intercosm& inter,
	F location_callback
) {
	auto traverse = [&](const std::string& preffix, const auto& object) {
		find_object_in_object<ignore_while_looking_for_id>(id, object, [&](const auto& location) {
			location_callback(preffix + location);
		});
	};

	const auto& common = inter.world.get_common_significant();

	traverse("Common: ", common);

	for_each_entity_type([&](auto e){ 
		using E = decltype(e);
		using Fl = entity_flavour<E>;

		if constexpr(can_type_contain_another_v<Fl, asset_id_type>) {
			common.get_flavours<E>().for_each([&](const auto, const auto& flavour) {
				const auto& name = flavour.template get<invariants::name>().name;

				auto for_each_through = [&](const auto& where) {
					for_each_through_std_get(
						where,
						[&](const auto& c) {
							using C = remove_cref<decltype(c)>;

							if constexpr(can_type_contain_another_v<C, asset_id_type>) {
								find_object_in_object(id, c, [&](const auto& location) {
									/* location_callback(format_struct_name(c) + "of " + name + ": " + location); */
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

	//traverse("Particle effects: ", inter.viewables.particle_effects);
}
