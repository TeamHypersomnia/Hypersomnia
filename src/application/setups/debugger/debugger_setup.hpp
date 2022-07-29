#pragma once
#include "application/setups/debugger/debugger_setup.h"
#include "application/setups/debugger/detail/make_command_from_selections.h"
#include "view/rendering_scripts/for_each_iconed_entity.h"
#include "game/detail/visible_entities.h"
#include "augs/drawing/general_border.h"

template <class F>
void debugger_setup::try_to_open_new_folder(F&& new_folder_provider) {
	const auto new_index = signi.current_index + 1;

	signi.folders.reserve(signi.folders.size() + 1);
	signi.folders.emplace(signi.folders.begin() + new_index);

	auto& new_folder = signi.folders[new_index];

	try {
		new_folder_provider(new_folder);
		set_current(new_index);
	}
	catch (const simple_popup& p) {
		signi.folders.erase(signi.folders.begin() + new_index);
		set_popup(p);
	}
}

template <class T, class... Args>
auto debugger_setup::make_command_from_selections(Args&&... args) const {
	return ::make_command_from_selections<T>(
		make_for_each_selected_entity(),
		work().world,
		std::forward<Args>(args)...
	);
}

template <class F>
void debugger_setup::for_each_dashed_line(F&& callback) const {
	if (is_editing_mode()) {
		const auto& world = work().world;

		if (const auto handle = world[selector.get_hovered()]) {
			if (const auto tr = handle.find_logic_transform()) {
				/* Draw dashed lines around the selected entity */
				const auto ps = augs::make_rect_points(tr->pos, handle.get_logical_size(), tr->rotation);

				for (std::size_t i = 0; i < ps.size(); ++i) {
					const auto& v = ps[i];
					const auto& nv = wrap_next(ps, i);

					callback(v, nv, settings.entity_selector.hovered_dashed_line_color, 0);
				}
			}
		}

		for_each_selected_entity(
			[&](const entity_id id) {
				const auto handle = world[id];

				if (handle.dead()) {
					return;
				}

				handle.dispatch_on_having_all<components::light>([&](const auto typed_handle) {
					const auto center = typed_handle.get_logic_transform().pos;

					const auto& light = typed_handle.template get<components::light>();

					const auto light_color = light.color;

					auto draw_reach_indicator = [&](const auto reach, const auto col) {
						callback(center, center + reach / 2, col);

						augs::general_border_from_to(
							ltrb(xywh::center_and_size(center, reach)),
							0,
							[&](const vec2 from, const vec2 to) {
								callback(from, to, col);
							}
						);
					};

					draw_reach_indicator(light.calc_reach_trimmed(), light_color);
					draw_reach_indicator(light.calc_wall_reach_trimmed(), rgba(light_color).mult_alpha(0.7f));
				});

				if (is_mover_active()) {
					handle.dispatch_on_having_all<components::overridden_geo>([&](const auto& typed_handle) {
						const auto s = typed_handle.get_logical_size();
						const auto tr = typed_handle.get_logic_transform();

						const auto& history = folder().history;
						const auto& last = history.last_command();

						if (const auto* const cmd = std::get_if<resize_entities_command>(std::addressof(last))) {
							const auto active = cmd->get_active_edges();
							const auto edges = ltrb::center_and_size(tr.pos, s).make_edges();

							auto draw_edge = [&](auto e) {
								callback(e[0].rotate(tr), e[1].rotate(tr), red, global_time_seconds * 8, true);
							};

							if (active.top) {
								draw_edge(edges[0]);
							}
							if (active.right) {
								draw_edge(edges[1]);
							}
							if (active.bottom) {
								draw_edge(edges[2]);
							}
							if (active.left) {
								draw_edge(edges[3]);
							}
						}
					});
				}
			}
		);
	}
}

template <class F>
void debugger_setup::for_each_icon(
	const visible_entities& entities, 
	const faction_view_settings& settings,
	F&& callback
) const {
	if (is_editing_mode()) {
		const auto& world = work().world;

		::for_each_iconed_entity(
			world, 
			entities,
			settings,

			[&](auto&&... args) {
				callback(std::forward<decltype(args)>(args)...);
			}
		);
	}
}
