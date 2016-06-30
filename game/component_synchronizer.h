#pragma once
#include "game/entity_handle.h"
#include <type_traits>

template <bool is_const, class component_type>
class component_synchronizer_base {
public:
	friend class basic_entity_handle<is_const>;
	typedef typename maybe_const_ref<is_const, component_type>::type component_reference;
	
	component_reference component;
	basic_entity_handle<is_const> handle;
	
	component_synchronizer_base(component_reference c, basic_entity_handle<is_const> h) : component(c), handle(h) {
	}

	operator typename std::remove_reference<component_reference>::type() const {
		return *component;
	}
};

template <bool is_const, class component_type>
class component_synchronizer {};

//: public component_synchronizer_base<is_const, component_type> {
//
//};