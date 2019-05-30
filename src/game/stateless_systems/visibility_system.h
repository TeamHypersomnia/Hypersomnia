#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

#include "game/debug_drawing_settings.h"
#include "game/cosmos/step_declaration.h"

using visibility_requests = std::vector<messages::visibility_information_request>;
using visibility_responses = std::vector<messages::visibility_information_response>;

inline auto& thread_local_visibility_requests() {
	thread_local visibility_requests requests;
	requests.clear();
	return requests;
}


struct performance_settings;

inline auto& thread_local_visibility_responses() {
	thread_local visibility_responses responses;
	return responses;
}

class visibility_system {
	using lines_ref = std::vector<debug_line>&;

public:
	lines_ref DEBUG_LINES_TARGET;
	visibility_system(lines_ref ref) : DEBUG_LINES_TARGET(ref) {}

	messages::visibility_information_response& calc_visibility(
		const cosmos&,
		const messages::visibility_information_request&
	) const;

	void calc_visibility(
		const cosmos&,
		const visibility_requests&,
		visibility_responses&,
		const performance_settings&
	) const;

	void calc_visibility(const logic_step) const;
};