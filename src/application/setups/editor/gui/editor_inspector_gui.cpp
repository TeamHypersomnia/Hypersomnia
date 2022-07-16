#include "augs/templates/history.hpp"
#include "application/setups/editor/editor_setup.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/string/string_templates.h"

#include "application/setups/editor/resources/editor_sound_resource.h"
#include "application/setups/editor/resources/editor_sprite_resource.h"
#include "application/setups/editor/resources/editor_light_resource.h"
#include "application/setups/editor/resources/editor_prefab_resource.h"

#include "application/setups/editor/nodes/editor_sound_node.h"
#include "application/setups/editor/nodes/editor_sprite_node.h"
#include "application/setups/editor/nodes/editor_light_node.h"
#include "application/setups/editor/nodes/editor_prefab_node.h"

#include "application/setups/editor/gui/editor_inspector_gui.h"

void editor_tweaked_widget_tracker::reset() {
	last_tweaked.reset();
}

void editor_tweaked_widget_tracker::poll_change(const std::size_t current_command_index) {
	if (last_tweaked) {
		const auto possible_session = tweak_session { ImGui::GetActiveID(), current_command_index };

		if (last_tweaked.value() != possible_session) {
			last_tweaked.reset();
		}
	}
}

bool editor_tweaked_widget_tracker::changed(const std::size_t current_command_index) const {
	const auto current_session = tweak_session { ImGui::GetActiveID(), current_command_index };
	return last_tweaked != current_session;
}

void editor_tweaked_widget_tracker::update(const std::size_t current_command_index) {
	const auto current_session = tweak_session { ImGui::GetActiveID(), current_command_index };
	last_tweaked = current_session;
}

std::string get_hex_representation(const unsigned char*, size_t length);

std::string as_hex(const rgba& col) {
	return to_uppercase(get_hex_representation(std::addressof(col.r), 4));
}

template <class T>
void edit_property(
	std::string& result,
	const std::string& label,
	T& property
) {
	using namespace augs::imgui;

	if constexpr(std::is_same_v<T, rgba>) {
		if (label.size() > 0 && label[0] == '#') {
			if (color_picker(label, property)) {
				result = typesafe_sprintf("Set Color to %x in %x", as_hex(property));
			}
		}
		else {
			if (color_edit(label, property)) {
				result = typesafe_sprintf("Set %x to %x in %x", label, as_hex(property));
			}
		}
	}
	else if constexpr(std::is_same_v<T, bool>) {
		if (checkbox(label, property)) {
			if (property) {
				result = typesafe_sprintf("Enable %x in %x", label);
			}
			else {
				result = typesafe_sprintf("Disable %x in %x", label);
			}
		}
	}
	else if constexpr(std::is_arithmetic_v<T>) {
		if (drag(label, property)) { 
			result = typesafe_sprintf("Set %x to %x in %x", label, property);
		}
	}
	else if constexpr(is_one_of_v<T, vec2, vec2i>) {
		if (drag_vec2(label, property)) { 
			result = typesafe_sprintf("Set %x to %x in %x", label, property);
		}
	}
}

std::string perform_editable_gui(editor_sprite_node_editable& e) {
	using namespace augs::imgui;

	std::string result;

	edit_property(result, "Flip horizontally", e.flip_horizontally);
	edit_property(result, "Flip vertically", e.flip_vertically);

	return result;
}

std::string perform_editable_gui(editor_sound_node_editable&) {
	using namespace augs::imgui;
	std::string result;

	return result;
}

std::string perform_editable_gui(editor_sprite_resource_editable& e) {
	using namespace augs::imgui;

	std::string result;

	edit_property(result, "##Defaultcolor", e.color);
	edit_property(result, "Size", e.size);

	ImGui::Separator();

	return result;
}

std::string perform_editable_gui(editor_sound_resource_editable&) {
	using namespace augs::imgui;
	std::string result;

	return result;
}

void editor_inspector_gui::inspect(const inspected_variant inspected, bool wants_multiple) {
	auto different_found = [&]<typename T>(const T&) {
		return inspects_any_different_than<T>();
	};

	const bool different_type_found = std::visit(different_found, inspected);

	if (wants_multiple) {
		if (different_type_found) {
			return;
		}

		if (found_in(all_inspected, inspected)) {
			erase_element(all_inspected, inspected);
		}
		else {
			all_inspected.push_back(inspected);
		}
	}
	else {
		const bool not_the_same = all_inspected != decltype(all_inspected) { inspected };

		if (not_the_same) {
			all_inspected = { inspected };

			/*
				Commands will invoke "inspect" on undo/redo,
				even when executing them for the first time - directly by user interaction.

				This is why we need to prevent the tweaked widget from being reset if the inspected object does not change,
				because otherwise continuous commands wouldn't work.
			*/
			tweaked_widget.reset();
		}
	}
}

