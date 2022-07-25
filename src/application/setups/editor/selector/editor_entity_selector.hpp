#pragma once
#include "application/setups/editor/selector/editor_entity_selector.h"

template <class F>
void editor_entity_selector::for_each_highlight(
	F&& callback,
	const editor_entity_selector_settings& settings,
	const cosmos& cosm,
	const entity_selector_input in
) const {
	for_each_selected_entity(
		[&](const auto e) {
			if (cosm[e]) {
				callback(e, settings.selected_color);
			}
		},
		in.saved_selections
	);

	auto propagate_to = [&](const entity_id id, const rgba color) {
		if (cosm[id]) {
			callback(id, color);
		}
	};

	propagate_to(held, settings.held_color);
	propagate_to(hovered, settings.hovered_color);
}

template <class F>
void editor_entity_selector::for_each_selected_entity(
	F callback,
	const current_selections_type& saved_selections
) const {
	for (const auto& e : saved_selections) {
		if (!found_in(in_rectangular_selection, e)) {
			callback(e);
		}
	}

	for (const auto& e : in_rectangular_selection) {
		if (!found_in(saved_selections, e)) {
			callback(e);
		}
	}
}
