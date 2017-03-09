#pragma once

template <class T>
struct is_id_type {
	static constexpr bool value = std::is_base_of_v<entity_id, T>;
};

template <class T>
void transform_component_ids_to_guids(
	T& comp, 
	const cosmos& cosm
) {
	augs::introspect_recursive<is_id_type>(
		comp, 
		[&cosm](auto& id) {
			const auto handle = cosm[id];

			if (handle.alive()) {
				id.guid = handle.get_guid();
			}
		}
	);
}

template <class T>
void transform_component_guids_to_ids(
	T& comp, 
	const cosmos& cosm
) {
	augs::introspect_recursive<is_id_type>(
		comp, 
		[&cosm](auto& id) {
			const entity_guid guid = id.guid;

			if (guid != 0) {
				id = cosm.guid_map_for_transport.at(guid);
			}
		}
	);
}

template <class H, class F>
void for_each_held_id(const H handle, F callback) {
	const auto& ids = handle.get().component_ids;
	auto& cosm = handle.get_cosmos();

	for_each_in_tuple(
		ids,
		[&](const auto& id) {
			typedef typename std::decay_t<decltype(id)>::element_type component_type;
			const auto component_handle = cosm[id];

			if (component_handle.alive()) {
				held_id_introspector<component_type>::for_each_held_id(
					component_handle.get(), 
					callback
				);
			}
		}
	);
}