#pragma once
#include "game/transcendental/entity_handle.h"
#include "augs/templates/maybe_const.h"

template <bool is_const, class component_type>
class component_synchronizer_base {
	typedef maybe_const_ptr_t<is_const, component_type> component_pointer;
protected:
	/*
		A value of nullptr means that the entity has no such component.
	*/

	component_pointer component;
	basic_entity_handle<is_const> handle;

public:
	auto get_handle() const {
		return handle;
	}

	auto& get_raw_component() const {
		return *component;
	}

	bool operator==(std::nullptr_t) const {
		return component == nullptr;
	}

	bool operator!=(std::nullptr_t) const {
		return component != nullptr;
	}

	component_synchronizer_base(
		component_pointer c, 
		basic_entity_handle<is_const> h
	) : 
		component(c), 
		handle(h) 
	{
	}
};

template <bool is_const, class component_type>
class component_synchronizer : public component_synchronizer_base<is_const, component_type> {};