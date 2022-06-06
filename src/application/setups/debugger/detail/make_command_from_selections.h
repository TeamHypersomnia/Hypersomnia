#pragma once
#include "game/detail/describers.h"

template <class T, class F, class C, class P>
T make_command_from_selections(
	F for_each_selected,
	C& cosm,
   	const std::string& preffix,
   	P inclusion_predicate
) {
	thread_local name_accumulator counts;
	counts.clear();

	T command;

	auto& _counts = counts;

	for_each_selected(
		[&](const auto e) {
			const auto handle = cosm[e];

			if (inclusion_predicate(handle)) {
				command.push_entry(handle);
				++_counts[handle.get_name()];
			}
		}
	);

	if (command.size() == cosm.get_entities_count()) {
		command.built_description = preffix + "all entities";
	}
	else {
		command.built_description = preffix + ::describe_names_of(counts);
	}

	return command;
}

template <class T, class C, class F>
T make_command_from_selections(
	F for_each_selected,
	C& cosm,
	const std::string& preffix
) {
	return make_command_from_selections<T>(for_each_selected, cosm, preffix, [](const auto&) { return true; });
}
