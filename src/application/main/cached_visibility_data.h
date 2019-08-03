#pragma once
#include "game/stateless_systems/visibility_system.h"

struct cached_visibility_data {
	visibility_response fow_response;
	std::vector<visibility_response> light_responses;
	std::vector<visibility_request> light_requests;
};
