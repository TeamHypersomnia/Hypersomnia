#pragma once
#include "view/viewables/images_in_atlas_map.h"
#include "application/setups/editor/property_editor/tweaker_type.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_drawers.h"

struct image_offset_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;
	const bool nodeize;

	template <class T>
	static constexpr bool handles = is_one_of_v<T, vec2i, transformi>;

	template <class T>
	static constexpr bool handles_prologue = false;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x to %x", formatted_label, to);
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		constexpr bool only_vec = std::is_same_v<T, vec2i>;

		auto& current_pos = [&]() -> vec2i& {
			if constexpr(only_vec) {
				return object;
			}
			else {
				return object.pos;
			}
		}();

		std::optional<tweaker_type> result;

		auto iw = scoped_item_width(80);

		auto perform_widget = [&]() {
			const auto& entry = game_atlas.at(id);

			const auto is = vec2i(entry.get_original_size());
			const auto zoom = 4;

			const auto viewing_size = (is * zoom).operator ImVec2();

			text("Image size: %x, zoom: %x", is, zoom);

			invisible_button_reset_cursor("###OffsetSelector", viewing_size);
			game_image(entry.diffuse, viewing_size);

			const auto cross_alpha = 200;
			const auto ray_alpha = 120;

			auto draw_ray = [is](const vec2i center, const int degrees, const rgba col) {
				draw_ray_overlay(is, center, degrees, zoom, col);
			};

			auto draw_cross = [is](const vec2i where, const rgba col) {
				draw_cross_overlay(is, where, zoom, col);
			};

			const bool reference_to_the_right = identity_label == "##bullet_spawn";

			const auto reference_point = 
				reference_to_the_right ? 
				vec2i(is.x - 1, is.y / 2) 
				: vec2i(is / 2)
			;

			const auto pos = ImGui::GetCursorScreenPos();

			const auto& io = ImGui::GetIO();

			const auto image_space_new = vec2i(vec2(io.MousePos.x - pos.x, io.MousePos.y - pos.y) / zoom);
			const auto image_space_old = reference_point + current_pos;

			const auto chosen_new_offset = image_space_new - reference_point;

			draw_cross(image_space_old, rgba(red.rgb(), cross_alpha));

			if constexpr(!only_vec) {
				draw_ray(image_space_old, object.rotation, rgba(white.rgb(), ray_alpha));
			}
			else {
				(void)ray_alpha;
				(void)draw_ray;
			}

			const bool pos_mode = only_vec || !io.KeyShift;

			if (pos_mode) {
				if (ImGui::IsItemClicked()) {
					current_pos = chosen_new_offset;
					result = tweaker_type::DISCRETE;
				}

				if (ImGui::IsItemHovered()) {
					draw_cross(image_space_new, rgba(green.rgb(), cross_alpha));

					text_tooltip("Chosen offset: %x\nImage space: %x", chosen_new_offset, image_space_new);
				}
			}
			else {
				if constexpr(!only_vec) {
					const auto degrees_new = (image_space_new - image_space_old).degrees();

					if (ImGui::IsItemClicked()) {
						object.rotation = degrees_new;
						result = tweaker_type::DISCRETE;
					}

					if (ImGui::IsItemHovered()) {
						draw_cross(image_space_new, rgba(green.rgb(), ray_alpha));
						draw_ray(image_space_old, degrees_new, rgba(green.rgb(), cross_alpha));

						text_tooltip("Chosen rotation: %x", degrees_new);
					}
				}
			}

			invisible_button("###OffsetSelectorAfter", viewing_size);
		};

		auto perform_standard_widget = [&]() {
			/* Perform the standard widget for manual tweaking */
			ImGui::SameLine();

			auto iw = scoped_item_width(-1);

			if constexpr(only_vec) {
				if (drag_vec2(identity_label, object)) {
					result = tweaker_type::CONTINUOUS;
				}
			}
			else {
				if (drag_transform(identity_label, object)) {
					result = tweaker_type::CONTINUOUS;
				}
			}
		};

		if (nodeize) {
			const auto node_label = "" + identity_label;

			auto node = scoped_tree_node(node_label.c_str());

			perform_standard_widget();

			if (node) {
				perform_widget();
			}
		}
		else {
			if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick...", ImGuiComboFlags_HeightLargest)) {
				perform_widget();
			}

			perform_standard_widget();
		}

		return result;
	}
};
