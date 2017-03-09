#pragma once

template <typename T>
struct has_held_ids_introspector
{
	struct dummy { /* something */ };

	template <typename C, typename P>
	static auto test(P * p) -> decltype(std::declval<C>().for_each_held_id(*p), std::true_type());

	template <typename, typename>
	static std::false_type test(...);

	typedef decltype(test<T, dummy>(nullptr)) type;
	static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};

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


struct ids_to_guid_transformer {
	template <class T>
	void operator()(
		T&,
		std::enable_if_t<!std::is_base_of_v<entity_id, T>>* dummy = nullptr
	) {

	}

	template <class T>
	void operator()(
		T& id,
		std::enable_if_t<std::is_base_of_v<entity_id, T>>* dummy = nullptr
	) {
		const auto handle = cosm[id];
		id = T();

		if (handle.alive()) {
			id.guid = handle.get_guid();
		}
	}
};


template <class H, class F>
void for_each_held_id(const H handle, F callback) {
	const auto& ids = handle.get().component_ids;
	auto& cosm = handle.get_cosmos();

	for_each_in_tuple(ids, [&](const auto& id) {
		typedef typename std::decay_t<decltype(id)>::element_type component_type;
		const auto component_handle = cosm[id];

		if (component_handle.alive()) {
			held_id_introspector<component_type>::for_each_held_id(component_handle.get(), callback);
		}
	});
}

template<class T>
void transform_component_ids_to_guids(T& comp, const cosmos& cosm) {
	held_id_introspector<T>::for_each_held_id(comp, [&cosm](entity_id& id) {
		const auto handle = cosm[id];
		id = entity_id();

		if (handle.alive()) {
			id.guid = handle.get_guid();
		}
	});
}

template<class T>
void transform_component_guids_to_ids(T& comp, const cosmos& cosm) {
	held_id_introspector<T>::for_each_held_id(comp, [&cosm](entity_id& id) {
		const entity_guid guid = id.guid;

		if (guid != 0) {
			id = cosm.guid_map_for_transport.at(guid);
		}
	});
}
