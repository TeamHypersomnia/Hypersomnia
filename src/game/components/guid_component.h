#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct guid {
		static constexpr bool is_always_present = true;
		static constexpr bool is_synchronized = true;

		// GEN INTROSPECTOR struct components::guid
		entity_guid value = 0;
		// END GEN INTROSPECTOR
	};
}

template <class E>
class component_synchronizer<E, components::guid> : public synchronizer_base<E, components::guid> {
protected:
	using base = synchronizer_base<E, components::guid>;
public:
	using base::get_raw_component;
	using base::synchronizer_base;

	auto get_value() const {
		return get_raw_component().value;
	}
};