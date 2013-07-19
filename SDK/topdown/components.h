#pragma once
#include "../../entity_system/entity_system.h"

struct render_component : public augmentations::entity_system::component {
	unsigned layer;
};