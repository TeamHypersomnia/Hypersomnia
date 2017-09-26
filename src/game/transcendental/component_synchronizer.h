#pragma once
#include "game/transcendental/entity_handle.h"
#include "augs/templates/maybe_const.h"

template <bool is_const, class component_type>
class component_synchronizer_base {
	using component_pointer = maybe_const_ptr_t<is_const, component_type>;

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

	bool operator==(const std::nullptr_t) const {
		return component == nullptr;
	}

	bool operator!=(const std::nullptr_t) const {
		return component != nullptr;
	}

	operator bool() const {
		return component != nullptr;
	}

	component_synchronizer_base(
		const component_pointer c, 
		const basic_entity_handle<is_const> h
	) : 
		component(c), 
		handle(h)
	{}
};

template <bool is_const, class component_type>
class component_synchronizer;