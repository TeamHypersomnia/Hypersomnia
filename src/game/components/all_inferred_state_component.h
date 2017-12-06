#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct all_inferred_state {
		static constexpr bool is_fundamental = true;
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::all_inferred_state
		bool activated = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

template<bool is_const>
class basic_all_inferred_state_synchronizer : public component_synchronizer_base<is_const, components::all_inferred_state> {
public:
	using base = component_synchronizer_base<is_const, components::all_inferred_state>;
	using base::component_synchronizer_base;
	using base::get_raw_component;
	using base::handle;

	bool is_activated() const;
};

template<>
class component_synchronizer<false, components::all_inferred_state> : public basic_all_inferred_state_synchronizer<false> {
	void regenerate_caches() const;
public:
	using basic_all_inferred_state_synchronizer<false>::basic_all_inferred_state_synchronizer;

	void set_activated(const bool flag) const;
};

template<>
class component_synchronizer<true, components::all_inferred_state> : public basic_all_inferred_state_synchronizer<true> {
public:
	using basic_all_inferred_state_synchronizer<true>::basic_all_inferred_state_synchronizer;
};