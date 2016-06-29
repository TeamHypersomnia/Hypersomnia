#pragma once
#include "game/entity_handle.h"
#include <type_traits>

template <class component_type, bool is_const>
class component_synchronizer {
public:
	friend class basic_entity_handle<is_const>;
	typedef typename std::conditional<is_const, const component_type&, component_type&>::type component_reference;
	
	component_reference component;
	basic_entity_handle<is_const> handle;
	
	component_synchronizer(component_reference component, basic_entity_handle<is_const> handle) : component(component), handle(handle) {
	}

	operator typename std::remove_reference<component_reference>::type() const {
		return *component;
	}
};