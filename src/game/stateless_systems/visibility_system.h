#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

#include "game/debug_drawing_settings.h"
#include "game/transcendental/step_declaration.h"

class visibility_system {
	using lines_ref = std::vector<debug_line>&;

public:
	lines_ref DEBUG_LINES_TARGET;
	visibility_system(lines_ref ref) : DEBUG_LINES_TARGET(ref) {}

	struct visibility_responses {
		std::vector<messages::line_of_sight_response> los;
		std::vector<messages::visibility_information_response> vis;
	};
	
	visibility_responses respond_to_visibility_information_requests(
		const cosmos&,
		const std::vector<messages::line_of_sight_request>&,
		const std::vector<messages::visibility_information_request>&
	);

	void respond_to_visibility_information_requests(
		const cosmos&,
		const std::vector<messages::line_of_sight_request>&,
		const std::vector<messages::visibility_information_request>&,
		std::vector<messages::line_of_sight_response>&,
		std::vector<messages::visibility_information_response>&
	) const;

	void respond_to_visibility_information_requests(const logic_step) const;
};