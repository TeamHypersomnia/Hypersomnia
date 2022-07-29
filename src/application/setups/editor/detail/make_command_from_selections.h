#pragma once
#include "game/detail/describers.h"

template <class T, class F, class P>
T make_command_from_selections(
	const editor_setup& setup,
	F for_each_selected,
   	const std::string& preffix,
   	P inclusion_predicate
) {
	T command;

	std::string last_name;

	for_each_selected(
		[&](const auto& selected) {
			if (inclusion_predicate(selected)) {
				command.push_entry(selected);
				last_name = setup.get_name(selected);
			}
		}
	);

	if (command.size() == 1) {
		command.built_description = preffix + last_name;
	}
	else if (command.size() == setup.get_node_count()) {
		command.built_description = preffix + "all nodes";
	}
	else {
		command.built_description = preffix + typesafe_sprintf("%x nodes", command.size());
	}

	return command;
}

template <class T, class F>
T make_command_from_selections(
	const editor_setup& setup,
	F for_each_selected,
	const std::string& preffix
) {
	return make_command_from_selections<T>(setup, for_each_selected, preffix, [](const auto&) { return true; });
}

template <class T, class... Args>
auto editor_setup::make_command_from_selected_entities(Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			for_each_inspected_entity(
				[&](const auto id) {
					if (const auto handle = scene.world[id]) {
						callback(handle);
					}
				}
			);	
		},
		std::forward<Args>(args)...
	);
}

template <class T, class... Args>
auto editor_setup::make_command_from_selected_nodes(Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			gui.inspector.for_each_inspected<editor_node_id>(callback);
		},
		std::forward<Args>(args)...
	);
}
