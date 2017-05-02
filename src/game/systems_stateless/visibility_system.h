#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

#include "game/transcendental/step_declaration.h"

class visibility_system {
public:
	struct visibility_responses {
		std::vector<messages::line_of_sight_response> los;
		std::vector<messages::visibility_information_response> vis;
	} respond_to_visibility_information_requests(
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