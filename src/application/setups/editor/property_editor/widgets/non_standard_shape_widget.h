#pragma once
#include "view/viewables/images_in_atlas_map.h"
#include "application/setups/editor/property_editor/tweaker_type.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_drawers.h"
#include "augs/misc/from_concave_polygon.h"
#include "augs/templates/wrap_templates.h"

struct non_standard_shape_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;
	const bool nodeize;

	template <class T>
	static constexpr bool handles = std::is_same_v<T, image_shape_type>;

	template <class T>
	static constexpr bool handles_prologue = false;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		(void)to;
		return typesafe_sprintf("Altered %x", formatted_label);
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		auto iw = scoped_item_width(80);

		auto perform_widget = [&]() {
			if (ImGui::Button("Reset")) {
				result = tweaker_type::DISCRETE;
				object.clear();
			}

			const auto& entry = game_atlas.at(id);

			const auto is = vec2i(entry.get_original_size());
			const auto zoom = 4;

			const auto viewing_size = (is * zoom).operator ImVec2();

			text("Image size: %x, zoom: %x", is, zoom);

			invisible_button_reset_cursor("###VertexSelector", viewing_size);
			game_image(entry.diffuse, viewing_size);

			const auto segment_alpha = 180;

			const auto hs = vec2(is) * 0.5f;
			const auto& reference_point = hs;

			auto segment = [&](const auto a, const auto b, rgba col) {
				col.a = segment_alpha;
				draw_segment((reference_point + a) * zoom, (reference_point + b) * zoom, col, 2.f);
			};

			auto considered = [&]() {
				if (object.empty()) {
					T box;

					auto& v = box.original_poly;

					v.resize(4);
					v[0].set(-hs.x, -hs.y);
					v[1].set( hs.x, -hs.y);
					v[2].set( hs.x,  hs.y);
					v[3].set(-hs.x,  hs.y);

					return box;
				}

				return object;
			}();
			
			auto& considered_poly = considered.original_poly;

			const auto n = considered_poly.size();

			const auto& io = ImGui::GetIO();

			const auto mpos = [&]() {
				const auto pos = ImGui::GetCursorScreenPos();
				const auto image_space_new = vec2i(vec2(io.MousePos.x - pos.x, io.MousePos.y - pos.y) / zoom);
				return vec2(image_space_new - reference_point);
			}();

			const auto closest_i = [&]() {
				return static_cast<std::size_t>(minimum_i(
					std::as_const(considered_poly),
					[&](const auto& a, const auto& b) {
						return a - mpos < b - mpos;
					}
				));
			}();

			//const auto& closest_v = considered_poly[closest_i];

			const auto closest_prev = wrap_prev(n, closest_i);
			const auto closest_next = wrap_next(n, closest_i);

			const auto closest_edge_a = [&]() {
				std::size_t ci = 0;
				float dist = -1.f;

				for (std::size_t i = 0; i < considered_poly.size(); ++i) {
					const auto seg = vec2::segment_type{ considered_poly[i], wrap_next(considered_poly, i) };
					const auto new_dist = mpos.distance_from_segment_sq(seg);

					if (dist == -1.f || new_dist < dist) {
						dist = new_dist;
						ci = i;
					}
				}

				return ci;
			}();

			const auto closest_edge_b = wrap_next(n, closest_edge_a);

			const bool hovered = ImGui::IsItemHovered();

			const bool is_adding_mode = n < considered_poly.max_size() && io.KeyShift;
			const bool is_removing_mode = n > 3 && io.KeyAlt;
			const bool is_normal_mode = !is_removing_mode && !is_adding_mode;
			const bool is_show_partition_mode = is_normal_mode && io.KeyCtrl;

			if (is_show_partition_mode) {
				if (considered.take_vertices_one_after_another()) {
					for (std::size_t i = 0; i < considered_poly.size(); ++i) {
						const auto& a = considered_poly[i];
						const auto& b = wrap_next(considered_poly, i);

						segment(a, b, green);
					}
				}
				else {
					const auto& cp = considered.convex_partition;

					for (std::size_t i = 0; i < cp.size(); ++i) {
						const auto& a = considered_poly[cp[i]];
						const auto& b = considered_poly[wrap_next(cp, i)];

						segment(a, b, orange);
					}
				}
			}
			else {
				for (std::size_t i = 0; i < considered_poly.size(); ++i) {
					auto a = considered_poly[i];
					auto b = wrap_next(considered_poly, i);

					if (hovered) {
						if (is_normal_mode) {
							if (i == closest_prev) {
								segment(a, mpos, green);
							}
							else if (i == closest_i) {
								segment(mpos, b, green);
							}

							segment(a, b, white);
						}
						else if (is_adding_mode) {
							if (i == closest_edge_a) {
								segment(a, mpos, yellow);
							}
							else if (i == closest_edge_b) {
								segment(mpos, a, yellow);
							}

							segment(a, b, white);
						}
						else if (is_removing_mode) {
							if (i == closest_prev) {
								segment(a, considered_poly[closest_next], white);
								segment(a, b, red);
							}
							else if (i == closest_i) {
								segment(a, b, red);
							}
							else {
								segment(a, b, white);
							}
						}
					}
					else {
						segment(a, b, white);
					}
				}

				if (ImGui::IsItemClicked()) {
					result = tweaker_type::DISCRETE;

					if (is_normal_mode) {
						considered_poly[closest_i] = mpos;
					}
					else if (is_adding_mode) {
						considered_poly.insert(considered_poly.begin() + closest_edge_b, mpos);
					}
					else if (is_removing_mode) {
						considered_poly.erase(considered_poly.begin() + closest_i);
					}

					object = considered;
				}
			}
			invisible_button("###VertexSelectorAfter", viewing_size);
		};

		auto perform_standard_widget = [&]() {
			/* Perform the standard widget for manual tweaking */
			auto iw = scoped_item_width(-1);

			for (auto& v : object.original_poly) {
				auto id = scoped_id(index_in(object.original_poly, v));

				if (drag_vec2(identity_label, v)) {
					result = tweaker_type::CONTINUOUS;
				}
			}
		};

		if (nodeize) {
			const auto node_label = "" + identity_label;

			auto node = scoped_tree_node(node_label.c_str());

			if (node) {
				perform_widget();
				perform_standard_widget();
			}
		}
		else {
			if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick...", ImGuiComboFlags_HeightLargest)) {
				perform_widget();
				perform_standard_widget();
			}
		}

		if (result.has_value()) {
		}

		return result;
	}
};
