#pragma once
#include "game/entity_handle.h"
#include <type_traits>

template <bool is_const, class component_type>
class component_synchronizer_base {
	template<bool, class, class>
	friend class augs::basic_handle;

	typedef maybe_const_ref_t<is_const, component_type> component_reference;
protected:
	component_reference component;
	basic_entity_handle<is_const> handle;

	void complete_resubstantialization() const {
		handle.get_cosmos().complete_resubstantialization(handle);
	}

public:
	const component_type& get_data() const {
		return component;
	}

	bool is_activated() const {
		return component.activated;
	}

	component_synchronizer_base(component_reference c, basic_entity_handle<is_const> h) : component(c), handle(h) {
	}

	operator typename std::remove_reference<component_reference>::type() const {
		return component;
	}
};

template <bool is_const, class component_type>
class component_synchronizer {};