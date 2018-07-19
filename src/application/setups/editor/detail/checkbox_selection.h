#pragma once
#include <tuple>
#include "augs/templates/container_templates.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

#include "application/setups/editor/detail/maybe_different_colors.h"

inline auto selection_checkbox_spacing() {
	using namespace augs::imgui;

	return std::make_tuple(
		scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(3, 1)),
		scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1))
	);
}

template <class C, class V, class T>
ImGuiTreeNodeFlags do_selection_checkbox(
	C& current_selections, 
	const V& current_id, 
	const bool current_selected,
	const T& checkbox_id
) {
	using namespace augs::imgui;

	const auto scoped_style = selection_checkbox_spacing();
	
	{
		bool now_current_selected = current_selected;

		if (checkbox(typesafe_sprintf("###%x", checkbox_id).c_str(), now_current_selected)) {
			if (current_selected && !now_current_selected) {
				erase_element(current_selections, current_id);
			}
			else if (!current_selected && now_current_selected) {
				emplace_element(current_selections, current_id);
			}
		}
	}

	ImGui::SameLine();

	if (current_selected) {
		return ImGuiTreeNodeFlags_Selected;
	}

	return 0;
}

template <class C, class A, class I>
ImGuiTreeNodeFlags do_tick_all_checkbox(
	const property_editor_settings& settings,
	C& current_selections, 
	A for_each_item,
	const I& checkbox_id
) {
	using namespace augs::imgui;

	const auto scoped_style = selection_checkbox_spacing();

	bool all_selected = true;
	bool any_selected = false;
	bool any_exists = false;

	for_each_item([&](const auto& item) {
		any_exists = true;

		if (found_in(current_selections, item)) {
			any_selected = true;
		}
		else {
			all_selected = false;
		}
	});

	{
		if (any_exists) {
			auto cols = ::maybe_different_value_cols(
				settings,
				any_selected && !all_selected
			);

			bool now_all_altered = all_selected;

			if (checkbox(typesafe_sprintf("###all-checkbox%x", checkbox_id).c_str(), now_all_altered)) {
				if (all_selected && !now_all_altered) {
					for_each_item([&](const auto& item) {
						erase_element(current_selections, item);
					});
				}
				else if (!all_selected && now_all_altered) {
					for_each_item([&](const auto& item) {
						emplace_element(current_selections, item);
					});
				}
			}
		}
		else {
			bool dummy = false;
			checkbox(typesafe_sprintf("###all-checkbox%x", checkbox_id).c_str(), dummy);
		}
	}

	ImGui::SameLine();

	if (all_selected && any_exists) {
		return ImGuiTreeNodeFlags_Selected;
	}

	return 0;
}
