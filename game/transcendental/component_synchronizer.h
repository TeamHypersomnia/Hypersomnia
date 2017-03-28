#pragma once
#include "game/transcendental/entity_handle.h"
#include "augs/templates/maybe_const.h"

template <bool is_const, class component_type>
class component_synchronizer_base {
	typedef maybe_const_ref_t<is_const, component_type> component_reference;
protected:
	component_reference component;
	basic_entity_handle<is_const> handle;

public:
	const component_type& get_data() const {
		return component;
	}

	component_synchronizer_base(component_reference c, basic_entity_handle<is_const> h) : component(c), handle(h) {
	}
};

template <bool is_const, class component_type>
class component_synchronizer : public component_synchronizer_base<is_const, component_type> {};

template <class T>
auto& get_component_ref_from_component_or_synchronizer(T& s) {
	return s;
}

template <bool is_const, class component_type>
auto& get_component_ref_from_component_or_synchronizer(component_synchronizer<is_const, component_type> s) {
	return s.get_data();
}
