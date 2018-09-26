#pragma once
#include "game/detail/visible_entities.h"
#include "view/faction_view_settings.h"

struct marker_icon {
	using I = assets::necessary_image_id;
	I id = I::INVALID;
	rgba col = white;

	template <class F>
	marker_icon(const invariants::box_marker& p, F get_faction_color) {
		const auto type = p.type;
		const auto meta = p.meta;

		if (type == area_marker_type::BOMBSITE_A) {
			id = I::EDITOR_ICON_BOMBSITE_A;
			col = get_faction_color(meta.associated_faction);
		}
		else if (type == area_marker_type::BOMBSITE_B) {
			id = I::EDITOR_ICON_BOMBSITE_B;
			col = get_faction_color(meta.associated_faction);
		}
		else if (type == area_marker_type::BOMBSITE_C) {
			id = I::EDITOR_ICON_BOMBSITE_C;
			col = get_faction_color(meta.associated_faction);
		}
		else if (type == area_marker_type::BUY_AREA) {
			id = I::EDITOR_ICON_BUY_AREA;
			col = get_faction_color(meta.associated_faction);
		}
		else if (type == area_marker_type::ORGANISM_AREA) {
			id = I::EDITOR_ICON_ORGANISM_AREA;
			col = green;
		}
	}

	template <class F>
	marker_icon(const invariants::point_marker& p, F get_faction_color) {
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
	const faction_view_settings& settings,
	F callback
) {
	auto get_faction_color = [&settings](const faction_type f) {
		return settings.colors[f].standard;
	};

	visible.for_each<render_layer::POINT_MARKERS, render_layer::AREA_MARKERS>(cosm, [&](const auto handle) {
		handle.template dispatch_on_having_any<invariants::point_marker, invariants::box_marker>([&](const auto typed_handle) {
			using E = remove_cref<decltype(typed_handle)>;

			if constexpr(E::template has<invariants::point_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::point_marker>(), get_faction_color);

				callback(
					typed_handle,
					m.id, 
					typed_handle.get_logic_transform(),
					m.col
				);
			}
			else if constexpr(E::template has<invariants::box_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::box_marker>(), get_faction_color);

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
			assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE, 
			handle.get_logic_transform(),
			handle.template get<invariants::continuous_particles>().effect.modifier.colorize
		);
	});
}


