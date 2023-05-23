#pragma once
#include "application/setups/editor/detail/is_editor_typed_node.h"

template <class T>
struct recurse_to_find_node_ids {
	static constexpr bool value = !augs::has_custom_to_json_value_v<T>;
};

template <class O, class F>
void on_each_node_id_in_object(O& object, F callback) {
	auto handle = [&](auto& field) {
		using Field = remove_cref<decltype(field)>;

		if constexpr(is_editor_typed_node_id_v<Field>) {
			callback(field);
		}
	};

	augs::on_each_object_in_object<recurse_to_find_node_ids>(object, handle);
}

template <class P, class F>
void on_each_node_id_in_project(P& project, F callback) {
	auto traverse = [&](auto& object) {
		on_each_node_id_in_object(object, callback);
	};

	project.nodes.pools.for_each(
		[&](auto& node) {
			traverse(node.editable);
		}
	);
}

