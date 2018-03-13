#pragma once

namespace augs {
	template <class...>
	class history_with_marks;
};

struct delete_entities_command;
struct change_invariant_property_command;
struct fill_with_test_scene_command;

using editor_history = augs::history_with_marks<
	delete_entities_command,
	fill_with_test_scene_command
>;
