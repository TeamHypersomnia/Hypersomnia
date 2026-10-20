#pragma once
#include <string>
#include <algorithm>
#include "3rdparty/imgui/imgui.h"
#include "3rdparty/imgui/imgui_internal.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"

namespace augs {
	namespace imgui {
		/*
			The darker background under miniatures - e.g. a column of them.
		*/

		inline ImU32 miniature_background_color() {
			return ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.35f));
		}

		/*
			For a column of miniatures with the old ImGui::Columns: darkens the first column between two screen heights.
			Called at the end of a row, it darkens over the row's selectable too - also a colored one - but under the miniature.
		*/

		inline void darken_first_column(const float top_y, const float bottom_y) {
			auto& window = *ImGui::GetCurrentWindow();

			const auto l = window.Pos.x + ImGui::GetColumnOffset(0);
			const auto r = window.Pos.x + ImGui::GetColumnOffset(1);

			ImGui::PushColumnsBackground();
			window.DrawList->AddRectFilled(ImVec2(l, top_y), ImVec2(r, bottom_y), miniature_background_color());
			ImGui::PopColumnsBackground();
		}

		/*
			A miniature fitted into a rect with a margin, centered. Nothing when there is no miniature.
		*/

		inline void miniature_image(
			ImDrawList& draw_list,
			const augs::atlas_entry* const miniature,
			const vec2 rect_lt,
			const vec2 rect_size,
			const float margin,
			const imgui_atlas_type atlas_type
		) {
			if (miniature == nullptr) {
				return;
			}

			const auto inner_size = rect_size - vec2::square(2 * margin);
			const auto original_size = vec2(miniature->get_original_size());

			const auto fit_mult = std::min(
				inner_size.x / std::max(original_size.x, 1.f),
				inner_size.y / std::max(original_size.y, 1.f)
			);

			const auto shown_size = original_size * fit_mult;
			const auto image_lt = rect_lt + (rect_size - shown_size) / 2;

			const auto image_rb = image_lt + shown_size;

			/*
				All four corners - the atlas packer may have stored the image rotated (was_flipped).
			*/

			draw_list.AddImageQuad(
				reinterpret_cast<ImTextureID>(atlas_type),

				static_cast<ImVec2>(image_lt),
				ImVec2(image_rb.x, image_lt.y),
				static_cast<ImVec2>(image_rb),
				ImVec2(image_lt.x, image_rb.y),

				static_cast<ImVec2>(miniature->get_atlas_space_uv(vec2(0.f, 0.f))),
				static_cast<ImVec2>(miniature->get_atlas_space_uv(vec2(1.f, 0.f))),
				static_cast<ImVec2>(miniature->get_atlas_space_uv(vec2(1.f, 1.f))),
				static_cast<ImVec2>(miniature->get_atlas_space_uv(vec2(0.f, 1.f)))
			);
		}

		/*
			The highlight of a hovered or pressed miniature frame - subtle, like in the project selector.
		*/

		inline ImU32 miniature_frame_highlight(const bool active) {
			return ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, active ? 60.f / 255 : 30.f / 255));
		}

		/*
			A combo shown as a tall, transparent selectable framed like a separator:
			a miniature on the left, the chosen name centered in the space right of it, the label right of the frame.
			A click unfolds the same popup a combo would - use it like scoped_combo.
			Without a miniature, its slot stays empty.
		*/

		inline bool begin_miniature_combo(
			const std::string& label,
			const std::string& displayed,
			const augs::atlas_entry* const miniature,
			const imgui_atlas_type atlas_type,
			const ImGuiComboFlags flags = ImGuiComboFlags_HeightLargest
		) {
			auto& g = *ImGui::GetCurrentContext();
			auto* const window = ImGui::GetCurrentWindow();

			if (window->SkipItems) {
				return false;
			}

			const auto& style = g.Style;
			const auto id = window->GetID(label.c_str());

			const auto label_size = vec2(ImGui::CalcTextSize(label.c_str(), nullptr, true));
			const auto size = vec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight() * 3.f);
			const auto lt = vec2(window->DC.CursorPos);

			const auto bb = ImRect(static_cast<ImVec2>(lt), static_cast<ImVec2>(lt + size));
			const auto label_space = label_size.x > 0.f ? style.ItemInnerSpacing.x + label_size.x : 0.f;
			const auto total_bb = ImRect(static_cast<ImVec2>(lt), static_cast<ImVec2>(lt + size + vec2(label_space, 0.f)));

			ImGui::ItemSize(total_bb, style.FramePadding.y);

			if (!ImGui::ItemAdd(total_bb, id, &bb)) {
				return false;
			}

			bool hovered = false;
			bool held = false;

			const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
			const auto popup_id = ImHashStr("##ComboPopup", 0, id);
			bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);

			if (pressed && !popup_open) {
				ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
				popup_open = true;
			}

			auto& draw_list = *window->DrawList;

			if (held || popup_open || hovered) {
				draw_list.AddRectFilled(bb.Min, bb.Max, miniature_frame_highlight(held || popup_open));
			}

			/*
				The miniature's square: darker up to the frame's left, top and bottom edges, only its right edge drawn.
			*/

			const auto slot_side = size.y;
			const auto slot_rb = lt + vec2::square(slot_side);

			draw_list.AddRectFilled(bb.Min, static_cast<ImVec2>(slot_rb), miniature_background_color());
			draw_list.AddLine(ImVec2(slot_rb.x, lt.y), static_cast<ImVec2>(slot_rb), ImGui::GetColorU32(ImGuiCol_Separator));

			ImGui::RenderNavHighlight(bb, id);
			draw_list.AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Separator));

			miniature_image(draw_list, miniature, lt, vec2::square(slot_side), 6.f, atlas_type);

			{
				const auto name_pad = vec2(style.FramePadding.x, 0.f);
				const auto name_lt = vec2(lt.x + slot_side, lt.y) + name_pad;
				const auto name_rb = lt + size - name_pad;

				ImGui::RenderTextClipped(
					static_cast<ImVec2>(name_lt),
					static_cast<ImVec2>(name_rb),
					displayed.c_str(),
					nullptr,
					nullptr,
					ImVec2(0.5f, 0.5f)
				);
			}

			if (label_size.x > 0.f) {
				ImGui::RenderText(ImVec2(bb.Max.x + style.ItemInnerSpacing.x, bb.Min.y + (size.y - label_size.y) / 2), label.c_str());
			}

			if (!popup_open) {
				return false;
			}

			return ImGui::BeginComboPopup(popup_id, bb, flags);
		}

		template <class... Args>
		auto scoped_miniature_combo(Args&&... args) {
			const auto result = begin_miniature_combo(std::forward<Args>(args)...);

			auto opt = scope_guard([]() { ImGui::EndCombo(); });

			if (!result) {
				opt.release();
			}

			return opt;
		}
	}
}
