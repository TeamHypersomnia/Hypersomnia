#pragma once
#include "application/setups/editor/gui/editor_entity_selector.h"

template <class F>
void editor_entity_selector::for_each_highlight(
	F&& callback,
	const editor_entity_selector_settings& settings,
	const cosmos& cosm,
	const grouped_selector_op_input in
) const {
	for_each_selected_entity(
		[&](const auto e) {
			callback(e, settings.selected_color);
		},
		in.signi_selections
	);

	auto propagate_to_group_of = [&](const entity_id id, const rgba color) {
		if (cosm[id]) {
			callback(id, color);

			if (!in.ignore_groups) {
				in.groups.for_each_sibling(id, [&](const auto sibling) {
					callback(sibling, color);
				});
			}
		}
	};

	propagate_to_group_of(held, settings.held_color);
	propagate_to_group_of(hovered, settings.hovered_color);
}

template <class F>
void editor_entity_selector::for_each_selected_entity(
	F callback,
	const current_selections_type& signi_selections
) const {
	for (const auto e : signi_selections) {
		if (!found_in(in_rectangular_selection, e)) {
			callback(e);
		}
	}

	for (const auto e : in_rectangular_selection) {
		if (!found_in(signi_selections, e)) {
			callback(e);
		}
	}
}
