#pragma once
#include "application/setups/editor/selector/editor_entity_selector.hpp"

template <class F>
void editor_setup::for_each_highlight(F&& callback) const {
	if (is_playtesting()) {
		return;
	}

	const auto& world = scene.world;

	selector.for_each_highlight(
		std::forward<F>(callback),
		settings.entity_selector,
		world,
		make_selector_input()
	);

	auto additional_hover_highlight = [&](const auto id) {
		if (world[id]) {
			callback(id, settings.entity_selector.hovered_color);
		}
	};
	
	additional_hover_highlight(gui.filesystem.entity_to_highlight);
	additional_hover_highlight(gui.layers.entity_to_highlight);

#if 0
	// use this later to implement layer hovered detection

	if (const auto hovered = world[fae_gui.get_hovered_id()]) {
		/* Hovering from GUI, so choose the stronger, held color for it */
		callback(hovered.get_id(), settings.entity_selector.held_color);
	}

	if (const auto hovered = world[selected_fae_gui.get_hovered_id()]) {
		/* Hovering from GUI, so choose the stronger, held color for it */
		callback(hovered.get_id(), settings.entity_selector.held_color);
	}

	if (const auto match = get_matching_go_to_entity()) {
		auto color = green;
		color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

		callback(match.get_id(), settings.matched_entity_color);
	}
#endif
}
