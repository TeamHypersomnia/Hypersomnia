#pragma once
#include "augs/global_libraries.h"
#include "setups.h"

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest(argc, argv);

	local_setup();

	augs::global_libraries::deinit();
	return 0;
}