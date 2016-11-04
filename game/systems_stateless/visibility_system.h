#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

class logic_step;

class visibility_system {
public:
	void respond_to_visibility_information_requests(
		const cosmos&,
		const std::vector<messages::line_of_sight_request>&,
		const std::vector<messages::visibility_information_request>&,
		std::vector<messages::line_of_sight_response>&,
		std::vector<messages::visibility_information_response>&
	) const;

	void respond_to_visibility_information_requests(logic_step&) const;
};