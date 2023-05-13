#pragma once
#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/commands/edit_resource_command.h"
#include "application/setups/editor/commands/editor_command_meta.h"
#include "application/setups/editor/resources/resource_traits.h"

template <class T>
void edit_resource_command<T>::push_entry(const editor_typed_resource_id<T> id) {
	entries.push_back({ id, {}, {} });
}

template <class T>
void edit_resource_command<T>::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (entry.resource_id.is_official) {
			built_description = "Error: trying to edit official resource.";
			continue;
		}

		if (auto resource = in.setup.find_resource(entry.resource_id)) {
			resource->editable = entry.before;

			if (do_inspector && override_inspector_state == std::nullopt) {
				in.setup.inspect_add_quiet(editor_resource_id(entry.resource_id));
			}
		}
	}

	if (do_inspector) {
		if (override_inspector_state.has_value()) {
			in.setup.inspect_only(*override_inspector_state);
			in.setup.set_inspector_tab(inspected_node_tab_type::RESOURCE);
		}
		else {
			in.setup.after_quietly_adding_inspected();
		}
	}
}

template <class T>
void edit_resource_command<T>::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		if (entry.resource_id.is_official) {
			built_description = "Error: trying to edit official resource.";
			continue;
		}

		if (auto resource = in.setup.find_resource(entry.resource_id)) {
			entry.before = resource->editable;
			resource->editable = entry.after;

			if (do_inspector && override_inspector_state == std::nullopt) {
				in.setup.inspect_only(editor_resource_id(entry.resource_id));
			}
		}
	}

	if (do_inspector) {
		if (override_inspector_state.has_value()) {
			in.setup.inspect_only(*override_inspector_state);
			in.setup.set_inspector_tab(inspected_node_tab_type::RESOURCE);
		}
		else {
			in.setup.after_quietly_adding_inspected();
		}
	}
}

void rename_resource_command::push_entry(const editor_resource_id id) {
	entries.push_back({ id, {} });
}

void rename_resource_command::undo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		in.setup.on_resource(
			entry.resource_id,
			[&]<typename R>(R& resource, const auto id) {
				(void)id;

				if constexpr(is_internal_resource_v<R>) {
					resource.unique_name = entry.before;

					if (do_inspector) {
						in.setup.inspect_add_quiet(entry.resource_id);
					}
				}
			}
		);
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}

	in.setup.rebuild_project_internal_resources_gui();
}

void rename_resource_command::redo(editor_command_input in) {
	const bool do_inspector = !in.skip_inspector;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	for (auto& entry : entries) {
		in.setup.on_resource(
			entry.resource_id,
			[&]<typename R>(R& resource, const auto id) {
				(void)id;

				if constexpr(is_internal_resource_v<R>) {
					entry.before = resource.unique_name;
					resource.unique_name = "";
					resource.unique_name = in.setup.get_free_internal_resource_name_for(after);

					if (do_inspector) {
						in.setup.inspect_add_quiet(entry.resource_id);
					}
				}
			}
		);
	}

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}

	in.setup.rebuild_project_internal_resources_gui();
}
