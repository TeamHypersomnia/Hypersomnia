#include <imgui/imgui_internal.h>

#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"

namespace augs {
	namespace imgui {
		bool selectable_with_icon(
			const augs::atlas_entry& icon,
			const std::string& label,
			const float size_mult,
			const vec2 padding_mult,
			const rgba label_color,
			const std::array<rgba, 3> bg_cols,
			const float icon_rotation,
			const bool pad_from_left,
			const std::function<void()> after_cb
		) {
			const auto text_h = ImGui::GetTextLineHeight();
			const auto button_size = ImVec2(0, text_h * size_mult);

			shift_cursor(vec2(0, text_h * padding_mult.y));

			const auto before_pos = ImGui::GetCursorPos();

			bool result = false;

			{
				auto colored_selectable = scoped_selectable_colors(bg_cols);
				auto id = scoped_id(label.c_str());

				result = ImGui::Selectable("###Button", true, ImGuiSelectableFlags_None, button_size);

				if (after_cb) {
					after_cb();
				}
			}

			{
				auto scope = scoped_preserve_cursor();

				ImGui::SetCursorPos(before_pos);

				const auto icon_size = icon.get_original_size();
				const auto icon_padding = vec2(icon_size) * padding_mult;/// 1.5f;

				const auto image_offset = vec2(pad_from_left ? icon_padding.x : 0, button_size.y / 2 - icon_size.y / 2);
				game_image(icon, icon_size, label_color, image_offset, imgui_atlas_type::GAME, icon_rotation);

				const auto text_pos = vec2(before_pos) + image_offset + vec2(icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
				ImGui::SetCursorPos(ImVec2(text_pos));
				text_color(label, label_color);
			}

			shift_cursor(vec2(0, text_h * padding_mult.y));

			return result;
		}
	}
}

namespace ImGui {
	bool BeginTabMenuBar(const float y) {
		ImGuiContext& g = *GImGui;
		SetNextWindowPos(ImVec2(0.0f, y));
		SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f + 1.f));

		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));

		if (!Begin("##MainMedasdsanuBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)
			|| !BeginMenuBar())
		{
			End();
			PopStyleVar(4);
			return false;
		}
		g.CurrentWindow->DC.MenuBarOffset.x += g.Style.DisplaySafeAreaPadding.x;
		return true;
	}

	void EndTabMenuBar() {
		EndMenuBar();
		End();
		PopStyleVar(4);
	}

	bool DragIntN(const char* label, int* v, int components, float v_speed, int* v_min, int* v_max, const char* display_format) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		return false;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		PushMultiItemsWidths(components, CalcItemWidth());
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			value_changed |= DragInt("##v", &v[i], v_speed, v_min[i], v_max[i], display_format);
			SameLine(0, g.Style.ItemInnerSpacing.x);
			PopID();
			PopItemWidth();
		}
		PopID();

		TextUnformatted(label, FindRenderedTextEnd(label));
		EndGroup();

		return value_changed;
	}
}
