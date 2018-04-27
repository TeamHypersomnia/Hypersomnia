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

template <class C, class T>
auto checkbox_and_node_facade(
	C& current_selections, 
	const typename C::value_type& current_id, 
	const bool current_selected,
	const T& checkbox_id,
	const std::string& node_label
) {
	using namespace augs::imgui;

	const auto scoped_style = selection_checkbox_spacing();
	
	{
		bool altered = current_selected;

		if (checkbox(typesafe_sprintf("###%x", checkbox_id).c_str(), altered)) {
			if (current_selected && !altered) {
				erase_element(current_selections, current_id);
			}
			else if(!current_selected && altered) {
				current_selections.emplace(current_id);
			}
		}
	}

	ImGuiTreeNodeFlags flags = 0;

	if (current_selected) {
		flags = ImGuiTreeNodeFlags_Selected;
	}

	ImGui::SameLine();
	return scoped_tree_node_ex(node_label.c_str(), flags);
}

template <class C, class A, class I>
void do_select_all_checkbox(
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

			bool altered = all_selected;

			if (checkbox(typesafe_sprintf("###all-checkbox%x", checkbox_id).c_str(), altered)) {
				if (all_selected && !altered) {
					for_each_item([&](const auto& item) {
						erase_element(current_selections, item);
					});
				}
				else if (!all_selected && altered) {
					for_each_item([&](const auto& item) {
						current_selections.insert(item);
					});
				}
			}
		}
		else {
			bool altered = false;
			checkbox(typesafe_sprintf("###all-checkbox%x", checkbox_id).c_str(), altered);
		}
	}

	ImGui::SameLine();
}
