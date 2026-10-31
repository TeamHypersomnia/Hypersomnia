#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/window_framework/shell.h"

namespace augs {
	namespace imgui {
		/*
			Text drawn as a link: underlined, a hand and the url in a tooltip over it - a click opens it in the browser.
			With highlight_on_hover, a light rectangle behind it while hovered, stronger while held.
			On CrazyGames only its color: no links out of the game there.
			Returns whether it was clicked.
		*/

		inline bool text_link(const std::string& label, const std::string& url, const bool highlight_on_hover = false) {
			const auto link_col = rgba(130, 190, 255, 255);

#if WEB_CRAZYGAMES
			(void)url;
			(void)highlight_on_hover;

			auto sc = scoped_text_color(link_col);
			ImGui::TextUnformatted(label.c_str());

			return false;
#else
			const auto pos = vec2(ImGui::GetCursorScreenPos());
			const auto size = vec2(ImGui::CalcTextSize(label.c_str()));
			const auto draw_list = ImGui::GetWindowDrawList();

			const auto pad = highlight_on_hover ? vec2(4, 1) : vec2::zero;
			const auto rect_lt = ImVec2(pos - pad);
			const auto rect_rb = ImVec2(pos + size + pad);

			const bool hovered = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(rect_lt, rect_rb);

			/*
				Drawn before the text, so under it.
			*/
			if (hovered && highlight_on_hover) {
				const auto rect_col = ImGui::IsMouseDown(0) ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered;
				draw_list->AddRectFilled(rect_lt, rect_rb, ImGui::GetColorU32(rect_col), 2.0f);
			}

			{
				auto sc = scoped_text_color(link_col);
				ImGui::TextUnformatted(label.c_str());
			}

			const auto underline_y = pos.y + size.y;
			draw_list->AddLine(ImVec2(pos.x, underline_y), ImVec2(pos.x + size.x, underline_y), ImGui::GetColorU32(link_col));

			if (!hovered) {
				return false;
			}

			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			ImGui::SetTooltip("%s", url.c_str());

			if (ImGui::IsMouseClicked(0)) {
				augs::open_url(url);
				return true;
			}

			return false;
#endif
		}
	}
}
