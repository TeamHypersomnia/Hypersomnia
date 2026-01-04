#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/maybe_official_path.h"
#include "application/setups/editor/gui/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/gui/property_editor/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/log.h"
#include "application/setups/editor/gui/widgets/inspectable_with_icon.h"
#include "application/setups/editor/editor_get_icon_for.h"

template <class R>
class node_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;
	using id_type = editor_typed_node_id<R>;

	struct sorted_entry {
		std::string name;
		id_type target_id;
		
		bool operator<(const sorted_entry& b) const {
			return name < b.name;
		}
	};

	std::vector<sorted_entry> sorted_nodes;
	ImGuiTextFilter filter;

public:
	template <class F, class L>
	bool perform(
		const std::string& label, 
		const std::string& current_source,
		const id_type current_source_id,
		const editor_setup& setup,
		const editor_icon_info_in icon_in,
		const bool allow_none,
		F on_choice,
		L should_add,
		const std::string none_label = "(None)",
		const bool show_icon = true
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(Invalid)") : current_source;

		if (show_icon) {
			const auto before_pos = ImGui::GetCursorPos();
			auto icon_result = setup.get_icon_for(current_source_id.operator editor_node_id(), icon_in);

			const auto icon = icon_result.icon;
			const auto icon_color = setup.get_icon_color_for(current_source_id.operator editor_node_id());
			const auto atlas_type = icon_result.atlas;

			const float indent_level = 0.f;
			const auto max_icon_size = ImGui::GetTextLineHeight();
			const float content_x_offset = max_icon_size * indent_level;
			const auto icon_size = vec2::square(max_icon_size);
			const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);

			const auto diff = (icon_size - scaled_icon_size) / 2;

			const auto icon_padding = vec2(icon_size) / 2.5f;

			auto icon_cur_pos = ImVec2(vec2(before_pos) + vec2(content_x_offset, 0) + diff);
			auto target_cur_pos = ImVec2(vec2(before_pos) + vec2(content_x_offset, 0) + vec2(max_icon_size + icon_padding.x, 0));

			// ugly but I don't have time to do this nicely
			ImGui::SetCursorPos(icon_cur_pos);
			game_image(icon, scaled_icon_size, icon_color, vec2::zero, atlas_type);
			ImGui::SetCursorPos(target_cur_pos);
		}

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				sorted_nodes.clear();

				auto adder_lbd = [&](const auto& id, const auto& node) {
					if (should_add(node)) {
						sorted_nodes.push_back({ node.get_display_name(), id });
					}
				};

				setup.template for_each_node<R>(adder_lbd);

				sort_range(sorted_nodes);

				if (allow_none) {
					sorted_nodes.insert(sorted_nodes.begin(), sorted_entry { none_label, {} });
				}
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			if (acquire_keyboard) {
				ImGui::SetKeyboardFocusHere();
			}

			filter.Draw();

			auto nodes_view = scoped_child("nodes view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

			uint32_t idx = 0;

			for (auto& entry : sorted_nodes) {
				auto idx_id = scoped_id(idx++);

				const auto& entry_name = entry.name;

				if (!filter.PassFilter(entry_name.c_str())) {
					continue;
				}

				const bool is_current = entry.target_id == current_source_id;

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				auto icon_result = setup.get_icon_for(entry.target_id.operator editor_node_id(), icon_in);

				const auto icon = icon_result.icon;
				const auto icon_color = setup.get_icon_color_for(entry.target_id.operator editor_node_id());
				const auto atlas_type = icon_result.atlas;

				auto result = editor_widgets::inspectable_with_icon(
					icon,
					icon_color,
					atlas_type,
					entry_name,
					white,
					0.5f,
					is_current,
					[](auto&&...) {}
				);

				if (result) {
					ImGui::CloseCurrentPopup();

					on_choice(entry.target_id, entry.name);
					return true;
				}
			}
		}
		else {
			base::mark_not_opened();
		}

		return false;
	}
};
