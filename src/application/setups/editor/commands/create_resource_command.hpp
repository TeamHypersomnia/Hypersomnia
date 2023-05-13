#pragma once
#include "application/setups/editor/commands/create_resource_command.h"
#include "augs/misc/pool/pool_allocate.h"

template <class T>
inline void create_resource_command<T>::undo(editor_command_input in) {
	{
		auto& resource_pool = in.setup.project.resources.template get_pool_for<T>();
		base::undo(resource_pool);
	}

	in.setup.rebuild_project_internal_resources_gui();

	if (!omit_inspector) {
		in.setup.clear_inspector();
	}
}

template <class T>
inline void create_resource_command<T>::redo(editor_command_input in) {
	{
		created_resource.unique_name = in.setup.get_free_internal_resource_name_for(created_resource.unique_name);
		built_description = "Created " + created_resource.unique_name;

		auto& resource_pool = in.setup.project.resources.template get_pool_for<T>();
		base::redo(resource_pool, created_resource);
	}

	const auto resource_id = get_resource_id();

	in.setup.rebuild_project_internal_resources_gui();

	if (!omit_inspector) {
		in.setup.inspect_only(resource_id);
		in.setup.scroll_once_to(resource_id);
	}
}

template <class T>
inline editor_typed_resource_id<T> create_resource_command<T>::get_typed_resource_id() const {
	const bool is_official = false;
	return editor_typed_resource_id<T>::from_raw(base::get_allocated_id(), is_official);
}

template <class T>
inline editor_resource_id create_resource_command<T>::get_resource_id() const {
	return get_typed_resource_id().operator editor_resource_id();
}
