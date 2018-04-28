#pragma once
#include <string>
#include <vector>
#include <optional>

#include "augs/math/vec2.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

class cosmos;
struct editor_go_to_settings;
struct editor_view;

class editor_go_to_entity_gui {
	bool show = false;
	std::string textbox_data;

	std::string last_input;
	unsigned selected_index = 0;
	std::vector<entity_guid> matches;

public:
	void open();

	std::optional<const_entity_handle> perform(
		const editor_go_to_settings& settings,
		const cosmos& cosm,
		vec2 dialog_pos
	);

	const_entity_handle get_matching_go_to_entity(const cosmos&) const;
};

void standard_confirm_go_to(const_entity_handle, bool has_ctrl, editor_view&);
