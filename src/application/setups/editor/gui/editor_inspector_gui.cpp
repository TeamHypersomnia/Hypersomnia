#include "augs/templates/history.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/commands/edit_resource_command.hpp"
#include "application/setups/editor/commands/edit_node_command.hpp"

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

bool perform_editable_gui(editor_sprite_node_editable& e) {
	using namespace augs::imgui;
	bool changed = false;

	auto detect_change = [&](auto expression_result) {
		if (expression_result) {
			changed = true;
		}
	};

	detect_change(checkbox("Flip horizontally", e.flip_horizontally));
	detect_change(checkbox("Flip vertically", e.flip_vertically));

	return changed;
}

bool perform_editable_gui(editor_sound_node_editable&) {
	using namespace augs::imgui;
	bool changed = false;

	auto detect_change = [&](auto expression_result) {
		if (expression_result) {
			changed = true;
		}
	};

	(void)detect_change;
	return changed;
}

bool perform_editable_gui(editor_sprite_resource_editable& e) {
	using namespace augs::imgui;

	bool changed = false;

	auto detect_change = [&](auto expression_result) {
		if (expression_result) {
			changed = true;
		}
	};

	text("Default sprite color");

	detect_change(color_picker("##Defaultcolor", e.color));

	return changed;
}

bool perform_editable_gui(editor_sound_resource_editable&) {
	using namespace augs::imgui;
	bool changed = false;

	auto detect_change = [&](auto expression_result) {
		if (expression_result) {
			changed = true;
		}
	};

	(void)detect_change;
	return changed;
}

void editor_inspector_gui::inspect(inspected_variant inspected) {
	currently_inspected = std::move(inspected);
	tweaked_widget.reset();
}

void editor_inspector_gui::perform(const editor_inspector_input in) {
	using namespace augs::imgui;

	(void)in;
	
	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto get_command_index = [&]() {
		return in.setup.get_last_command_index();
	};

	auto post_new_or_rewrite = [&]<typename T>(T&& cmd) {
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
			auto id = scoped_id(resource.external_file.path_in_project.string());

			auto edited_copy = resource.editable;
			const bool changed = perform_editable_gui(edited_copy);

			if (changed) {
				edit_resource_command<R> cmd;
				cmd.resource_id = resource_id;
				cmd.after = edited_copy;
				cmd.built_description = "Edit " + resource.get_display_name();

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
			const bool changed = perform_editable_gui(edited_copy);

			if (changed) {
				edit_node_command<N> cmd;
				cmd.node_id = node_id;
				cmd.after = edited_copy;
				cmd.built_description = "Edit " + node.get_display_name();

				post_new_or_rewrite(std::move(cmd)); 
			}
		}

		(void)node;
	};

	auto handler = [&]<typename T>(const T& inspected_id) {
		if constexpr(std::is_same_v<T, editor_node_id>) {
			in.setup.on_node(inspected_id, node_handler);
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			in.setup.on_resource(inspected_id, resource_handler);
		}
		else {
			static_assert("Non-exhaustive handler");
		}

		(void)inspected_id;
	};

	std::visit(handler, currently_inspected);
}
