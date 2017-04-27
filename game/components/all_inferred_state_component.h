#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct all_inferred_state : synchronizable_component {
		// GEN INTROSPECTOR struct components::all_inferred_state
		bool activated = true;
		std::array<padding_byte, 3> pad;
		// END GEN INTROSPECTOR
	};
}

template<bool is_const>
class basic_all_inferred_state_synchronizer : public component_synchronizer_base<is_const, components::all_inferred_state> {
public:
	using component_synchronizer_base<is_const, components::all_inferred_state>::component_synchronizer_base;

	auto get_radius() const {
		return get_data().radius;
	}

	bool is_activated() const {
		return get_data().activated;
	}
};

template<>
class component_synchronizer<false, components::all_inferred_state> : public basic_all_inferred_state_synchronizer<false> {
	void reinference() const {
		handle.get_cosmos().complete_reinference(handle);
	}
public:
	using basic_all_inferred_state_synchronizer<false>::basic_all_inferred_state_synchronizer;

	void set_activated(const bool flag) const {
		if (flag == get_data().activated) {
			return;
		}

		get_data().activated = flag;
		reinference();
	}
};

template<>
class component_synchronizer<true, components::all_inferred_state> : public basic_all_inferred_state_synchronizer<true> {
public:
	using basic_all_inferred_state_synchronizer<true>::basic_all_inferred_state_synchronizer;
};