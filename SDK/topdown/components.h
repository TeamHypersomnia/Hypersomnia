#pragma once
#include "../../entity_system/entity_system.h"
#include "../../texture_baker/texture_baker.h"


namespace components {
	using namespace augmentations;
	using namespace entity_system;

	struct render : public component {
		unsigned layer;
		texture_baker::texture texture;
	};

	struct transform : public component {


	};
}