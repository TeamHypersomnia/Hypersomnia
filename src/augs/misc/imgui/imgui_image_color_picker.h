#pragma once
#include "augs/misc/imgui/imgui_drawers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/texture_atlas/atlas_entry.h"

namespace augs {
	namespace imgui {
		inline std::optional<rgba> image_color_picker(
			const int zoom,
			const augs::atlas_entry& entry,
			const augs::image& viewed_image
		) {
			auto& io = ImGui::GetIO();

			const auto is = vec2i(entry.get_original_size());

			const auto viewing_size = (is * zoom).operator ImVec2();

			text("Image size: %x, zoom: %x", is, zoom);

			invisible_button_reset_cursor("###ColorSelector", viewing_size);
			game_image(entry, viewing_size);

			const auto cross_alpha = 100;

			auto draw_cross = [is, zoom](const vec2i where, const rgba col) {
				draw_cross_overlay(is, where, zoom, col);
			};

			const auto pos = ImGui::GetCursorScreenPos();

			const auto image_space_new = vec2i(vec2(io.MousePos.x - pos.x, io.MousePos.y - pos.y) / zoom);

			std::optional<rgba> result;

			if (ImGui::IsItemClicked()) {
				result = viewed_image.pixel(image_space_new);
			}

			if (ImGui::IsItemHovered()) {
				draw_cross(image_space_new, rgba(green.rgb(), cross_alpha));

				auto scope = scoped_tooltip();
				text("Image space: %x", image_space_new);
				rgba color_preview = viewed_image.pixel(image_space_new);
				color_edit("##colorpreview", color_preview);
			}

			invisible_button("###ColorSelectorAfter", viewing_size);

			return result;
		}
	}
}
