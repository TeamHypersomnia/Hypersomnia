#pragma once
#include <cmath>
#include <string>
#include <unordered_map>
#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"
#include "augs/graphics/rgba.h"

namespace augs {
	namespace imgui {
		/*
			A line of text after a status icon drawn by ImGui itself - sharp at any scale, in the theme's colors.
		*/

		inline void status_icon_gap(const float size) {
			ImGui::Dummy(ImVec2(size, size));
			ImGui::SameLine();
		}

		inline void checkmark_line(const std::string& label, const rgba col) {
			const auto size = ImGui::GetTextLineHeight();
			const auto pos = ImGui::GetCursorScreenPos();

			ImGui::RenderCheckMark(ImGui::GetWindowDrawList(), ImVec2(pos.x + size * 0.1f, pos.y + size * 0.1f), ImGui::GetColorU32(ImVec4(col)), size * 0.8f);

			status_icon_gap(size);
			ImGui::TextUnformatted(label.c_str());
		}

		/*
			Three quarters of a ring turning with time, for something still on its way.
		*/

		inline void spinner(const float size, const rgba col) {
			const auto pos = ImGui::GetCursorScreenPos();
			const auto radius = size * 0.4f;
			const auto center = ImVec2(pos.x + size * 0.5f, pos.y + size * 0.5f);
			const auto start = static_cast<float>(ImGui::GetTime()) * 6.0f;

			auto* const draw_list = ImGui::GetWindowDrawList();
			draw_list->PathClear();
			draw_list->PathArcTo(center, radius, start, start + 3.14159265f * 1.5f, 24);
			draw_list->PathStroke(ImGui::GetColorU32(ImVec4(col)), 0, size * 0.12f);

			ImGui::Dummy(ImVec2(size, size));
		}

		/*
			A copy icon (two overlapping sheets) that copies the text when clicked and shows a check mark for a moment.
		*/

		inline bool copy_icon_button(const std::string& id, const std::string& text_to_copy) {
			static std::unordered_map<ImGuiID, double> copied_at;

			const auto size = ImGui::GetTextLineHeight();
			const auto pos = ImGui::GetCursorScreenPos();
			const auto now = ImGui::GetTime();

			ImGui::PushID(id.c_str());
			const auto key = ImGui::GetID("##copy");
			const bool clicked = ImGui::InvisibleButton("##copy", ImVec2(size, size));
			const bool hovered = ImGui::IsItemHovered();
			ImGui::PopID();

			if (clicked) {
				ImGui::SetClipboardText(text_to_copy.c_str());
				copied_at[key] = now;
			}

			const auto found = copied_at.find(key);
			const bool just_copied = found != copied_at.end() && now - found->second < 1.5;

			auto* const draw_list = ImGui::GetWindowDrawList();

			if (hovered) {
				draw_list->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(255, 255, 255, 30), size * 0.15f);
			}

			if (just_copied) {
				ImGui::RenderCheckMark(draw_list, ImVec2(pos.x + size * 0.15f, pos.y + size * 0.15f), IM_COL32(0, 200, 0, 255), size * 0.7f);
			}
			else {
				const auto col = ImGui::GetColorU32(ImGuiCol_Text);
				const auto sheet = size * 0.5f;
				const auto back = ImVec2(pos.x + size * 0.18f, pos.y + size * 0.18f);
				const auto front = ImVec2(pos.x + size * 0.34f, pos.y + size * 0.34f);

				draw_list->AddRect(back, ImVec2(back.x + sheet, back.y + sheet), col, size * 0.08f, 0, 1.2f);
				draw_list->AddRectFilled(front, ImVec2(front.x + sheet, front.y + sheet), ImGui::GetColorU32(ImGuiCol_WindowBg), size * 0.08f);
				draw_list->AddRect(front, ImVec2(front.x + sheet, front.y + sheet), col, size * 0.08f, 0, 1.2f);
			}

			if (hovered) {
				ImGui::SetTooltip("%s", just_copied ? "Copied" : "Copy");
			}

			return clicked;
		}

		inline void spinner_line(const std::string& label, const rgba col) {
			const auto size = ImGui::GetTextLineHeight();

			spinner(size, col);
			ImGui::SameLine();
			ImGui::TextUnformatted(label.c_str());
		}
	}
}
