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
	in.setup.clear_inspector();
	layers_backup = in.setup.project.layers.pool;

	deleted_nodes.for_each([&]<typename T>(T& entry) {
		using N = decltype(T::node_content);

		{
			const auto node_id = editor_node_id(entry.node_id);
			const auto found_layer = in.setup.find_parent_layer(node_id);

			ensure(found_layer != std::nullopt);

			const auto layer = in.setup.find_layer(found_layer->layer_id);

			ensure(layer != nullptr);

			auto& nodes = layer->hierarchy.nodes;
			nodes.erase(nodes.begin() + found_layer->index_in_layer);
		}

		auto& pool = in.setup.project.nodes.get_pool_for<N>();
		entry.node_content = pool.at(entry.node_id.raw);
		entry.undo_delete_input = *pool.free(entry.node_id.raw);
	});
}

void delete_nodes_command::undo(const editor_command_input in) {
	in.setup.clear_inspector();

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
		in.setup.inspect_add(undone_generic_id, false);
	});

	in.setup.project.layers.pool = layers_backup;
	in.setup.inspected_to_entity_selector_state();
	in.setup.sort_inspected();
}
