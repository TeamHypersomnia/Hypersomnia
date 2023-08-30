#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_controls.h"
#include "augs/misc/imgui/imgui_game_image.h"

#include "application/setups/editor/editor_filesystem.h"

#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/project/editor_layers.h"
#include "application/setups/editor/gui/editor_layers_gui.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/gui/widgets/icon_button.h"
#include "application/setups/editor/editor_get_icon_for.h"
#include "application/setups/editor/detail/make_command_from_selections.h"
#include "application/setups/editor/resources/resource_traits.h"
#include "application/setups/editor/defaults/editor_node_defaults.h"

#include "application/setups/editor/detail/make_command_from_selections.h"

void editor_layers_gui::perform(const editor_layers_input in) {
	using namespace augs::imgui;
	using namespace editor_widgets;

	(void)in;
	
	if (nodeize_request.is_set()) {
		in.setup.unpack_prefab(nodeize_request);
		nodeize_request.unset();
	}

	entity_to_highlight.unset();

	auto release_enter = augs::scope_guard([this]() {
		if (request_confirm_rename) {
			ImGui::GetIO().KeysDown[int(augs::event::keys::key::ENTER)] = false;
		}
	});

	bool rename_requested = request_rename;
	const bool has_double_click = !in.setup.wants_multiple_selection() && in.setup.handle_doubleclick_in_layers_gui;
	in.setup.handle_doubleclick_in_layers_gui = false;
	request_rename = false;

	bool inspect_next_item = false;
	bool inspect_next_layer = false;
	std::optional<inspected_variant> previous_item;

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

	const auto dragged_cols = std::array<rgba, 3> {
		rgba(70, 70, 70, 255),
		rgba(0, 0, 0, 255),
		rgba(0, 0, 0, 255)
	};

	const auto hovered_cols = std::array<rgba, 3> {
		rgba(15, 40, 70, 255),
		rgba(15, 40, 70, 255),
		rgba(15, 40, 70, 255)
	};

	const auto hovered_inspected_cols = std::array<rgba, 3> {
		rgba(35+10, 60+10, 90+10, 255),
		rgba(35+10, 60+10, 90+10, 255),
		rgba(35+10, 60+10, 90+10, 255)
	};

	auto new_layer_drag_drop_callback = [&]() {
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_node")) {
				in.setup.move_dragged_to_new_layer(dragged_node);
			}

			ImGui::EndDragDropTarget();
		}
	};

	if (icon_button("##NewLayer", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_ADD], new_layer_drag_drop_callback, "New layer (N)\nMove selection to new layer (Shift+N)\nYou can drag & drop nodes on this button.", true)) {
		in.setup.create_new_layer();
	}

	ImGui::SameLine();

	if (icon_button("##Clone", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CLONE], [](){}, "Clone selection (C)", node_or_layer_inspected)) {
		const bool move_selection = false;
		in.setup.clone_selection(move_selection);
	}

	ImGui::SameLine();

	{
		const auto remove_bgs = std::array<rgba, 3> {
			rgba(0, 0, 0, 0),
			rgba(80, 20, 20, 255),
			rgba(150, 40, 40, 255)
		};

		const auto remove_tint = rgba(220, 80, 80, 255);

		if (icon_button("##Remove", in.necessary_images[assets::necessary_image_id::EDITOR_ICON_REMOVE], [](){}, "Delete selection (Ctrl+D/DEL)", node_or_layer_inspected, remove_tint, remove_bgs)) {
			in.setup.delete_selection();
		}
	}
	
	ImGui::SameLine();

	acquire_keyboard_once();
	filter_with_hint(filter, "##HierarchyFilter", "Filter layers/nodes... (CTRL+L)");

	if (filter.IsActive()) {
		for (auto& l : in.layers.pool) {
			const auto layer_name = l.get_display_name();
			l.passed_filter = filter.PassFilter(layer_name.c_str());

			for (auto& n : l.hierarchy.nodes) {
				in.setup.on_node(
					n,
					[&](const auto& typed_node, const auto) {
						const auto node_name = typed_node.get_display_name();
						typed_node.passed_filter = filter.PassFilter(node_name.c_str());

						if (typed_node.passed_filter) {
							l.passed_filter = true;
						}
					}
				);
			}
		}
	}

	auto do_collapse_all_opts = [&]() {
		ImGui::Separator();

		bool all_collapsed = true;
		bool none_collapsed = true;

		if (in.layers.pool.empty()) {
			all_collapsed = false;
			none_collapsed = false;
		}

		for (auto& l : in.layers.pool) {
			if (l.is_open) {
				all_collapsed = false;
			}
			else {
				none_collapsed = false;
			}
		}

		{
			auto disabled = maybe_disabled_cols(all_collapsed);

			if (ImGui::Selectable("Collapse all layers")) {
				for (auto& l : in.layers.pool) {
					l.is_open = false;
				}
			}
		}

		{
			auto disabled = maybe_disabled_cols(none_collapsed);

			if (ImGui::Selectable("Expand all layers")) {
				for (auto& l : in.layers.pool) {
					l.is_open = true;
				}
			}
		}
	};

	//const auto& style = ImGui::GetStyle();
	const auto disabled_color = rgba(255, 255, 255, 110);

	int node_id_counter = 0;

	auto create_dragged_resource_in = [&](const auto layer_id, const auto index, const vec2 pos) {
		if (!in.dragged_resource.is_set()) {
			return;
		}

		auto instantiate = [&]<typename T>(const T& typed_resource, const auto resource_id) {
			const auto resource_name = typed_resource.get_display_name();

			using node_type = typename T::node_type;

			node_type new_node;
			::setup_node_defaults(new_node.editable, typed_resource);

			new_node.resource_id = resource_id;
			new_node.unique_name = resource_name;
			new_node.editable.pos = pos;

			create_node_command<node_type> command;

			command.built_description = typesafe_sprintf("Created %x", new_node.unique_name);
			command.created_node = std::move(new_node);
			command.layer_id = layer_id;
			command.index_in_layer = index;

			in.setup.post_new_command(command);
		};

		in.setup.on_resource(
			in.dragged_resource,
			[&]<typename T>(const T& typed_resource, const auto resource_id) {
				if constexpr(can_be_instantiated_v<T>) {
					instantiate(typed_resource, resource_id);
				}
			}
		);

		in.dragged_resource.unset();
	};

	auto move_nodes_to_layer = [&](
		editor_node_id anchor_node_id, 
		editor_layer_id target_layer_id, 
		const std::size_t target_index
	) {
		const auto all_inspected = in.setup.get_all_inspected<editor_node_id>();

		reorder_nodes_command command;

		command.target_layer_id = target_layer_id;
		command.target_index = target_index;

		if (in.setup.is_inspected(anchor_node_id) && all_inspected.size() > 1) {
			command.nodes_to_move = all_inspected;
			command.built_description = typesafe_sprintf("Reordered %x nodes", all_inspected.size());
		}
		else {
			command.nodes_to_move = { anchor_node_id };
			command.built_description = typesafe_sprintf("Reordered %x", in.setup.get_name(anchor_node_id));
		}

		in.setup.post_new_command(command);
	};

	auto accept_dragged_nodes = [&](
		editor_layer_id target_layer_id, 
		const std::size_t target_index
	) {
		move_nodes_to_layer(dragged_node, target_layer_id, target_index);
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
		const editor_typed_node_id<T> typed_node_id, 
		editor_layer& layer, 
		const editor_layer_id layer_id
	) {
		auto id_scope = scoped_id(node_id_counter++);

		const auto node_id = typed_node_id.operator editor_node_id();

		const auto icon_result = in.setup.get_icon_for(node, in);
		const auto icon = icon_result.icon;
		const auto icon_color = in.setup.get_icon_color_for(node);
		const auto atlas_type = icon_result.atlas;

		const auto label = node.get_display_name();

		const float size_mult = 1.1f;
		const auto text_h = ImGui::GetTextLineHeight();
		const auto button_size = ImVec2(0, std::round(text_h * size_mult));
		const int node_level = 1;

		const bool node_disabled = !node.active || !layer.is_active();

		bool scroll_here = false;

		{
			const auto before_pos = ImGui::GetCursorPos();

			if (inspect_next_item) {
				inspect_next_item = false;
				in.setup.inspect_only(node_id);
				scroll_here = true;
			}

			bool is_inspected = in.setup.is_inspected(node_id);

			if (is_inspected) {
				if (!pressed_arrow.is_zero()) {
					if (pressed_arrow.y == -1 && previous_item.has_value()) {
						in.setup.inspect_only(*previous_item);
						in.setup.scroll_once_to(*previous_item);
						previous_item = std::nullopt;
						is_inspected = false;
					}

					if (pressed_arrow.y == 1) {
						inspect_next_item = true;
					}

					if (pressed_arrow.x == -1) {
						//layer.is_open = false;
						in.setup.inspect_only(layer_id);
						in.setup.scroll_once_to(layer_id);
					}

					if (pressed_arrow.x == 1) {
						inspect_next_layer = true;
					}

					pressed_arrow.set(0, 0);
				}
			}

			previous_item = node_id;

			bool rename_this_node = false;

			{
				auto id = scoped_id(label.c_str());

				const bool is_hovered_on_scene = node_id == in.setup.get_hovered_node();
				auto payload = ImGui::GetDragDropPayload();

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				ImGuiID sid = window->GetID("###NodeButton");

				const bool is_dragged = window && payload && payload->SourceId == sid;

				const auto final_cols = [&]() {
					if (is_dragged) {
						return dragged_cols;
					}

					if (is_hovered_on_scene) {
						if (is_inspected) {
							return hovered_inspected_cols;
						}

						return hovered_cols;
					}
					else {
						if (is_inspected) {
							return inspected_cols;
						}

						return bg_cols;
					}
				}();

				auto colored_selectable = scoped_selectable_colors(final_cols);

				if (ImGui::Selectable("###NodeButton", is_inspected || is_hovered_on_scene || is_dragged, 0, button_size)) {
					if (ImGui::GetIO().KeyShift && !in.setup.wants_multiple_selection()) {
						auto last_inspected = in.setup.get_last_inspected_layer_or_node();
						auto last_node = std::get_if<editor_node_id>(&last_inspected);

						if (in.setup.inspects_any<editor_node_id>() && last_node && *last_node != node_id) {
							auto& ordered = in.setup.get_layers();

							int state = 0;

							in.setup.clear_inspector();

							for (const editor_layer_id lid : ordered) {
								auto layer = in.setup.find_layer(lid);
								ensure(layer != nullptr);

								if (filter.IsActive() && !layer->passed_filter) {
									continue;
								}

								for (const editor_node_id nid : layer->hierarchy.nodes) {
									bool passed = true;

									in.setup.on_node(
										nid,
										[&](const auto& typed_node, const auto) {
											if (filter.IsActive() && !typed_node.passed_filter) {
												passed = false;
											}
										}
									);

									if (!passed) {
										continue;
									}


									if (nid == node_id || nid == *last_node) {
										++state;
									}

									if (state) {
										in.setup.inspect_add_quiet(nid);
									}

									if (state == 2) {
										break;
									}
								}

								if (state == 2) {
									break;
								}
							}

							in.setup.after_quietly_adding_inspected();
							in.setup.quiet_set_last_inspected_layer_or_node(*last_node);
						}
						else {
							in.setup.inspect(node_id);
						}
					}
					else {
						in.setup.inspect(node_id);
					}
				}

				bool skip_scroll = false;
				if (ImGui::BeginPopupContextItem()) {
					//in.setup.inspect_only(node_id);

					if (ImGui::Selectable("Rename (F2)")) {
						rename_this_node = true;
					}

					if (ImGui::BeginMenu("Move to layer...")) {
						const auto& ordered = in.setup.get_layers();

						thread_local ImGuiTextFilter layer_filter;

						filter_with_hint(layer_filter, "##LayersFilter", "Filter layers...");

						for (const editor_layer_id lid : ordered) {
							auto layer = in.setup.find_layer(lid);

							ensure(layer != nullptr);

							if (layer == nullptr) {
								continue;
							}

							const auto name = layer->get_display_name();

							if (layer_filter.IsActive() && !layer_filter.PassFilter(name.c_str())) {
								continue;
							}

							const bool highlight = layer_id == lid;

							auto colored_selectable = scoped_selectable_colors(highlight ? inspected_cols : bg_cols);

							auto disabled = maybe_disabled_only_cols(!layer->is_active());

							if (ImGui::Selectable(name.c_str(), highlight)) {
								move_nodes_to_layer(node_id, lid, 0);
								layer->is_open = true;
								in.setup.scroll_once_to(node_id);
								skip_scroll = true;
							}

							if (const bool layer_is_empty = layer->hierarchy.nodes.empty()) {
								ImGui::SameLine();
								text_disabled("(empty)");
							}
						}

						ImGui::EndMenu();
					}

					if constexpr(std::is_same_v<T, editor_prefab_node>) {
						if (ImGui::Selectable("Unpack prefab")) {
							nodeize_request = typed_node_id;
						}
					}

					do_collapse_all_opts();

					ImGui::EndPopup();
				}

				const bool selectable_double_clicked = ImGui::IsItemHovered() && has_double_click;

				if (selectable_double_clicked) {
					in.setup.center_view_at(node_id);
				}

				if (ImGui::IsItemHovered()) {
					entity_to_highlight = in.setup.to_entity_id(node_id);
				}

				if (!skip_scroll) {
					if (scroll_here || scroll_once_to == inspected_variant(node_id)) {
						scroll_once_to = std::nullopt;

						ImGui::SetScrollHereY(0.5f);
					}
				}

				if (ImGui::BeginDragDropSource()) {
					dragged_node = node_id;

					ImGui::SetDragDropPayload("dragged_node", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_resource")) {
						create_dragged_resource_in(layer_id, node_index, node.get_transform().pos);
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
				game_image(icon, scaled_icon_size, icon_color * image_color, vec2::zero, atlas_type);

				const auto image_offset = vec2(0, button_size.y / 2 - icon_size.y / 2);
				const auto text_pos = vec2(before_pos) + image_offset + vec2(content_x_offset + icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
				ImGui::SetCursorPos(ImVec2(text_pos));

				bool just_set = false;

				if (rename_this_node || (rename_requested && is_inspected)) {
					if (!is_inspected) {
						in.setup.inspect(node_id);
					}

					rename_requested = false;
					rename_this_node = false;

					currently_renamed_object = node_id;
					ImGui::SetKeyboardFocusHere();
					just_set = true;
				}

				if (currently_renamed_object == inspected_variant(node_id)) {
					auto edited_node_name = node.unique_name;

					if (input_text<100>("##InlineNameInput", edited_node_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
						currently_renamed_object = std::nullopt;

						if (edited_node_name != node.unique_name && !edited_node_name.empty()) {
							auto cmd = in.setup.make_command_from_selected_nodes<rename_node_command>("Renamed ");
							cmd.after = edited_node_name;

							if (cmd.size() == 1) {
								cmd.built_description = typesafe_sprintf("Renamed node to %x", cmd.after);
							}

							if (!cmd.empty()) {
								in.setup.post_new_command(std::move(cmd)); 
							}
						}
					}

					if (!just_set && !ImGui::IsItemActive()) {
						currently_renamed_object = std::nullopt;
					}
				}
				else {
					auto disabled = maybe_disabled_cols(node_disabled);
					text(label);
				}
			}

			ImGui::NextColumn();

			{
				const auto visibility_icon = in.necessary_images[node.active
					? assets::necessary_image_id::EDITOR_ICON_VISIBLE
					: assets::necessary_image_id::EDITOR_ICON_HIDDEN
				];

				const auto scaled_icon_size = vec2::scaled_to_max_size(visibility_icon.get_original_size(), max_icon_size);
				const auto cols = node_disabled ? colors_nha { disabled_color, disabled_color, disabled_color } : colors_nha{};

				if (game_image_button("###NodeVisibility", visibility_icon, scaled_icon_size, cols, augs::imgui_atlas_type::GAME)) {
					const auto next_value = !node.active;

					if (in.setup.is_inspected(node_id)) {
						auto command = in.setup.make_command_from_selected_nodes<toggle_nodes_active_command>(next_value ? "Enabled " : "Disabled ");
						command.next_value = next_value;
						in.setup.post_new_command(std::move(command));
					}
					else {
						auto command = toggle_nodes_active_command(); 
						command.push_entry(node_id);
						command.built_description = (next_value ? "Enabled " : "Disabled ") + node.get_display_name();
						command.next_value = next_value;
						command.update_inspector = false;
						in.setup.post_new_command(std::move(command));
					}
				}
			}

			ImGui::NextColumn();
		}
	};

	{
		auto child = scoped_child("hierarchy view");

		auto& layers = in.layers;

		int id_counter = 0;

		const auto avail = ImGui::GetContentRegionAvail().x;
		const auto spacing = ImGui::GetStyle().ItemSpacing;

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, avail - max_icon_size - spacing.x);

		std::optional<editor_layer_id> previous_layer;

		for (const auto& layer_id : layers.order) {
			auto save_prev = augs::scope_guard([&]() { previous_layer = layer_id; });

			auto* maybe_layer = in.setup.find_layer(layer_id);

			if (maybe_layer == nullptr) {
				continue;
			}

			auto& layer = *maybe_layer;
			const bool layer_disabled = !layer.is_active();

			auto id_scope = scoped_id(id_counter++);
			const auto label = layer.unique_name;

			using namespace augs::imgui;

			if (filter.IsActive()) {
				if (!layer.passed_filter) {
					continue;
				}
			}

			bool rename_this_layer = false;
			bool scroll_here = false;

			{
				const bool all_children_inspected = [&]() {
					if (layer.hierarchy.nodes.empty()) {
						return false;
					}

					for (const auto child_node_id : layer.hierarchy.nodes) {
						if (!in.setup.is_inspected(child_node_id)) {
							return false;
						}
					}

					return true;
				}();

				if (inspect_next_item || inspect_next_layer) {
					inspect_next_item = false;
					inspect_next_layer = false;
					in.setup.inspect_only(layer_id);
					scroll_here = true;
				}

				const bool is_inspected = in.setup.is_inspected(layer_id);

				if (is_inspected) {
					if (!pressed_arrow.is_zero()) {
						if (pressed_arrow.y == -1 && previous_item.has_value()) {
							in.setup.inspect_only(*previous_item);
							in.setup.scroll_once_to(*previous_item);
						}

						if (pressed_arrow.y == 1) {
							inspect_next_item = true;
						}

						if (pressed_arrow.x == 1) {
							if (layer.is_open || layer.empty()) {
								inspect_next_layer = true;
							}
							else {
								layer.is_open = true;
							}
						}

						if (pressed_arrow.x == -1) {
							if (!layer.is_open || layer.empty()) {
								if (previous_layer.has_value()) {
									in.setup.inspect_only(*previous_layer);
									in.setup.scroll_once_to(*previous_layer);
								}
							}
							else {
								layer.is_open = false;
							}
						}

						pressed_arrow.set(0, 0);
					}
				}

				const bool is_visually_inspected = is_inspected || all_children_inspected;

				previous_item = layer_id;

				auto payload = ImGui::GetDragDropPayload();

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				ImGuiID sid = window->GetID("###LayerButton");

				const bool is_dragged = window && payload && payload->SourceId == sid;

				auto colored_selectable = scoped_selectable_colors(is_dragged ? dragged_cols : (is_visually_inspected ? inspected_cols : bg_cols));

				const auto before_pos = ImGui::GetCursorPos();

				auto flags = 
					layer.empty() ?
					ImGuiSelectableFlags_AllowDoubleClick :
					ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap
				;

				const bool tree_node_pressed = ImGui::Selectable("###LayerButton", is_dragged || is_visually_inspected, flags);

				if (ImGui::BeginPopupContextItem()) {
					//in.setup.inspect_only(layer_id);

					if (ImGui::Selectable("Rename (F2)")) {
						rename_this_layer = true;
					}

					do_collapse_all_opts();

					ImGui::EndPopup();
				}

				if (scroll_here || scroll_once_to == inspected_variant(layer_id)) {
					scroll_once_to = std::nullopt;

					ImGui::SetScrollHereY(0.5f);
				}

				if (ImGui::BeginDragDropSource()) {
					dragged_layer = layer_id;

					ImGui::SetDragDropPayload("dragged_layer", nullptr, 0);
					text(label);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_resource")) {
						auto average = vec2::zero;
						int count = 0;

						for (const auto& child_node_id : layer.hierarchy.nodes) {
							in.setup.on_node(child_node_id, [&](const auto& n, auto) { average += n.get_transform().pos; ++count; });
						}

						if (count != 0) {
							average /= count;
						}

						create_dragged_resource_in(layer_id, 0, average);
						layer.is_open = true;
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_layer")) {
						accept_dragged_layers(index_in(layers.order, layer_id));
					}

					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("dragged_node")) {
						accept_dragged_nodes(layer_id, 0);
						layer.is_open = true;
					}

					ImGui::EndDragDropTarget();
				}

				if (tree_node_pressed) {
					if (in.setup.wants_multiple_selection()) {
						if (in.setup.inspects_any<editor_node_id>()) {
							if (all_children_inspected) {
								auto node_in_this_layer = [&](const inspected_variant iv) {
									if (const auto id = std::get_if<editor_node_id>(std::addressof(iv))) {
										return found_in(layer.hierarchy.nodes, *id);
									}

									return false;
								};

								in.setup.inspect_erase_if(node_in_this_layer);
							}
							else {
								for (const auto child_node_id : layer.hierarchy.nodes) {
									if (!in.setup.is_inspected(child_node_id)) {
										in.setup.inspect_add_quiet(child_node_id);
									}
								}

								in.setup.after_quietly_adding_inspected();
							}
						}
						else {
							in.setup.inspect(layer_id);
						}
					}
					else {
						if (ImGui::GetIO().KeyShift) {
							auto last_inspected = in.setup.get_last_inspected_layer_or_node();
							auto last_layer = std::get_if<editor_layer_id>(&last_inspected);
							auto last_node = std::get_if<editor_node_id>(&last_inspected);

							if (in.setup.inspects_any<editor_layer_id>() && last_layer && *last_layer != layer_id) {
								auto& ordered = in.setup.get_layers();

								int state = 0;

								in.setup.clear_inspector();

								for (const editor_layer_id id : ordered) {
									auto layer = in.setup.find_layer(id);
									ensure(layer != nullptr);

									if (filter.IsActive() && !layer->passed_filter) {
										continue;
									}

									if (id == layer_id || id == *last_layer) {
										++state;
									}

									if (state) {
										in.setup.inspect_add_quiet(id);
									}

									if (state == 2) {
										break;
									}
								}

								in.setup.after_quietly_adding_inspected();
								in.setup.quiet_set_last_inspected_layer_or_node(*last_layer);
							}
							else if (in.setup.inspects_any<editor_node_id>() && last_node) {
								auto& ordered = in.setup.get_layers();

								int state = 0;

								in.setup.clear_inspector();

								for (const editor_layer_id lid : ordered) {
									if (lid == layer_id) {
										++state;
									}

									if (state == 2) {
										break;
									}

									auto layer = in.setup.find_layer(lid);
									ensure(layer != nullptr);

									for (const editor_node_id nid : layer->hierarchy.nodes) {
										if (nid == *last_node) {
											++state;
										}

										if (state) {
											in.setup.inspect_add_quiet(nid);
										}

										if (state == 2) {
											break;
										}
									}
								}

								in.setup.after_quietly_adding_inspected();
								in.setup.quiet_set_last_inspected_layer_or_node(*last_node);
							}
							else {
								in.setup.inspect(layer_id);
							}
						}
						else {
							in.setup.inspect(layer_id);
						}
					}
				}

				const bool selectable_double_clicked = ImGui::IsItemHovered() && has_double_click;
				bool was_arrow_clicked = false;
				bool was_arrow_hovered = false;

				{
					ImGui::SetCursorPos(before_pos);

					const auto dir = (filter.IsActive() || layer.is_open || layer.empty()) ? ImGuiDir_Down : ImGuiDir_Right;

					const bool layer_is_empty = layer.hierarchy.nodes.empty();

					{
						auto col = scoped_style_color(ImGuiCol_Button, rgba(0, 0, 0, 0));
						auto col2 = cond_scoped_style_color(layer_is_empty, ImGuiCol_Text, rgba(255, 255, 255, 0));

						if (ImGui::ArrowButtonEx("###IsOpen", dir, { max_icon_size, max_icon_size })) {
							layer.is_open = !layer.is_open;
							was_arrow_clicked = true;
						}

						was_arrow_hovered = ImGui::IsItemHovered();
					}

					ImGui::SameLine();

					bool just_set = false;

					if (rename_this_layer || (rename_requested && is_inspected)) {
						if (!is_inspected) {
							in.setup.inspect(layer_id);
						}

						rename_requested = false;
						rename_this_layer = false;

						currently_renamed_object = layer_id;
						ImGui::SetKeyboardFocusHere();
						just_set = true;
					}

					if (currently_renamed_object == inspected_variant(layer_id)) {
						auto edited_layer_name = layer.unique_name;

						if (input_text<100>("##InlineNameInput", edited_layer_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
							currently_renamed_object = std::nullopt;

							if (edited_layer_name != layer.unique_name && !edited_layer_name.empty()) {
								auto cmd = in.setup.make_command_from_selected_layers<rename_layer_command>("Renamed ");
								cmd.after = edited_layer_name;

								if (cmd.size() == 1) {
									cmd.built_description = typesafe_sprintf("Renamed layer to %x", cmd.after);
								}

								if (!cmd.empty()) {
									in.setup.post_new_command(std::move(cmd)); 
								}
							}
						}

						if (!just_set && !ImGui::IsItemActive()) {
							currently_renamed_object = std::nullopt;
						}
					}
					else {
						{
							auto disabled = maybe_disabled_cols(layer_disabled);
							text(label);
						}

						if (const bool layer_is_empty = layer.hierarchy.nodes.empty()) {
							ImGui::SameLine();
							text_disabled("(empty)");
						}
					}
				}

				if (selectable_double_clicked && !was_arrow_hovered && !was_arrow_clicked) {
					layer.is_open = !layer.is_open;
				}
			}

			ImGui::NextColumn();

			{
				const auto visibility_icon = in.necessary_images[layer.is_active()
					? assets::necessary_image_id::EDITOR_ICON_VISIBLE
					: assets::necessary_image_id::EDITOR_ICON_HIDDEN
				];

				const auto scaled_icon_size = vec2::scaled_to_max_size(visibility_icon.get_original_size(), max_icon_size);
				const auto cols = layer_disabled ? colors_nha { disabled_color, disabled_color, disabled_color } : colors_nha{};

				if (game_image_button("###Visibility", visibility_icon, scaled_icon_size, cols, augs::imgui_atlas_type::GAME)) {
					const auto next_value = !layer.is_active();

					if (in.setup.is_inspected(layer_id)) {
						auto command = in.setup.make_command_from_selected_layers<toggle_layers_active_command>(next_value ? "Enabled " : "Disabled ");
						command.next_value = next_value;
						in.setup.post_new_command(std::move(command));
					}
					else {
						auto command = toggle_layers_active_command(); 
						command.push_entry(layer_id);
						command.built_description = (next_value ? "Enabled " : "Disabled ") + layer.get_display_name();
						command.next_value = next_value;
						command.update_inspector = false;
						in.setup.post_new_command(std::move(command));
					}
				}
			}

			ImGui::NextColumn();

			if (layer.is_open || filter.IsActive()) {
				for (std::size_t i = 0; i < layer.hierarchy.nodes.size(); ++i) {
					const auto node_id = layer.hierarchy.nodes[i];

					in.setup.on_node(node_id, [&](auto& typed_node, const auto typed_node_id) {
						if (filter.IsActive()) {
							if (!typed_node.passed_filter) {
								return;
							}
						}

						handle_node_and_id(i, typed_node, typed_node_id, layer, layer_id);
					});
				}
			}
		}
	}
}
