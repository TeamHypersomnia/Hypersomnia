#pragma once
#include <cmath>
#include <string>
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

		inline void spinner_line(const std::string& label, const rgba col) {
			const auto size = ImGui::GetTextLineHeight();

			spinner(size, col);
			ImGui::SameLine();
			ImGui::TextUnformatted(label.c_str());
		}
	}
}
