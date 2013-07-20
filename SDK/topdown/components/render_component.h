#pragma once
#include "../../../entity_system/entity_system.h"

namespace components {
	namespace texture_baker {
		class texture;
	}

	struct render : public augmentations::entity_system::component {
		unsigned layer;
		texture_baker::texture* texture;

		render(unsigned layer) : layer(layer) {}
	};
}