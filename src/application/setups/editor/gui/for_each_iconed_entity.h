#pragma once
#include "game/detail/visible_entities.h"

struct marker_icon {
	using I = assets::necessary_image_id;
	I id = I::INVALID;
	rgba col = white;

	void from_area_marker(
		const area_marker_type& type, 
		const marker_meta& meta
	) {
		if (type == area_marker_type::BOMBSITE_A) {
			id = I::EDITOR_ICON_BOMBSITE_A;
		}
		else if (type == area_marker_type::BOMBSITE_B) {
			id = I::EDITOR_ICON_BOMBSITE_B;
		}
		else if (type == area_marker_type::BOMBSITE_C) {
			id = I::EDITOR_ICON_BOMBSITE_C;
		}

		(void)meta;
	}

	marker_icon(const invariants::box_marker& p) {
		from_area_marker(p.type, p.meta);
	}

	marker_icon(const invariants::point_marker& p) {
		if (p.type == point_marker_type::TEAM_SPAWN) {
			id = I::EDITOR_ICON_SPAWN;
			col = get_faction_color(p.meta.associated_faction);
		}
		else if (p.type == point_marker_type::FFA_SPAWN) {
			id = I::EDITOR_ICON_SPAWN;
			col = gray;
		}
	}
};

template <class C, class F>
void for_each_iconed_entity(
	const C& cosm, 
	const visible_entities& visible,
	F callback
) {
	visible.for_each<render_layer::POINT_MARKERS, render_layer::AREA_MARKERS>(cosm, [&](const auto handle) {
		handle.template dispatch_on_having_any<invariants::point_marker, invariants::box_marker>([&](const auto typed_handle) {
			using E = remove_cref<decltype(typed_handle)>;

			if constexpr(E::template has<invariants::point_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::point_marker>());

				callback(
					typed_handle,
					m.id, 
					typed_handle.get_logic_transform(),
					m.col
				);
			}
			else if constexpr(E::template has<invariants::box_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::box_marker>());

				callback(
					typed_handle,
					m.id, 
					typed_handle.get_logic_transform(),
					m.col
				);
			}
			else {
				static_assert(always_false_v<E>);
			}
		});
	});

	visible.for_each<render_layer::LIGHTS>(cosm, [&](const auto handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_LIGHT, 
			handle.get_logic_transform(),
			handle.template get<components::light>().color
		);
	});

	visible.for_each<render_layer::ILLUMINATING_WANDERING_PIXELS, render_layer::DIM_WANDERING_PIXELS>(cosm, [&](const auto handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			handle.get_logic_transform(),
			handle.template get<components::wandering_pixels>().colorize
		);
	});

	visible.for_each<render_layer::CONTINUOUS_SOUNDS>(cosm, [&](const auto handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_SOUND, 
			handle.get_logic_transform(),
			white
		);
	});

	visible.for_each<render_layer::CONTINUOUS_PARTICLES>(cosm, [&](const auto handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			handle.get_logic_transform(),
			handle.template get<invariants::continuous_particles>().effect.modifier.colorize
		);
	});
}


