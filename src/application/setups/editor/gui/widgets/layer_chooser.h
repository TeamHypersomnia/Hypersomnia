#pragma once
#include <vector>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/maybe_official_path.h"
#include "application/setups/debugger/property_debugger/browsed_path_entry_base.h"
#include "application/setups/debugger/property_debugger/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/log.h"
#include "application/setups/editor/gui/widgets/inspectable_with_icon.h"
#include "application/setups/editor/editor_get_icon_for.h"

class layer_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;
	using id_type = editor_layer_id;

	struct sorted_entry {
		std::string name;
		id_type target_id;
	};

	std::vector<sorted_entry> sorted_layers;
	ImGuiTextFilter filter;

public:
	template <class F, class L>
	bool perform(
		const std::string& label, 
		const std::string& current_source,
		const id_type current_source_id,
		const editor_setup& setup,
		const bool allow_none,
		F on_choice,
		L should_add,
		const std::string none_label = "(None)"
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(Invalid)") : current_source;

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				sorted_layers.clear();

				auto adder_lbd = [&](const auto& id, const auto& layer) {
					if (should_add(layer)) {
						sorted_layers.push_back({ layer.get_display_name(), id });
					}
				};

				for (const auto& id : setup.get_project().layers.order) {
					adder_lbd(id, *setup.find_layer(id));
				}

				if (allow_none) {
					sorted_layers.insert(sorted_layers.begin(), sorted_entry { none_label, {} });
				}
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			if (acquire_keyboard) {
				ImGui::SetKeyboardFocusHere();
			}

			filter.Draw();

			auto layers_view = scoped_child("layers view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

			uint32_t idx = 0;

			for (auto& entry : sorted_layers) {
				auto idx_id = scoped_id(idx++);

				const auto& entry_name = entry.name;

				if (!filter.PassFilter(entry_name.c_str())) {
					continue;
				}

				const bool is_current = entry.target_id == current_source_id;

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				auto result = ImGui::Selectable(entry_name.c_str(), is_current);

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
