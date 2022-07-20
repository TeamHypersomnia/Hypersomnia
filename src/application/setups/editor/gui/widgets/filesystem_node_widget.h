#pragma once
#include "application/setups/editor/gui/editor_filesystem_gui.h"

namespace editor_widgets {
	inline void filesystem_node_widget(
		editor_setup& setup,
		editor_filesystem_node& node,
		const necessary_images_in_atlas_map& necessary_images,
		const ad_hoc_in_atlas_map& ad_hoc_atlas,
		const bool filter_active,
		editor_resource_id& dragged_resource
	) {
		using namespace augs::imgui;
		augs::atlas_entry icon;

		if (filter_active) {
			if (!node.passed_filter) {
				return;
			}
		}

		if (node.is_child_of_root()) {
			/* Ignore some editor-specific files in the project folder */

			if (node.name == "editor_view.json") {
				return;
			}
		}

		auto atlas_type = augs::imgui_atlas_type::GAME;

		if (node.game_atlas_icon != std::nullopt) {
			icon = *node.game_atlas_icon;
		}
		else {
			if (node.is_folder()) {
				icon = necessary_images[assets::necessary_image_id::EDITOR_ICON_FOLDER];
			}
			else {
				if (auto ad_hoc = mapped_or_nullptr(ad_hoc_atlas, node.file_thumbnail_id)) {
					icon = *ad_hoc;
					atlas_type = augs::imgui_atlas_type::AD_HOC;
				}
				else {
					icon = necessary_images[assets::necessary_image_id::EDITOR_ICON_FILE];
				}
			}
		}

		const bool is_inspected = setup.is_inspected(node.associated_resource);

		const auto& label = node.name;
		const auto label_color = white;

		auto after_selectable_callback = [&]() {
			if (node.is_resource()) {
				if (ImGui::BeginDragDropSource()) {
					dragged_resource = node.associated_resource;

					ImGui::SetDragDropPayload("dragged_resource", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}
			}
		};

		const bool result = inspectable_with_icon(
			icon,
			atlas_type,
			label,
			label_color,
			node.level,
			is_inspected,
			after_selectable_callback,
			node.after_text
		);

		if (result) {
			if (node.is_folder()) {
				if (!filter_active) {
					node.toggle_open();
				}
			}
			else {
				if (setup.exists(node.associated_resource)) {
					setup.inspect(node.associated_resource);
				}
			}
		}
	};
}
