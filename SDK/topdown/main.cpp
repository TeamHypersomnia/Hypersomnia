#pragma once
#include "../../augmentations.h"

#include "../../window_framework/window.h"
#include "../../config/config.h"
#include "components.h"

using namespace augmentations;
using namespace entity_system;

int main() {
	augmentations::init();



	augmentations::deinit();
	return 0;
}
