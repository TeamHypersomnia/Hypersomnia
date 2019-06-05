#pragma once
#include "game/detail/visible_entities.h"
#include "view/faction_view_settings.h"

struct marker_icon {
	using I = assets::necessary_image_id;
	I id = I::INVALID;
	rgba col = white;

	template <class F>
	marker_icon(const invariants::box_marker& p, const components::marker& meta, F get_faction_color) {
		const auto type = p.type;

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
		else if (type == area_marker_type::CALLOUT || type == area_marker_type::OVERLAID_CALLOUT) {
			id = I::INVALID;
			col = white;
		}
	}

	template <class F>
	marker_icon(const invariants::point_marker& p, const components::marker& meta, F get_faction_color) {
		if (p.type == point_marker_type::TEAM_SPAWN) {
			id = I::EDITOR_ICON_SPAWN;
			col = get_faction_color(meta.associated_faction);
		}
		else if (p.type == point_marker_type::FFA_SPAWN) {
			id = I::EDITOR_ICON_SPAWN;
			col = gray;
		}
		else if (p.type == point_marker_type::EQUIPMENT_GENERATOR) {
			id = I::DETACHABLE_MAGAZINE_SLOT_ICON;
			col = white;
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
		handle.template dispatch_on_having_all<components::marker>([&](const auto& typed_handle) {
			using E = remove_cref<decltype(typed_handle)>;

			const auto& marker = typed_handle.template get<components::marker>();

			if constexpr(E::template has<invariants::point_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::point_marker>(), marker, get_faction_color);

				callback(
					typed_handle,
					m.id, 
					typed_handle.get_logic_transform(),
					m.col
				);
			}
			else if constexpr(E::template has<invariants::box_marker>()) {
				const auto m = marker_icon(typed_handle.template get<invariants::box_marker>(), marker, get_faction_color);

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

	visible.for_each<render_layer::LIGHTS>(cosm, [&](const auto& handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_LIGHT, 
			handle.get_logic_transform(),
			handle.template get<components::light>().color
		);
	});

	visible.for_each<render_layer::ILLUMINATING_WANDERING_PIXELS, render_layer::DIM_WANDERING_PIXELS>(cosm, [&](const auto& handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS, 
			handle.get_logic_transform(),
			handle.template get<components::wandering_pixels>().colorize
		);
	});

	visible.for_each<render_layer::CONTINUOUS_SOUNDS>(cosm, [&](const auto&	handle) {
		callback(
			handle,
			assets::necessary_image_id::EDITOR_ICON_SOUND, 
			handle.get_logic_transform(),
			white
		);
	});

	visible.for_each<render_layer::CONTINUOUS_PARTICLES>(cosm, [&](const auto& handle) {
		const bool has_displacement = handle.template get<invariants::continuous_particles>().wandering.is_enabled;
		const auto chosen_icon = 
			has_displacement
			? assets::necessary_image_id::EDITOR_ICON_SMOKE_EFFECT 
			: assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE
		;

		callback(
			handle,
			chosen_icon,
			handle.get_logic_transform(),
			handle.template get<invariants::continuous_particles>().effect.modifier.colorize
		);
	});

	visible.for_each<render_layer::CALLOUT_MARKERS, render_layer::OVERLAID_CALLOUT_MARKERS>(cosm, [&](const auto& handle) {
		callback(
			handle,
			assets::necessary_image_id::INVALID, 
			handle.get_logic_transform(),
			white
		);
	});
}


