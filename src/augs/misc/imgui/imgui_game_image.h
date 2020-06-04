#pragma once
#include "3rdparty/imgui/imgui.h"

#include "augs/graphics/imgui_payload.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/graphics/rgba.h"

namespace augs {
	namespace imgui {
		template <class T>
		bool invisible_button(const std::string& id, const T& size) {
			return ImGui::InvisibleButton(id.c_str(), static_cast<ImVec2>(size));
		}

		template <class T>
		bool invisible_button_reset_cursor(const std::string& id, const T& size) {
			const auto local_pos = ImGui::GetCursorPos();
			const auto result = invisible_button(id, size);
			ImGui::SetCursorPos(local_pos);

			return result;
		}

		template <class T>
		void rect_filled(const T& size, const rgba col = white, const vec2 offset = vec2::zero) {
			const auto imsize = static_cast<ImVec2>(size);

			const auto cpos = vec2(ImGui::GetCursorScreenPos()) + offset;

			ImGui::GetWindowDrawList()->AddQuadFilled(
				static_cast<ImVec2>(cpos),
				{ cpos.x + imsize.x, cpos.y },
				{ cpos.x + imsize.x, cpos.y + imsize.y },
				{ cpos.x, cpos.y + imsize.y },

				ImGui::ColorConvertFloat4ToU32(col.operator ImVec4())
			);
		}

		template <class T>
		void game_image(const augs::atlas_entry& entry, const T& size, const rgba col = white, const vec2 offset = vec2::zero, const imgui_atlas_type atlas_type = imgui_atlas_type::GAME, float rotation = 0.f) {
			const auto imsize = static_cast<vec2>(size);

			std::array<vec2, 4> texcoords = {
				vec2(0.f, 0.f),
				vec2(1.f, 0.f),
				vec2(1.f, 1.f),
				vec2(0.f, 1.f)
			};

			for (auto& t : texcoords) {
				t = entry.get_atlas_space_uv(t);
			}

			const auto cpos = vec2(ImGui::GetCursorScreenPos()) + offset;

			auto points = std::array<vec2, 4> {
				cpos,
				{ cpos.x + imsize.x, cpos.y },
				{ cpos.x + imsize.x, cpos.y + imsize.y },
				{ cpos.x, cpos.y + imsize.y }
			};

			if (rotation != 0.f) {
				const auto origin = cpos + imsize * 0.5f;

				for (auto& p : points) {
					p.rotate(rotation, origin);
				}
			}

			ImGui::GetWindowDrawList()->AddImageQuad(
				reinterpret_cast<ImTextureID>(atlas_type),

				static_cast<ImVec2>(points[0]),
				static_cast<ImVec2>(points[1]),
				static_cast<ImVec2>(points[2]),
				static_cast<ImVec2>(points[3]),

				static_cast<ImVec2>(texcoords[0]),
				static_cast<ImVec2>(texcoords[1]),
				static_cast<ImVec2>(texcoords[2]),
				static_cast<ImVec2>(texcoords[3]),
				ImGui::ColorConvertFloat4ToU32(col.operator ImVec4())
			);
		}

		inline void game_image(const augs::atlas_entry& entry, const rgba col = white, const vec2 offset = vec2::zero, const imgui_atlas_type atlas_type = imgui_atlas_type::GAME) {
			game_image(entry, entry.get_original_size(), col, offset, atlas_type);
		}

		struct colors_nha {
			rgba normal = white;
			rgba hovered = white;
			rgba active = white;

			static auto all_white() {
				return colors_nha { white, white, white };
			}

			static auto standard() {
				return colors_nha { rgba(255, 255, 255, 220), rgba(255, 255, 255, 240), rgba(255, 255, 255, 255) };
			}
		};

		template <class T>
		bool game_image_button(const std::string& id, const augs::atlas_entry& entry, const T& size, const colors_nha cols = colors_nha{}, const imgui_atlas_type atlas_type = imgui_atlas_type::GAME) {
			const auto local_pos = ImGui::GetCursorPos();
			const auto result = invisible_button(id, size);
			const auto after_pos = ImGui::GetCursorPos();

			rgba target_color = cols.normal;

			if (ImGui::IsItemHovered()) {
				target_color = cols.hovered;
			}

			if (ImGui::IsItemActive()) {
				target_color = cols.active;
			}

			ImGui::SetCursorPos(local_pos);
			game_image(entry, size, target_color, vec2::zero, atlas_type);
			ImGui::SetCursorPos(after_pos);

			return result;
		}

		inline bool game_image_button(const std::string& id, const augs::atlas_entry& entry, const colors_nha cols = colors_nha{}, const imgui_atlas_type atlas_type = imgui_atlas_type::GAME) {
			return game_image_button(id, entry, entry.get_original_size(), cols, atlas_type);
		}
	}
}
