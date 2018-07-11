#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

#include "game/debug_drawing_settings.h"
#include "game/transcendental/step_declaration.h"

using visibility_requests = std::vector<messages::visibility_information_request>;
using visibility_responses = std::vector<messages::visibility_information_response>;

class visibility_system {
	using lines_ref = std::vector<debug_line>&;

public:
	lines_ref DEBUG_LINES_TARGET;
	visibility_system(lines_ref ref) : DEBUG_LINES_TARGET(ref) {}

	visibility_responses calc_visibility(const cosmos&, const visibility_requests&) const;

	void calc_visibility(
		const cosmos&,
		const visibility_requests&,
		visibility_responses&
	) const;

	void calc_visibility(const logic_step) const;
};