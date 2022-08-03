#pragma once

namespace editor_widgets {
	template <class F>
	bool inspectable_with_icon(
		const augs::atlas_entry& icon,
		const augs::imgui_atlas_type atlas_type,
		const std::string& label,
		const rgba label_color,
		const int indent_level,
		const bool is_inspected,
		F&& after_selectable_callback,
		const std::string& after_text = "",
		const uint8_t bg_alpha = 255
	) {
		using namespace augs::imgui;

		const auto max_icon_size = ImGui::GetTextLineHeight();

		const auto bg_cols = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(15, 40, 70, bg_alpha),
			rgba(35, 60, 90, bg_alpha)
		};

		const auto inspected_cols = std::array<rgba, 3> {
			rgba(35-10, 60-10, 90-10, bg_alpha),
			rgba(35, 60, 90, bg_alpha),
			rgba(35+10, 60+10, 90+10, bg_alpha)
		};

		const float size_mult = 1.1f;
		//const float padding_mult = 0.1f;

		const auto text_h = ImGui::GetTextLineHeight();
		const auto button_size = ImVec2(0, text_h * size_mult);

		//shift_cursor(vec2(0, text_h * padding_mult));

		const auto before_pos = ImGui::GetCursorPos();

		bool result = false;

		{
			auto colored_selectable = scoped_selectable_colors(is_inspected ? inspected_cols : bg_cols);
			auto id = scoped_id(label.c_str());

			result = ImGui::Selectable("###Button", is_inspected, ImGuiSelectableFlags_DrawHoveredWhenHeld, button_size);
			after_selectable_callback();
		}

		{
			auto scope = scoped_preserve_cursor();

			const float content_x_offset = max_icon_size * indent_level;

			const auto icon_size = vec2::square(max_icon_size);
			const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);
			const auto diff = (icon_size - scaled_icon_size) / 2;

			ImGui::SetCursorPos(ImVec2(vec2(before_pos) + vec2(content_x_offset, 0) + diff));

			const auto icon_padding = vec2(icon_size) / 1.5f;

			game_image(icon, scaled_icon_size, white, vec2::zero, atlas_type);

			const auto image_offset = vec2(0, button_size.y / 2 - icon_size.y / 2);
			const auto text_pos = vec2(before_pos) + image_offset + vec2(content_x_offset + icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
			ImGui::SetCursorPos(ImVec2(text_pos));
			text_color(label, label_color);

			if (after_text.size() > 0) {
				ImGui::SameLine();
				text_disabled(after_text);
			}
		}

		return result;
	}
}
