#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"

#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"
#include "application/setups/editor/gui/editor_layers_gui.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/editor_setup.h"

void editor_layers_gui::perform(const editor_layers_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (dragged_node.is_set()) {
		const bool payload_still_exists = [&]() {
			const auto payload = ImGui::GetDragDropPayload();
			return payload && payload->IsDataType("dragged_node");

		}();

		if (!payload_still_exists) {
			const bool mouse_over_scene = !mouse_over_any_window();

			if (mouse_over_scene) {

			}

			dragged_node.unset();
		}
	}

	if (dragged_layer.is_set()) {
		const bool payload_still_exists = [&]() {
			const auto payload = ImGui::GetDragDropPayload();
			return payload && payload->IsDataType("dragged_layer");

		}();

		if (!payload_still_exists) {
			dragged_layer.unset();
		}
	}

	if (!window) {
		return;
	}

	const auto max_icon_size = ImGui::GetTextLineHeight();

	const bool node_or_layer_inspected = 
		in.setup.inspects_any<editor_node_id>()
		|| in.setup.inspects_any<editor_layer_id>()
	;

	thread_local ImGuiTextFilter filter;

	const auto bg_cols = std::array<rgba, 3> {
		rgba(0, 0, 0, 0),
		rgba(15, 40, 70, 255),
		rgba(35, 60, 90, 255)
	};

	const auto inspected_cols = std::array<rgba, 3> {
		rgba(35-10, 60-10, 90-10, 255),
		rgba(35, 60, 90, 255),
		rgba(35+10, 60+10, 90+10, 255)
	};

	auto icon_button = [&](
		const auto label, 
		const auto icon_id, 
		const std::string& tooltip,
		const bool enabled,
		rgba tint = white, 
		std::array<rgba, 3> icon_bg_cols = {
			rgba(0, 0, 0, 0),
			rgba(35, 60, 90, 255),
			rgba(35+10, 60+10, 90+10, 255)
		}
	) {
		auto id = scoped_id(label);

		if (!enabled) {
			tint = rgba(255, 255, 255, 80);
		}

		const auto icon = in.necessary_images[icon_id];
		const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);

		bool result = false;

		{
			auto preserved = scoped_preserve_cursor();

			auto colored_selectable = scoped_selectable_colors(icon_bg_cols);

			{
				auto scope = maybe_disabled_cols({}, !enabled);
				result = ImGui::Selectable("##ButtonBg", false, ImGuiSelectableFlags_None, ImVec2(scaled_icon_size));
			}

			if (ImGui::IsItemHovered()) {
				auto scope = scoped_tooltip();
				text_color(tooltip, tint);
			}
		}

		game_image(icon, scaled_icon_size, tint);
		return result;
	};

	if (icon_button("##NewLayer", assets::necessary_image_id::EDITOR_ICON_ADD, "New layer", true)) {
		in.setup.create_new_layer();
	}

	ImGui::SameLine();

	if (icon_button("##Duplicate", assets::necessary_image_id::EDITOR_ICON_CLONE, "Duplicate selection", node_or_layer_inspected)) {

	}

	ImGui::SameLine();

	{
		const auto remove_bgs = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(80, 20, 20, 255),
			rgba(150, 40, 40, 255)
		};

		const auto remove_tint = rgba(220, 80, 80, 255);

		if (icon_button("##Remove", assets::necessary_image_id::EDITOR_ICON_REMOVE, "Remove selection", node_or_layer_inspected, remove_tint, remove_bgs)) {

		}
	}
	
	ImGui::SameLine();

	acquire_keyboard_once();
	filter_with_hint(filter, "##HierarchyFilter", "Search objects and layers...");

	//const auto& style = ImGui::GetStyle();
	const auto disabled_color = rgba(255, 255, 255, 110);

	auto get_node_icon = [&]<typename T>(const T& node) -> std::pair<augs::atlas_entry, augs::imgui_atlas_type> {
		if constexpr(std::is_same_v<T, editor_sprite_node>) {
			const auto* maybe_resource = in.setup.find_resource(node.resource_id);

			if (maybe_resource != nullptr) {
				if (auto ad_hoc = mapped_or_nullptr(in.ad_hoc_atlas, maybe_resource->thumbnail_id)) {
					return { *ad_hoc, augs::imgui_atlas_type::AD_HOC };
				}
			}
		}

		return { in.necessary_images[assets::necessary_image_id::DEFUSING_INDICATOR], augs::imgui_atlas_type::GAME };
	};

	int node_id_counter = 0;

	auto create_dragged_resource_in = [&](const auto layer_id, const auto index) {
		if (in.dragged_resource == nullptr) {
			return;
		}

		auto instantiate = [&]<typename T>(const T& typed_resource, const auto resource_id) {
			const auto resource_name = typed_resource.get_display_name();
			const auto new_name = in.setup.get_free_node_name_for(resource_name);

			using node_type = typename T::node_type;

			node_type new_node;
			new_node.resource_id = resource_id;
			new_node.unique_name = new_name;

			create_node_command<node_type> command;

			command.built_description = typesafe_sprintf("Created %x", new_name);
			command.created_node = std::move(new_node);
			command.layer_id = layer_id;
			command.index_in_layer = index;

			in.setup.post_new_command(command);
		};

		in.setup.on_resource(
			in.dragged_resource->associated_resource,
			instantiate
		);

		in.dragged_resource = nullptr;
	};

	auto accept_dragged_nodes = [&](
		editor_layer_id target_layer_id, 
		const std::size_t target_index
	) {
		const auto all_inspected = in.setup.get_all_inspected<editor_node_id>();

		reorder_nodes_command command;

		command.target_layer_id = target_layer_id;
		command.target_index = target_index;

		if (in.setup.is_inspected(dragged_node) && all_inspected.size() > 1) {
			command.nodes_to_move = all_inspected;
			command.built_description = typesafe_sprintf("Reordered %x nodes", all_inspected.size());
		}
		else {
			command.nodes_to_move = { dragged_node };
			command.built_description = typesafe_sprintf("Reordered %x", in.setup.get_name(dragged_node));
		}

		in.setup.post_new_command(command);
	};

	auto accept_dragged_layers = [&](
		const std::size_t target_index
	) {
		const auto all_inspected = in.setup.get_all_inspected<editor_layer_id>();

		reorder_layers_command command;
		command.target_index = target_index;

		if (in.setup.is_inspected(dragged_layer) && all_inspected.size() > 1) {
			command.layers_to_move = all_inspected;
			command.built_description = typesafe_sprintf("Reordered %x layers", all_inspected.size());
		}
		else {
			command.layers_to_move = { dragged_layer };
			command.built_description = typesafe_sprintf("Reordered %x", in.setup.get_name(dragged_layer));
		}

		in.setup.post_new_command(command);
	};

	auto handle_node_and_id = [&]<typename T>(
		const std::size_t node_index, 
		T& node, 
		const editor_node_id node_id, 
		editor_layer& layer, 
		editor_layer_id layer_id
	) {
		auto id_scope = scoped_id(node_id_counter++);

		const auto icon_result = get_node_icon(node);
		const auto icon = icon_result.first;
		const auto atlas_type = icon_result.second;

		const auto label = node.get_display_name();

		const float size_mult = 1.1f;
		const auto text_h = ImGui::GetTextLineHeight();
		const auto button_size = ImVec2(0, text_h * size_mult);
		const int node_level = 1;

		const bool node_disabled = !node.visible || !layer.visible;

		if (node_disabled) {
			ImGui::PushStyleColor(ImGuiCol_Text, disabled_color.operator ImVec4());
		}

		{
			const auto before_pos = ImGui::GetCursorPos();

			{
				const bool is_inspected = in.setup.is_inspected(node_id);

				auto colored_selectable = scoped_selectable_colors(is_inspected ? inspected_cols : bg_cols);
				auto id = scoped_id(label.c_str());

				if (ImGui::Selectable("###NodeButton", is_inspected, ImGuiSelectableFlags_DrawHoveredWhenHeld, button_size)) {
					in.setup.inspect(node_id);
				}

				if (ImGui::BeginDragDropSource()) {
					dragged_node = node_id;

					ImGui::SetDragDropPayload("dragged_node", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_resource")) {
						create_dragged_resource_in(layer_id, node_index);
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_node")) {
						accept_dragged_nodes(layer_id, node_index);
					}

					ImGui::EndDragDropTarget();
				}
			}

			{
				auto scope = scoped_preserve_cursor();

				const float content_x_offset = max_icon_size * node_level;

				const auto icon_size = vec2::square(max_icon_size);
				const auto scaled_icon_size = vec2::scaled_to_max_size(icon.get_original_size(), max_icon_size);
				const auto diff = (icon_size - scaled_icon_size) / 2;

				ImGui::SetCursorPos(ImVec2(vec2(before_pos) + vec2(content_x_offset, 0) + diff));

				const auto icon_padding = vec2(icon_size) / 1.5f;

				const auto image_color = node_disabled ? disabled_color : white;
				game_image(icon, scaled_icon_size, image_color, vec2::zero, atlas_type);

				const auto image_offset = vec2(0, button_size.y / 2 - icon_size.y / 2);
				const auto text_pos = vec2(before_pos) + image_offset + vec2(content_x_offset + icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
				ImGui::SetCursorPos(ImVec2(text_pos));
				text(label);
			}

			ImGui::NextColumn();

			{
				const auto visibility_icon = in.necessary_images[node.visible
					? assets::necessary_image_id::EDITOR_ICON_VISIBLE
					: assets::necessary_image_id::EDITOR_ICON_HIDDEN
				];

				const auto scaled_icon_size = vec2::scaled_to_max_size(visibility_icon.get_original_size(), max_icon_size);
				const auto cols = node_disabled ? colors_nha { disabled_color, disabled_color, disabled_color } : colors_nha{};

				if (game_image_button("###NodeVisibility", visibility_icon, scaled_icon_size, cols, augs::imgui_atlas_type::GAME)) {
					node.visible = !node.visible;
				}
			}

			ImGui::NextColumn();
		}

		if (node_disabled) {
			ImGui::PopStyleColor();
		}
	};

	{
		//auto child = scoped_child("hierarchy view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		auto& layers = in.layers;

		int id_counter = 0;

		const auto avail = ImGui::GetContentRegionAvail().x;

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, avail - max_icon_size);

		for (const auto& layer_id : layers.order) {
			auto* maybe_layer = in.setup.find_layer(layer_id);

			if (maybe_layer == nullptr) {
				continue;
			}

			auto& layer = *maybe_layer;
			const bool was_disabled = !layer.visible;

			auto id_scope = scoped_id(id_counter++);
			const auto label = layer.unique_name;

			using namespace augs::imgui;

			if (filter.IsActive()) {
				if (!layer.passed_filter) {
					return;
				}
			}

			{
				const bool is_inspected = in.setup.is_inspected(layer_id);

				auto colored_selectable = scoped_selectable_colors(is_inspected ? inspected_cols : bg_cols);

				if (was_disabled) {
					ImGui::PushStyleColor(ImGuiCol_Text, disabled_color.operator ImVec4());
				}

				const auto before_pos = ImGui::GetCursorPos();

				const auto flags = 
					ImGuiSelectableFlags_AllowDoubleClick | 
					ImGuiSelectableFlags_AllowItemOverlap
				;

				const bool tree_node_pressed = ImGui::Selectable("###HierarchyButton", is_inspected, flags);

				if (ImGui::BeginDragDropSource()) {
					dragged_layer = layer_id;

					ImGui::SetDragDropPayload("dragged_layer", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_resource")) {
						create_dragged_resource_in(layer_id, 0);
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_layer")) {
						accept_dragged_layers(index_in(layers.order, layer_id));
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_node")) {
						accept_dragged_nodes(layer_id, 0);
					}

					ImGui::EndDragDropTarget();
				}

				if (tree_node_pressed) {
					in.setup.inspect(layer_id);
					if (ImGui::IsMouseDoubleClicked(0)) {
						layer.is_open = !layer.is_open;
					}
				}

				{
					ImGui::SetCursorPos(before_pos);

					const auto dir = layer.is_open ? ImGuiDir_Down : ImGuiDir_Right;

					const bool layer_is_empty = layer.hierarchy.nodes.empty();

					{
						auto col = scoped_style_color(ImGuiCol_Button, rgba(0, 0, 0, 0));
						auto col2 = cond_scoped_style_color(layer_is_empty, ImGuiCol_Text, rgba(255, 255, 255, 60));

						if (ImGui::ArrowButtonEx("###IsOpen", dir, { max_icon_size, max_icon_size })) {
							layer.is_open = !layer.is_open;
						}
					}

					ImGui::SameLine();

					text(layer.unique_name);
				}

				if (was_disabled) {
					ImGui::PopStyleColor();
				}
			}

			ImGui::NextColumn();

			{
				const auto visibility_icon = in.necessary_images[layer.visible
					? assets::necessary_image_id::EDITOR_ICON_VISIBLE
					: assets::necessary_image_id::EDITOR_ICON_HIDDEN
				];

				const auto scaled_icon_size = vec2::scaled_to_max_size(visibility_icon.get_original_size(), max_icon_size);
				const auto cols = was_disabled ? colors_nha { disabled_color, disabled_color, disabled_color } : colors_nha{};

				if (game_image_button("###Visibility", visibility_icon, scaled_icon_size, cols, augs::imgui_atlas_type::GAME)) {
					layer.visible = !layer.visible;
				}
			}

			ImGui::NextColumn();

			if (layer.is_open) {
				for (std::size_t i = 0; i < layer.hierarchy.nodes.size(); ++i) {
					const auto node_id = layer.hierarchy.nodes[i];

					in.setup.on_node(node_id, [&](auto& typed_node, const auto typed_node_id) {
						(void)typed_node_id;
						handle_node_and_id(i, typed_node, node_id, layer, layer_id);
					});
				}
			}
		}
	}
}
