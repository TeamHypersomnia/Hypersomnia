#pragma once
#include <vector>
#include "game/messages/visibility_information.h"

#include "game/debug_drawing_settings.h"
#include "game/cosmos/step_declaration.h"

using visibility_request = messages::visibility_information_request;
using visibility_response = messages::visibility_information_response;

inline auto& thread_local_visibility_response() {
	thread_local visibility_response response;
	return response;
}

class visibility_system {
public:
	void calc_visibility(
		const cosmos&,
		const visibility_request&,
		visibility_response&
	) const;
};