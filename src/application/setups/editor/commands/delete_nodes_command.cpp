#include "application/setups/editor/commands/delete_nodes_command.h"
#include "application/setups/editor/editor_setup.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/ensure.h"

std::string delete_nodes_command::describe() const {
	return built_description;
}

void delete_nodes_command::push_entry(const editor_node_id id) {
	id.type_id.dispatch([&]<typename E>(const E) {
		deleted_nodes.get_for<E>().push_back({ {}, editor_typed_node_id<E>::from_generic(id), {} });
	});
}

bool delete_nodes_command::empty() const {
	return size() == 0;
}

void delete_nodes_command::redo(const editor_command_input in) {
	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	layers_backup = in.setup.project.layers.pool;

	deleted_nodes.for_each([&]<typename T>(T& entry) {
		using N = decltype(T::node_content);

		{
			const auto node_id = editor_node_id(entry.node_id);
			const auto found_layer = in.setup.find_parent_layer(node_id);

			ensure(found_layer.has_value());

			const auto layer = in.setup.find_layer(found_layer->layer_id);

			ensure(layer != nullptr);

			auto& nodes = layer->hierarchy.nodes;
			nodes.erase(nodes.begin() + found_layer->index_in_layer);
		}

		auto& pool = in.setup.project.nodes.get_pool_for<N>();

		const auto found_node = pool.find(entry.node_id.raw);

		ensure(found_node != nullptr);

		entry.node_content = *found_node;

		auto delete_input = pool.free(entry.node_id.raw);
		ensure(delete_input.has_value());
		entry.undo_delete_input = *delete_input;
	});
}

void delete_nodes_command::undo(const editor_command_input in) {
	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	/* 
		NOTE: Pools should be independent, but to be theoretically pure,
		we should implement reverse_for_each_through_std_get inside for_each_reverse.
	*/

	deleted_nodes.for_each_reverse([&]<typename T>(T& entry) {
		using N = decltype(T::node_content);

		auto& pool = in.setup.project.nodes.get_pool_for<N>();

		const auto [undone_id, object] = pool.undo_free(entry.undo_delete_input, std::move(entry.node_content));
		ensure(undone_id == entry.node_id.raw);

		const auto undone_generic_id = entry.node_id.operator editor_node_id();

		if (!omit_inspector) {
			in.setup.inspect_add_quiet(undone_generic_id);
		}
	});

	in.setup.project.layers.pool = layers_backup;

	if (!omit_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

#include "application/setups/editor/commands/delete_resources_command.h"

std::string delete_resources_command::describe() const {
	return built_description;
}

void delete_resources_command::push_entry(const editor_resource_id id) {
	id.type_id.constrained_dispatch<internal_editor_resource_types>([&]<typename E>(const E&) {
		deleted_resources.get_for<E>().push_back({ {}, editor_typed_resource_id<E>::from_generic(id), {} });
	});
}

bool delete_resources_command::empty() const {
	return size() == 0;
}

void delete_resources_command::redo(const editor_command_input in) {
	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	deleted_resources.for_each([&]<typename T>(T& entry) {
		using N = decltype(T::resource_content);

		auto& pool = in.setup.project.resources.get_pool_for<N>();

		const auto found_resource = pool.find(entry.resource_id.raw);

		ensure(found_resource != nullptr);

		entry.resource_content = *found_resource;

		auto delete_input = pool.free(entry.resource_id.raw);
		ensure(delete_input.has_value());
		entry.undo_delete_input = *delete_input;
	});

	in.setup.rebuild_project_internal_resources_gui();
}

void delete_resources_command::undo(const editor_command_input in) {
	if (!omit_inspector) {
		in.setup.clear_inspector();
	}

	/* 
		NOTE: Pools should be independent, but to be theoretically pure,
		we should implement reverse_for_each_through_std_get inside for_each_reverse.
	*/

	bool scroll_once = true;

	deleted_resources.for_each_reverse([&]<typename T>(T& entry) {
		using N = decltype(T::resource_content);

		auto& pool = in.setup.project.resources.get_pool_for<N>();

		const auto [undone_id, object] = pool.undo_free(entry.undo_delete_input, std::move(entry.resource_content));
		ensure(undone_id == entry.resource_id.raw);

		const auto undone_generic_id = entry.resource_id.operator editor_resource_id();

		if (!omit_inspector) {
			in.setup.inspect_add_quiet(undone_generic_id);

			if (scroll_once) {
				in.setup.scroll_once_to(undone_generic_id);
				scroll_once = false;
			}
		}
	});

	in.setup.rebuild_project_internal_resources_gui();

	if (!omit_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}
