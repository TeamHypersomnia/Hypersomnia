#pragma once
#include <array>
#include "3rdparty/imgui/imgui.h"
#include "augs/graphics/rgba.h"
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

namespace augs {
	namespace imgui {
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

		inline void draw_cross_overlay(
			const vec2i is, 
			const vec2i where, 
			const int zoom,
			const rgba col
		) {
			draw_rect_local(
				ltrb::from_points(
					vec2(where.x, 0) * zoom,
					vec2(where.x + 1, is.y) * zoom
				),
				col
			);

			draw_rect_local(
				ltrb::from_points(
					vec2(0, where.y) * zoom,
					vec2(is.x, where.y + 1) * zoom
				),
				col
			);
		};

		inline void draw_ray_overlay(const vec2i is, const vec2i center, const int degrees, const int zoom, const rgba col) {
			std::array<vec2, 4> points;
			const auto w = 1.f / zoom;
			points[0] = vec2(center.x, center.y - w);
			points[1] = vec2(center.x + is.x, center.y - w);
			points[2] = vec2(center.x + is.x, center.y + w);
			points[3] = vec2(center.x, center.y + w);

			for (auto& p : points) {
				p.rotate(static_cast<float>(degrees), vec2(center));
				p *= zoom;
			}

			draw_quad_local(points, col);
		};

		inline void draw_segment(const vec2i a, const vec2i b, const rgba col, const float thickness) {
			const auto cpos = vec2(ImGui::GetCursorScreenPos());

			ImGui::GetWindowDrawList()->AddLine(
				static_cast<ImVec2>(cpos + a), 
				static_cast<ImVec2>(cpos + b), 
				ImGui::GetColorU32(col),
				thickness
			);
		};
	}
}
