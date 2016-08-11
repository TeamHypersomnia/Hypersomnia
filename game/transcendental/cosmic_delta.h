#pragma once

namespace RakNet {
	class BitStream;
}

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
	static void for_each_held_id(H handle, F callback) {
		handle.relations().for_each_held_id(callback);

		const auto& ids = handle.get().component_ids;

		auto& cosm = handle.get_cosmos();

		for_each_in_tuple(ids, [&cosm, &callback](const auto& id) {
			typedef typename std::decay_t<decltype(id)>::element_type component_type;
			auto component_handle = cosm[id];

			if(component_handle.alive())
				held_id_introspector<component_type>::for_each_held_id(component_handle.get(), callback);
		});
	}

public:
	static void encode(const cosmos& base, const cosmos& encoded, RakNet::BitStream& to);
	static void decode(cosmos& into, RakNet::BitStream& from, bool resubstantiate_partially = false);
};