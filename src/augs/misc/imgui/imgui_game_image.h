#pragma once
#include <imgui/imgui.h>

#include "augs/graphics/imgui_payload.h"
#include "augs/texture_atlas/atlas_entry.h"

namespace augs {
	namespace imgui {
		template <class T>
		void invisible_button(const std::string& id, const T& size) {
			ImGui::InvisibleButton(id.c_str(), static_cast<ImVec2>(size));
		}

		template <class T>
		void invisible_button_reset_cursor(const std::string& id, const T& size) {
			const auto local_pos = ImGui::GetCursorPos();
			invisible_button(id, size);
			ImGui::SetCursorPos(local_pos);
		}

		template <class T>
		void game_image(const augs::atlas_entry& entry, const T& size) {
			const auto imsize = static_cast<ImVec2>(size);

			std::array<vec2, 4> texcoords = {
				vec2(0.f, 0.f),
				vec2(1.f, 0.f),
				vec2(1.f, 1.f),
				vec2(0.f, 1.f)
			};

			for (auto& t : texcoords) {
				t = entry.get_atlas_space_uv(t);
			}

			const auto cpos = ImGui::GetCursorScreenPos();

			ImGui::GetWindowDrawList()->AddImageQuad(
				reinterpret_cast<ImTextureID>(augs::imgui_atlas_type::GAME),
				cpos,
				{ cpos.x + imsize.x, cpos.y },
				{ cpos.x + imsize.x, cpos.y + imsize.y },
				{ cpos.x, cpos.y + imsize.y },

				static_cast<ImVec2>(texcoords[0]),
				static_cast<ImVec2>(texcoords[1]),
				static_cast<ImVec2>(texcoords[2]),
				static_cast<ImVec2>(texcoords[3])
			);
		}

		template <class T>
		inline auto game_image_button(const std::string& id, const augs::atlas_entry& entry, const T& size) {
			invisible_button_reset_cursor(id, size);
			game_image(entry, size);
		}

		inline auto game_image_button(const std::string& id, const augs::atlas_entry& entry) {
			return game_image_button(id, entry, entry.get_original_size());
		}

		inline void draw_rect_local(const ltrb& origin, const rgba color) {
			const auto cpos = vec2(ImGui::GetCursorScreenPos());

			ImGui::GetWindowDrawList()->AddRectFilled(
				static_cast<ImVec2>(cpos + origin.left_top()), 
				static_cast<ImVec2>(cpos + origin.right_bottom()), 
				ImGui::GetColorU32(color),
				0.f
			);
		}

		inline void draw_quad_local(const std::array<vec2, 4>& points, const rgba color) {
			const auto cpos = vec2(ImGui::GetCursorScreenPos());

			ImGui::GetWindowDrawList()->AddQuadFilled(
				static_cast<ImVec2>(cpos + points[0]), 
				static_cast<ImVec2>(cpos + points[1]), 
				static_cast<ImVec2>(cpos + points[2]), 
				static_cast<ImVec2>(cpos + points[3]), 
				ImGui::GetColorU32(color)
			);
		}
	}
}
