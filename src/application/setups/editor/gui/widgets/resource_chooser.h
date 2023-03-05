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

template <class R>
class resource_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;

	struct sorted_entry {
		std::string name;
		editor_typed_resource_id<R> target_id;
		
		bool operator<(const sorted_entry& b) const {
			return name < b.name;
		}
	};

	std::vector<sorted_entry> sorted_resources;
	ImGuiTextFilter filter;

public:
	template <class F>
	bool perform(
		const std::string& label, 
		const std::string& current_source,
		const editor_setup& setup,
		F on_choice
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(Invalid)") : current_source;

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				sorted_resources.clear();

				auto adder_lbd = [&](const auto& id, const auto& resource) {
					sorted_resources.push_back({ resource.get_display_name(), id });
				};

				setup.template for_each_resource<R>(adder_lbd, false);
				setup.template for_each_resource<R>(adder_lbd, true);

				sort_range(sorted_resources);
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			if (acquire_keyboard) {
				ImGui::SetKeyboardFocusHere();
			}

			filter.Draw();

			auto resources_view = scoped_child("Resources view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

			for (auto& entry : sorted_resources) {
				const auto& entry_name = entry.name;

				if (!filter.PassFilter(entry_name.c_str())) {
					continue;
				}

				const bool is_current = entry_name == current_source;

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				if (ImGui::Selectable(entry_name.c_str(), is_current)) {
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