void editor_inspector_gui::perform(const editor_inspector_input in) {
	using namespace augs::imgui;

	(void)in;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto get_object_type_name = [&]<typename T>(const T&) {
		if constexpr(std::is_same_v<T, editor_node_id>) {
			return "nodes";
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			return "resources";
		}
		else if constexpr(std::is_same_v<T, editor_layer_id>) {
			return "layers";
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive handler");
		}

		return "objects";
	};

	auto get_command_index = [&]() {
		return in.setup.get_last_command_index();
	};

	auto is_command_discrete = [&](const auto& description) {
		return	
			begins_with(description, "Disable")
			|| begins_with(description, "Enable")
		;
	};

	auto post_new_or_rewrite = [&]<typename T>(T&& cmd) {
		if (is_command_discrete(cmd.describe())) {
			in.setup.post_new_command(std::forward<T>(cmd));
			tweaked_widget.reset();
			return;
		}

		if (tweaked_widget.changed(get_command_index())) {
			in.setup.post_new_command(std::forward<T>(cmd));
			tweaked_widget.update(get_command_index());
		}
		else {
			in.setup.rewrite_last_command(std::forward<T>(cmd));
		}
	};

	auto resource_handler = [&]<typename R>(R& resource, const auto& resource_id) {
		if constexpr(
			std::is_same_v<R, editor_sprite_resource> 
			|| std::is_same_v<R, editor_sound_resource>
		) {
			const auto name = resource.external_file.path_in_project.filename().string();

			if constexpr(std::is_same_v<R, editor_sprite_resource>) {
				text_color("Sprite: ", yellow);
			}
			if constexpr(std::is_same_v<R, editor_sound_resource>) {
				text_color("Sound: ", yellow);
			}

			ImGui::SameLine();
			text(name);

			//game_image(icon, scaled_icon_size, white, vec2::zero, atlas_type);

			ImGui::Separator();

			auto id = scoped_id(resource.external_file.path_in_project.string());

			auto edited_copy = resource.editable;
			const auto changed = perform_editable_gui(edited_copy);

			if (!changed.empty()) {
				edit_resource_command<R> cmd;
				cmd.resource_id = resource_id;
				cmd.after = edited_copy;
				cmd.built_description = typesafe_sprintf(changed, resource.get_display_name());

				post_new_or_rewrite(std::move(cmd)); 
			}
		}

		(void)resource;
	};

	auto node_handler = [&]<typename N>(N& node, const auto& node_id) {
		if constexpr(
			std::is_same_v<N, editor_sprite_node> 
			|| std::is_same_v<N, editor_sound_node>
		) {
			auto id = scoped_id(node.get_display_name());

			auto edited_copy = node.editable;
			const auto changed = perform_editable_gui(edited_copy);

			if (!changed.empty()) {
				edit_node_command<N> cmd;
				cmd.node_id = node_id;
				cmd.after = edited_copy;
				cmd.built_description = typesafe_sprintf(changed, node.get_display_name());

				post_new_or_rewrite(std::move(cmd)); 
			}
		}

		(void)node;
	};

	auto edit_properties = [&]<typename T>(const T& inspected_id) {
		if constexpr(std::is_same_v<T, editor_node_id>) {
			in.setup.on_node(inspected_id, node_handler);
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			in.setup.on_resource(inspected_id, resource_handler);
		}
		else if constexpr(std::is_same_v<T, editor_layer_id>) {

		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive handler");
		}

		(void)inspected_id;
	};

	if (all_inspected.size() == 0) {

	}
	else if (all_inspected.size() == 1) {
		std::visit(edit_properties, all_inspected[0]);
	}
	else {
		text_color(typesafe_sprintf("Selected %x %x:", all_inspected.size(), std::visit(get_object_type_name, all_inspected[0])), yellow);

		ImGui::Separator();

		for (const auto& a : all_inspected) {
			const auto name = in.setup.get_name(a);

			if (name.size() > 0) {
				text(name);
			}
		}
	}
}
