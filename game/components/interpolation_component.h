#pragma once
#include "game/transcendental/component_synchronizer.h"

namespace components {
	struct interpolation : synchronizable_component {
		float base_exponent = 0.9f;
		components::transform place_of_birth;
	};
}

template<bool is_const>
class component_synchronizer<is_const, components::interpolation> : public component_synchronizer_base<is_const, components::interpolation> {
public:
	using component_synchronizer_base<is_const, components::interpolation>::component_synchronizer_base;
	using component_synchronizer_base<is_const, components::interpolation>::component;

	void write_current_to_interpolated() const {
		handle.get_cosmos().systems_audiovisual.get<interpolation_system>().write_current_to_interpolated(handle);
	}

	components::transform& get_interpolated() const {
		return handle.get_cosmos().systems_audiovisual.get<interpolation_system>().get_interpolated(handle);
	}
};