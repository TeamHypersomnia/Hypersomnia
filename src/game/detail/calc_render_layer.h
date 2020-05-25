#pragma once
#include "game/components/render_component.h"
#include "augs/templates/traits/is_nullopt.h"
#include "augs/build_settings/compiler_defines.h"
#include "game/organization/all_component_includes.h"
#include "game/cosmos/entity_type_traits.h"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"
#include "game/detail/entities_with_render_layer.h"
#include "augs/log.h"
#include "game/detail/explosive/like_explosive.h"

template <class H>
FORCE_INLINE auto calc_render_layer(const H& handle) {
	if constexpr(H::is_specific) {
		if constexpr(H::template has<invariants::render>()) {
			return handle.template get<invariants::render>().layer;
		}
		else if constexpr(H::template has<invariants::hand_fuse>()) {
			if (is_like_planted_bomb(handle)) {
				return render_layer::PLANTED_BOMBS;
			} 

			return render_layer::DROPPED_ITEMS;
		}
		else if constexpr(H::template has<invariants::item>()) {
			return render_layer::DROPPED_ITEMS;
		}
		else if constexpr(H::template has<invariants::remnant>()) {
			return render_layer::DROPPED_ITEMS;
		}
		else if constexpr(H::template has<invariants::missile>()) {
			return render_layer::GLOWING_FOREGROUND;
		}
		else if constexpr(H::template has<invariants::sentience>()) {
			return render_layer::SENTIENCES;
		}
		else if constexpr(H::template has<components::wandering_pixels>()) {
			return 
				handle.template get<components::wandering_pixels>().illuminate ? 
				render_layer::ILLUMINATING_WANDERING_PIXELS :
				render_layer::DIM_WANDERING_PIXELS
			;
		}
		else if constexpr(H::template has<invariants::point_marker>()) {
			return render_layer::POINT_MARKERS;
		}
		else if constexpr(H::template has<invariants::box_marker>()) {
			const auto& m = handle.template get<invariants::box_marker>();

			if (m.type == area_marker_type::CALLOUT) {
				return render_layer::CALLOUT_MARKERS;
			}

			if (m.type == area_marker_type::OVERLAID_CALLOUT) {
				return render_layer::OVERLAID_CALLOUT_MARKERS;
			}

			return render_layer::AREA_MARKERS;
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
				if constexpr(is_nullopt_v<decltype(typed_handle)>) {
					return render_layer::INVALID;
				}
				else {
					return calc_render_layer(typed_handle);
				}
			}
		);
	}
}
