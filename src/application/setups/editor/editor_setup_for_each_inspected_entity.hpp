#pragma once

template <class F>
void editor_setup::for_each_inspected_entity(F&& callback) const {
	gui.inspector.for_each_inspected<editor_node_id>(
		[&](const editor_node_id& node_id) {
			on_node(
				node_id,
				[&](const auto& typed_node, const auto id) {
					(void)id;
					callback(typed_node.scene_entity_id);
				}
			);
		}
	);
}
