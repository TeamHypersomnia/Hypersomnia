#pragma once
#include <string>
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "game/cosmos/entity_flavour_id.h"
#include "application/setups/debugger/property_debugger/fae/fae_tree_structs.h"

template <class E>
void ex_on_buttons(
	const fae_tree_input fae_in,
	const std::size_t total_types,
	fae_tree_filter& filter
) {
	using namespace augs::imgui;

	if (fae_in.show_filter_buttons) {
		const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
		const auto this_type_id = entity_type_id::of<E>();

		const auto id = scoped_id(this_type_id.get_index());

		if (ImGui::Button("Ex")) {
			filter.deselect_type_id = this_type_id;
		}

		if (total_types > 1) {
			ImGui::SameLine();

			if (ImGui::Button("On")) {
				filter.select_only_type_id = this_type_id;
			}
		}

		ImGui::SameLine();
	}
}

inline void ex_on_buttons(
	const fae_tree_input fae_in,
	const entity_flavour_id flavour_id,
	const std::size_t total_types,
	const std::size_t total_flavours,
	const std::string& imgui_id,
	fae_tree_filter& filter
) {
	using namespace augs::imgui;

	if (fae_in.show_filter_buttons) {
		const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));

		const auto ex_label = "Ex##" + imgui_id;
		const auto on_label = "On##" + imgui_id;

		if (ImGui::Button(ex_label.c_str())) {
			filter.deselect_flavour_id = flavour_id;
		}

		ImGui::SameLine();

		if (!(total_flavours == 1 && total_types == 1)) {
			if (ImGui::Button(on_label.c_str())) {
				filter.select_only_flavour_id = flavour_id;
			}

			ImGui::SameLine();
		}
	}
}
