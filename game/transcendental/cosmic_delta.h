#pragma once
#include "augs/misc/streams.h"

class cosmos;

class cosmic_delta {
	template<class T, class = void>
	struct held_id_introspector {
		template<class C, class P>
		static void for_each_held_id(C&, P) {

		}
	};

	template <class T>
	struct held_id_introspector<T, std::enable_if_t<has_held_ids_introspector<T>::value>> {
		template<class C, class P>
		static void for_each_held_id(C& ref, P pred) {
			ref.for_each_held_id(pred);
		}
	};

	template <class H, class F>
	static void for_each_held_id(const H handle, F callback) {
		handle.relations().for_each_held_id(callback);

		const auto& ids = handle.get().component_ids;

		auto& cosm = handle.get_cosmos();

		for_each_in_tuple(ids, [&cosm, &callback](const auto& id) {
			typedef typename std::decay_t<decltype(id)>::element_type component_type;
			const auto component_handle = cosm[id];

			if(component_handle.alive())
				held_id_introspector<component_type>::for_each_held_id(component_handle.get(), callback);
		});
	}

	template<class T>
	static void transform_component_ids_to_guids(T& comp, const cosmos& cosm) {
		held_id_introspector<T>::for_each_held_id(comp, [&cosm](entity_id& id) {
			auto handle = cosm[id];
			id = entity_id();

			if (handle.alive())
				id.guid = handle.get_guid();
		});
	}

	template<class T>
	static void transform_component_guids_to_ids(T& comp, const cosmos& cosm) {
		held_id_introspector<T>::for_each_held_id(comp, [&cosm](entity_id& id) {
			const unsigned guid = id.guid;
			
			if(guid != 0)
				id = cosm.guid_map_for_transport.at(guid);
		});
	}

public:
	static bool encode(const cosmos& base, const cosmos& encoded, augs::stream& to);
	static void decode(cosmos& into, augs::stream& from, const bool resubstantiate_partially = false);
};