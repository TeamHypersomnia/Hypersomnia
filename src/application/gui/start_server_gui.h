#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"

struct server_vars;
struct server_solvable_vars;
class nat_detection_session;

class start_server_gui_state : public standard_window_mixin<start_server_gui_state> {
public:
	using base = standard_window_mixin<start_server_gui_state>;
	using base::base;

	server_instance_type instance_type = server_instance_type::INTEGRATED;

	bool show_help = false;
	bool show_nat_details = false;

	bool perform(
		server_start_input& into,
		server_vars&,
		server_solvable_vars&,

		const nat_detection_session*
	);
};
