#pragma once
#include <variant>
#include "application/setups/main_menu_setup.h"
#include "application/setups/test_scene_setup.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/client/client_setup.h"
#include "application/setups/server/server_setup.h"
#include "application/setups/builder/builder_setup.h"

using setup_variant = std::variant<
	test_scene_setup,
	editor_setup,
	builder_setup
#if BUILD_NETWORKING
	, client_setup
	, server_setup
#endif
>;
