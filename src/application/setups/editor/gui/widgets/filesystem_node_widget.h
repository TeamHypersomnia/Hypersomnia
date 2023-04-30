#pragma once
#include "application/setups/editor/gui/editor_filesystem_gui.h"
#include "application/setups/editor/editor_get_icon_for.h"

namespace editor_widgets {
	template <class F>
	inline bool filesystem_node_widget(
		editor_setup& setup,
		const editor_filesystem_node& node,
		const editor_icon_info_in& icon_in,
		editor_resource_id& dragged_resource,
		const std::optional<bool> override_is_inspected,
		F after_selectable
	) {
		using namespace augs::imgui;
		augs::atlas_entry icon;

		auto atlas_type = augs::imgui_atlas_type::GAME;

		auto icon_color = white;

		if (node.necessary_atlas_icon.has_value()) {
			icon = icon_in.necessary_images[*node.necessary_atlas_icon];
		}
		else {
			if (node.is_folder()) {
				icon = icon_in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FOLDER];
			}
			else if (node.is_resource()) {
				auto result = setup.get_icon_for(node.associated_resource, icon_in);
				icon = result.icon;
				icon_color = setup.get_icon_color_for(node.associated_resource);
				atlas_type = result.atlas;
			}
			else {
				icon = icon_in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FILE];
			}
		}

		const bool is_inspected = override_is_inspected.has_value() ? override_is_inspected.value() : setup.is_inspected(node.associated_resource);
		const bool inspects_via_node = [&]() {
			bool found_node = false;

			setup.for_each_inspected<editor_node_id>([&](const editor_node_id& id){
				setup.on_node(id, [&](const auto& typed_node, const auto id) {
					(void)id;

					if (node.associated_resource == typed_node.resource_id.operator editor_resource_id()) {
						found_node = true;
					}
				});
			});

			return found_node;
		}();

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

			after_selectable();
		};

		auto bg_alpha = 255;

		if (!is_inspected && inspects_via_node) {
			bg_alpha = 120;
		}

		return inspectable_with_icon(
			icon,
			icon_color,
			atlas_type,
			label,
			label_color,
			node.level,
			is_inspected || inspects_via_node,
			after_selectable_callback,
			node.after_text,
			bg_alpha
		);
	};
}
