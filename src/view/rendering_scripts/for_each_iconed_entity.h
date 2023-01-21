#pragma once
#include "game/detail/visible_entities.h"
#include "view/faction_view_settings.h"
#include "game/detail/visible_entities.hpp"

struct marker_icon {
	using I = assets::necessary_image_id;
	I id = I::INVALID;
	rgba col = white;

	marker_icon() = default;

	template <class M, class Meta, class F>
	static auto from_area(const M& p, const Meta& meta, F get_faction_color) {
		auto result = marker_icon();
		const auto type = p.type;

		if (type == area_marker_type::BOMBSITE) {
			result.id = I::EDITOR_ICON_BOMBSITE_A;
			result.col = get_faction_color(meta.faction);
		}
		else if (type == area_marker_type::BOMBSITE_DUMMY_B) {
			result.id = I::EDITOR_ICON_BOMBSITE_B;
			result.col = get_faction_color(meta.faction);
		}
		else if (type == area_marker_type::BOMBSITE_DUMMY_C) {
			result.id = I::EDITOR_ICON_BOMBSITE_C;
			result.col = get_faction_color(meta.faction);
		}
		else if (type == area_marker_type::BUY_AREA) {
			result.id = I::EDITOR_ICON_BUY_AREA;
			result.col = get_faction_color(meta.faction);
		}
		else if (type == area_marker_type::ORGANISM_AREA) {
			result.id = I::EDITOR_ICON_ORGANISM_AREA;
			result.col = green;
		}
		else if (type == area_marker_type::CALLOUT) {
			result.id = I::GUI_CURSOR_TEXT_INPUT;
			result.col = white;
		}

		return result;
	}

	template <class M, class Meta, class F>
	static auto from_point(const M& p, const Meta& meta, F get_faction_color) {
		auto result = marker_icon();

		if (p.type == point_marker_type::TEAM_SPAWN) {
			result.id = I::EDITOR_ICON_SPAWN;
			result.col = get_faction_color(meta.faction);
		}
		else if (p.type == point_marker_type::FFA_SPAWN) {
			result.id = I::EDITOR_ICON_SPAWN;
			result.col = white;
		}

		return result;
	}

	template <class F>
	marker_icon(const invariants::box_marker& p, const components::marker& meta, F get_faction_color) {
		*this = from_area(p, meta, get_faction_color);
	}

	template <class F>
	marker_icon(const invariants::point_marker& p, const components::marker& meta, F get_faction_color) {
		*this = from_point(p, meta, get_faction_color);
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

	visible.for_each<render_layer::CALLOUT_MARKERS>(cosm, [&](const auto& handle) {
		callback(
			handle,
			assets::necessary_image_id::INVALID, 
			handle.get_logic_transform(),
			white
		);
	});
}


