#pragma once
#include <algorithm>
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

		/*
			The usual "opens outside" mark: a box open at the top right, an arrow leaving through it.
		*/

		inline void external_link_icon(const ImVec2 pos, const float size, const ImU32 col) {
			auto* const draw_list = ImGui::GetWindowDrawList();
			const auto thickness = std::max(1.0f, size * 0.09f);
			const auto box_min = ImVec2(pos.x + size * 0.12f, pos.y + size * 0.3f);
			const auto box_max = ImVec2(pos.x + size * 0.7f, pos.y + size * 0.88f);
			const auto gap = size * 0.28f;

			draw_list->AddLine(ImVec2(box_min.x + gap, box_min.y), box_min, col, thickness);
			draw_list->AddLine(box_min, ImVec2(box_min.x, box_max.y), col, thickness);
			draw_list->AddLine(ImVec2(box_min.x, box_max.y), box_max, col, thickness);
			draw_list->AddLine(box_max, ImVec2(box_max.x, box_max.y - gap), col, thickness);

			const auto tip = ImVec2(pos.x + size * 0.9f, pos.y + size * 0.1f);
			const auto head = size * 0.3f;

			draw_list->AddLine(ImVec2(pos.x + size * 0.42f, pos.y + size * 0.58f), tip, col, thickness);
			draw_list->AddLine(tip, ImVec2(tip.x - head, tip.y), col, thickness);
			draw_list->AddLine(tip, ImVec2(tip.x, tip.y + head), col, thickness);
		}

		/*
			Text that opens a page in the browser: a light highlight on hover, a tooltip and the external link mark.
			Returns true when clicked - the caller opens the page.
		*/

		inline bool external_link(const std::string& id, const std::string& label, const std::string& tooltip, const ImU32 col) {
			const auto size = ImGui::GetTextLineHeight();
			const auto label_size = ImGui::CalcTextSize(label.c_str());
			const auto pos = ImGui::GetCursorScreenPos();
			const auto width = label_size.x + size * 0.3f + size;

			ImGui::PushID(id.c_str());
			const bool clicked = ImGui::InvisibleButton("##external", ImVec2(width, size));
			const bool hovered = ImGui::IsItemHovered();
			const bool held = ImGui::IsItemActive();
			ImGui::PopID();

			auto* const draw_list = ImGui::GetWindowDrawList();

			if (hovered || held) {
				const auto highlight = held ? IM_COL32(255, 255, 255, 70) : IM_COL32(255, 255, 255, 30);
				draw_list->AddRectFilled(ImVec2(pos.x - 2, pos.y), ImVec2(pos.x + width + 2, pos.y + size), highlight, size * 0.15f);
				ImGui::SetTooltip("%s", tooltip.c_str());
			}

			draw_list->AddText(pos, col, label.c_str());
			external_link_icon(ImVec2(pos.x + label_size.x + size * 0.3f, pos.y), size, col);

			return clicked;
		}

		/*
			A pencil, the usual "edit" mark - filled, from its tip at the bottom left up to the top right.
		*/

		inline void edit_icon(const ImVec2 pos, const float size, const ImU32 col, const ImU32 gap_col) {
			auto* const draw_list = ImGui::GetWindowDrawList();

			const auto along = ImVec2(0.7071f, -0.7071f);
			const auto across = ImVec2(0.7071f, 0.7071f);
			const auto half_width = size * 0.13f;

			auto at = [&](const ImVec2 from, const ImVec2 dir, const float len) {
				return ImVec2(from.x + dir.x * len, from.y + dir.y * len);
			};

			const auto tip = ImVec2(pos.x + size * 0.12f, pos.y + size * 0.88f);
			const auto tip_base = at(tip, along, size * 0.25f);
			const auto end = at(tip_base, along, size * 0.6f);

			draw_list->AddTriangleFilled(tip, at(tip_base, across, half_width), at(tip_base, across, -half_width), col);
			draw_list->AddQuadFilled(
				at(tip_base, across, half_width),
				at(end, across, half_width),
				at(end, across, -half_width),
				at(tip_base, across, -half_width),
				col
			);

			const auto band = at(end, along, -size * 0.16f);
			const auto thickness = std::max(1.0f, size * 0.07f);

			draw_list->AddLine(at(tip_base, across, half_width), at(tip_base, across, -half_width), gap_col, thickness);
			draw_list->AddLine(at(band, across, half_width), at(band, across, -half_width), gap_col, thickness);
		}

		/*
			An image that edits when clicked: on hover a light frame and a pencil in its corner.
			Call after drawing the image at pos; returns true when clicked.
		*/

		inline bool editable_image_overlay(const std::string& id, const ImVec2 pos, const ImVec2 size, const std::string& tooltip) {
			ImGui::SetCursorScreenPos(pos);

			ImGui::PushID(id.c_str());
			const bool clicked = ImGui::InvisibleButton("##editable", size);
			const bool hovered = ImGui::IsItemHovered();
			const bool held = ImGui::IsItemActive();
			ImGui::PopID();

			if (hovered || held) {
				auto* const draw_list = ImGui::GetWindowDrawList();
				const auto max = ImVec2(pos.x + size.x, pos.y + size.y);
				const auto frame_col = held ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 200);

				draw_list->AddRectFilled(pos, max, IM_COL32(0, 0, 0, held ? 110 : 70));
				draw_list->AddRect(pos, max, frame_col, 0.0f, 0, 2.0f);

				const auto icon = std::min(size.x, size.y) * 0.3f;
				const auto badge_center = ImVec2(max.x - icon * 0.75f, max.y - icon * 0.75f);
				const auto badge_col = IM_COL32(20, 20, 20, 220);

				draw_list->AddCircleFilled(badge_center, icon * 0.68f, badge_col);
				edit_icon(ImVec2(badge_center.x - icon * 0.4f, badge_center.y - icon * 0.4f), icon * 0.8f, frame_col, badge_col);

				ImGui::SetTooltip("%s", tooltip.c_str());
			}

			return clicked;
		}

		/*
			A padlock (e.g. before a server with a password): a filled body, the shackle a half ring above it.
		*/

		inline void padlock(const float size, const rgba col) {
			const auto pos = ImGui::GetCursorScreenPos();
			const auto col_u32 = ImGui::GetColorU32(ImVec4(col));
			auto* const draw_list = ImGui::GetWindowDrawList();

			const auto body_min = ImVec2(pos.x + size * 0.15f, pos.y + size * 0.45f);
			const auto body_max = ImVec2(pos.x + size * 0.75f, pos.y + size * 0.92f);
			const auto shackle_center = ImVec2((body_min.x + body_max.x) / 2, body_min.y);

			draw_list->PathClear();
			draw_list->PathArcTo(shackle_center, size * 0.2f, 3.14159265f, 3.14159265f * 2, 12);
			draw_list->PathStroke(col_u32, 0, std::max(1.0f, size * 0.09f));
			draw_list->AddRectFilled(body_min, body_max, col_u32, size * 0.08f);

			ImGui::Dummy(ImVec2(size * 0.8f, size));
		}

		/*
			A five-pointed star (favorites): gold and filled, or only its outline - drawn by ImGui, no glyph needed.
		*/

		inline void star_shape(ImDrawList* const draw_list, const ImVec2 center, const float radius, const ImU32 col, const bool filled) {
			ImVec2 points[10];

			for (int i = 0; i < 10; ++i) {
				const auto r = (i % 2 == 0) ? radius : radius * 0.45f;
				const auto angle = -3.14159265f / 2 + i * 3.14159265f / 5;
				points[i] = ImVec2(center.x + std::cos(angle) * r, center.y + std::sin(angle) * r);
			}

			if (filled) {
				/*
					Not convex - filled as five triangles around the middle, and the middle pentagon.
				*/

				for (int i = 0; i < 10; i += 2) {
					draw_list->AddTriangleFilled(points[i], points[(i + 1) % 10], points[(i + 9) % 10], col);
				}

				const ImVec2 inner[5] = { points[1], points[3], points[5], points[7], points[9] };
				draw_list->AddConvexPolyFilled(inner, 5, col);
			}
			else {
				draw_list->AddPolyline(points, 10, col, ImDrawFlags_Closed, std::max(1.0f, radius * 0.15f));
			}
		}

		inline void spinner_line(const std::string& label, const rgba col) {
			const auto size = ImGui::GetTextLineHeight();

			spinner(size, col);
			ImGui::SameLine();
			ImGui::TextUnformatted(label.c_str());
		}
	}
}
