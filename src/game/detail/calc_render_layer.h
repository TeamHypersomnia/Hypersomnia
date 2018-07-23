#pragma once
#include "game/components/render_component.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_component_includes.h"
#include "game/transcendental/entity_type_traits.h"

using entities_with_render_layer = entity_types_with_any_of<
	invariants::render,
	invariants::light,
	invariants::continuous_particles,
	invariants::continuous_sound
>;

template <class H>
FORCE_INLINE auto calc_render_layer(const H& handle) {
	if constexpr(H::is_specific) {
		if constexpr(H::template has<invariants::render>()) {
			return handle.template get<invariants::render>().layer;
		}
		else if constexpr(H::template has<invariants::light>()) {
			return render_layer::LIGHTS;
		}
		else if constexpr(H::template has<invariants::continuous_particles>()) {
			return render_layer::CONTINUOUS_PARTICLES;
		}
		else if constexpr(H::template has<invariants::continuous_sound>()) {
			return render_layer::CONTINUOUS_SOUNDS;
		}
		else {
			return render_layer::INVALID;
		}
	}
	else {
		return handle.template conditional_dispatch_ret<entities_with_render_layer>(
			[&](const auto& typed_handle) -> render_layer {
				using E = remove_cref<decltype(typed_handle)>;

				if constexpr(std::is_same_v<E, std::nullopt_t>) {
					return render_layer::INVALID;
				}
				else {
					return calc_render_layer(typed_handle);
				}
			}
		);
	}
}
