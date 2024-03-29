#pragma once
#include <string>
#include <vector>
#include <optional>

#include "augs/math/vec2.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

class cosmos;
struct debugger_go_to_settings;
struct debugger_view;
struct debugger_view_ids;

class debugger_go_to_entity_gui {
	bool show = false;
	std::string textbox_data;

	std::string last_input;
	unsigned selected_index = 0;
	std::vector<entity_id> matches;
	bool moved_since_opening = false;

public:
	void open();

	std::optional<const_entity_handle> perform(
		const debugger_go_to_settings& settings,
		const cosmos& cosm,
		vec2 dialog_pos
	);

	const_entity_handle get_matching_go_to_entity(const cosmos&) const;
};

void standard_confirm_go_to(
	const_entity_handle, 
	bool has_ctrl, 
	debugger_view&,
	debugger_view_ids&
);
