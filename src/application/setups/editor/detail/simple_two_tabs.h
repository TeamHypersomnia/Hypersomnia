#pragma once

namespace augs {
	namespace imgui {
		template <class E, class F>
		void simple_two_tabs(
			E& current_tab, 
			const E a,
			const E b,
			const std::string& a_str,
			const std::string& b_str,
			F after_first,
			const std::array<rgba, 3>  bg_cols = {
				rgba(0, 0, 0, 0),
				rgba(15, 40, 70, 255),
				rgba(35, 60, 90, 255)
			},

			const std::array<rgba, 3> selected_cols = {
				rgba(35-10, 60-10, 90-10, 255),
				rgba(35, 60, 90, 255),
				rgba(35+10, 60+10, 90+10, 255)
			}
		) {
			const auto avail = ImGui::GetContentRegionAvail();
			auto spacing = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			auto tab_button = [&](const std::string& label, const E type) {
				const bool selected = current_tab == type;
				auto cols = scoped_button_colors(selected ? selected_cols : bg_cols);

				if (ImGui::Button(label.c_str(), ImVec2(avail.x / 2, 0))) {
					current_tab = type;
				}
			};

			tab_button(a_str, a);
			after_first();
			ImGui::SameLine();
			tab_button(b_str, b);
		}

		template <class E, class... Args>
		void simple_tabs(
			E& current_tab,
			Args&&... args
		) {
			const auto bg_cols = std::array<rgba, 3> {
				rgba(0, 0, 0, 0),
				rgba(15, 40, 70, 255),
				rgba(35, 60, 90, 255)
			};

			const auto selected_cols = std::array<rgba, 3> {
				rgba(35-10, 60-10, 90-10, 255),
				rgba(35, 60, 90, 255),
				rgba(35+10, 60+10, 90+10, 255)
			};

			const auto avail = ImGui::GetContentRegionAvail();
			auto spacing = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			auto tab_button = [&](const std::string& label, const E type) {
				const bool selected = current_tab == type;
				auto cols = scoped_button_colors(selected ? selected_cols : bg_cols);

				if (ImGui::Button(label.c_str(), ImVec2(avail.x / sizeof...(args), 0))) {
					current_tab = type;
				}
			};

			int samelines = sizeof...(args) - 1;

			auto f = [&](auto entry) {
				tab_button(entry.first, entry.second);

				if (samelines) {
					--samelines;
					ImGui::SameLine();
				}
			};

			(f(args), ...);
		}
	}
}


