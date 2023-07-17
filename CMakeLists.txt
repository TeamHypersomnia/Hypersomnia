# Note: Release builds are treated as production builds
# and will take a lot longer to build due to link-time optimizations.
#
# Unless you are indeed releasing a binary, use Debug or RelWithDebInfo.

cmake_minimum_required(VERSION 3.2)

project(Hypersomnia)
enable_language(C ASM)

# Sanity logs.
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("CMAKE_LINKER: ${CMAKE_LINKER}")

message("Hypersomnia project source dir: ${PROJECT_SOURCE_DIR}")
message("Hypersomnia project binary dir: ${PROJECT_BINARY_DIR}")
message("Hypersomnia current binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

# Determine the used compiler.
# In case of MSVC, CMake sets the MSVC variable to ON on its own.

# Important note:
# MSVC build is currently hardcoded to be statically linked.

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message("Building with Clang.")
	set(CLANG ON)

	set(FUSE_LD_FLAG "-fuse-ld=lld")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	message("Building with AppleClang.")
	set(CLANG ON)
	set(USE_GLFW ON)
	include_directories("/usr/local/include")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("Building with gcc.")
	set(GCC ON)
else()
	message("Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

if(CLANG)
	EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
	string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

	message("Clang version: ${CLANG_VERSION_STRING}")
endif()

if(MSVC)
	if(CLANG)
		message("Building with Clang on Windows.")
	else()
		set(MSVC_SPECIFIC ON)
		message("Building with MSVC specifically.")
	endif()
endif()

if(UNIX)
	message("Building for UNIX systems.")

	if (NOT APPLE)
		set(LINUX ON)
	endif()

	if(NOT ARCHITECTURE)
		message(FATAL_ERROR "Architecture type was not specified.")
	endif()

	if(${ARCHITECTURE} STREQUAL "x86")
		message("Building for x86 architecture.")
	elseif(${ARCHITECTURE} STREQUAL "x64")
		message("Building for x64 architecture.")
	else()
		message(FATAL_ERROR "Unknown architecture: ${ARCHITECTURE}")
	endif()

elseif(MSVC)
	message("Building for Windows systems.")
	# Unlike makefiles, architecture is determined by the generator used.
else()
	message(FATAL_ERROR "Unknown system.")
endif()

if(CLANG)
	set(XCLANG_STR "-Xclang")
else()
	set(XCLANG_STR "")
endif()

##################################################
### ALL CUSTOMIZABLE BUILD SWITCHES BEGIN HERE ###

# If it is your first time building Hypersomnia and you get some errors,
# you might want to try turning HYPERSOMNIA_BARE on to build the minimal possible working Hypersomnia runtime.
# Then, try to work from there.
# The game should still run, even not having these features, although obviously, you won't see or hear anything.

if (HYPERSOMNIA_BARE)
	message("Building Hypersomnia without... anything")
	
	set(DEFAULT_OPT OFF)
	set(DEFAULT_NET_OPT OFF)
else()
	set(DEFAULT_OPT ON)
	set(DEFAULT_NET_OPT ON)
endif()

if (HYPERSOMNIA_DEDICATED_SERVER)
	message("Building a minimal executable required to run a dedicated server.")

	set(DEFAULT_OPT OFF)
	set(DEFAULT_NET_OPT ON)
endif()

# In the future, the switches will also differ depending on whether the target application is a client or a server.
# If you're a developer, you might also want to disable some functionality to have faster builds for quicker iteration.

## These variables are used by the archiver to know which files are to be included with the executable,
## and also for Visual Studio to properly set the working directory.

set(HYPERSOMNIA_RESOURCES_FOLDER_NAME "hypersomnia")
set(HYPERSOMNIA_EXE_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/${HYPERSOMNIA_RESOURCES_FOLDER_NAME}")

## Application packaging switches.
option(ADD_APPLICATION_ICON "Add application icon." ${DEFAULT_OPT})

## Hardcoded, relative paths for directories that the executable might use.

# From where will the game pull its official resources.

set(HYPERSOMNIA_OFFICIAL_CONTENT_DIR "content")

# Some essentials lua scripts, ssh-keygen, and internal data files needed by the executable.
set(HYPERSOMNIA_DETAIL_DIR "detail")

# Where the game will save logs.
# Deleting this folder will have no effect on the game.

set(HYPERSOMNIA_LOG_FILES_DIR "logs")

# Where the game will save neon maps and other regenerable content.
# Deleting this folder will have no effect on the game, except that some resources might take time to generate again.

set(HYPERSOMNIA_GENERATED_FILES_DIR "cache")

# Where the game will save user arenas, all downloaded arenas, gui layouts, user config etc.
# Deleting this folder might possibly result in data loss, e.g. the untitled works and some user settings.

set(HYPERSOMNIA_USER_FILES_DIR "user")

## Building switches for functionality using 3rdparty libraries.

option(BUILD_OPENAL "Build OpenAL Soft." ${DEFAULT_OPT})
option(BUILD_OPENGL "Build OpenGL-related code." ${DEFAULT_OPT})
option(BUILD_NETWORKING "Build the networking backend." ${DEFAULT_NET_OPT})
option(BUILD_OPENSSL "Build SSL support. Required for secure automatic application updates." ${DEFAULT_NET_OPT})
option(BUILD_FREETYPE "Build FreeType library responsible for loading fonts." ${DEFAULT_OPT})
option(BUILD_WINDOW_FRAMEWORK "Build code specific to a given platfom, e.g. window management done by WinAPI." ${DEFAULT_OPT})
option(BUILD_UNIT_TESTS "Build unit tests that are run on game startup." ${DEFAULT_NET_OPT})
option(BUILD_SOUND_FORMAT_DECODERS "Build ogg and vorbis. If this is off, there is also no point in enabling OpenAL." ${DEFAULT_OPT})

## Switches for functionality originating from the Hypersomnia codebase.

option(BUILD_VERSION_FILE_GENERATOR "Build version file generator to generate commit information." ${DEFAULT_NET_OPT})
option(BUILD_TEST_SCENES "Build unscripted test scenes hardcoded in C++." ${DEFAULT_NET_OPT})
option(BUILD_STENCIL_BUFFER "Build stencil buffer related code. Will be disabled in MMO setups, so everybody is equal in having wallhacks." ${DEFAULT_OPT})

option(STATIC_LINK_STDLIB "Statically link the C++ standard library." OFF)
option(PREFER_LIBCXX "Use llvm's implementation of the C++ standard library." ON)

set(BUILD_IN_CONSOLE_MODE OFF CACHE STRING "0")

## Build optimization switches.
## These may speed up build times.

option(USE_O3 "Use -O3 switch in Release." OFF)
option(LINK_TIME_OPTIMIZATION "Perform link-time optimization. Can drastically increase the build time." OFF)

## Debug information.

option(GENERATE_DEBUG_INFORMATION "Generate source-level debug information. If 0, -g flag will be removed to speed up the build." ${DEFAULT_OPT})
option(BUILD_DEBUGGER_SETUP "Build debugger setup. Due to hardcore templates, this takes amazingly long time to build, so it makes sense to turn it off sometimes." OFF)

## Thread sanitizer.

option(ENABLE_THREADSANITIZER "Enable ThreadSanitizer." OFF)
option(ENABLE_ADRRESSSANITIZER "Enable AddressSanitizer." OFF)

if (ENABLE_THREADSANITIZER)
	message("Game will build with the thread sanitizer.")
	set(GENERATE_DEBUG_INFORMATION ON)
	message("Forcing GENERATE_DEBUG_INFORMATION to ON.")
endif()

if (GENERATE_DEBUG_INFORMATION)
	message("Generating debug information. Compilation will take a while.")
else()
	message("Not generating debug information.")
endif()

if (BUILD_DEBUGGER_SETUP)
	message("Building with debugger setup. Compilation will take a while.")
else()
	message("Building without debugger setup.")
endif()

## Static allocation switches. Possible values are one or zero.

# If this variable is nonzero, the cosmos will use a statically allocated number
# of entities, drastically improving access performance. Might be very useful for a sever application.
# Pros: 
# 	+ Dereferencing entities with speed of light
# Cons:
#	- Compilation might be longer due to component includes needed everywhere
#	- Can't tweak maximum number of entities at runtime
#	- Even empty game worlds occupy as much space as those filled to the maximum

set(STATICALLY_ALLOCATE_ENTITIES OFF CACHE STRING "Statically allocate entities in the cosmos")

# If this variable is nonzero, the cosmos common will use a statically allocated number
# of entity flavours, drastically improving access performance. Might be very useful for a sever application.
# Pros: 
# 	+ Operating on entities with speed of light
# Cons:
#	- Compilation might be longer due to component includes needed everywhere
#	- Can't tweak maximum number of entity flavours at runtime
#	- Even empty game worlds occupy as much space as those filled to the maximum

set(STATICALLY_ALLOCATE_ENTITY_FLAVOURS OFF CACHE STRING "Statically allocate entity flavours in the cosmos common")

## Internal build flags for programmers' use. 
## Don't set them manually without a good reason.

set (BUILD_LUA_AS_CPP OFF)

if (UNIX)
	set(OpenGL_GL_PREFERENCE "LEGACY")
endif()

### ALL CUSTOMIZABLE BUILD SWITCHES END HERE ###
################################################

# Here we automatically adjust some of the switches,
# based on what was set by the user.

if (UNIX)
	# Honestly I don't know why we need that,
	# there is some weird CMAKE_AR-NOTFOUND error without this line.
	set(CMAKE_AR "ar")
endif()

set(BUILD_GLAD ${BUILD_OPENGL})

if (GCC)
	set(ADD_MCMODEL_LARGE_FLAG ON CACHE STRING "Needed to suppress some crazy linker errors")
else()
	set(ADD_MCMODEL_LARGE_FLAG OFF CACHE STRING "Needed to suppress some crazy linker errors")
endif()

if (NOT BUILD_WINDOW_FRAMEWORK)
	set(BUILD_IN_CONSOLE_MODE "1")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#####################################################
### SPECIFICATION OF ALL SOURCE FILES BEGINS HERE ###

# We will be building Lua manually as there is no up-to-date CMakeLists for Lua.

set(LUA_CPPS
	"src/3rdparty/lua/lapi.c"
	"src/3rdparty/lua/lauxlib.c"
	"src/3rdparty/lua/lbaselib.c"
	"src/3rdparty/lua/lbitlib.c"
	"src/3rdparty/lua/lcode.c"
	"src/3rdparty/lua/lcorolib.c"
	"src/3rdparty/lua/lctype.c"
	"src/3rdparty/lua/ldblib.c"
	"src/3rdparty/lua/ldebug.c"
	"src/3rdparty/lua/ldo.c"
	"src/3rdparty/lua/ldump.c"
	"src/3rdparty/lua/lfunc.c"
	"src/3rdparty/lua/lgc.c"
	"src/3rdparty/lua/linit.c"
	"src/3rdparty/lua/liolib.c"
	"src/3rdparty/lua/llex.c"
	"src/3rdparty/lua/lmathlib.c"
	"src/3rdparty/lua/lmem.c"
	"src/3rdparty/lua/loadlib.c"
	"src/3rdparty/lua/lobject.c"
	"src/3rdparty/lua/lopcodes.c"
	"src/3rdparty/lua/loslib.c"
	"src/3rdparty/lua/lparser.c"
	"src/3rdparty/lua/lstate.c"
	"src/3rdparty/lua/lstring.c"
	"src/3rdparty/lua/lstrlib.c"
	"src/3rdparty/lua/ltable.c"
	"src/3rdparty/lua/ltablib.c"
	"src/3rdparty/lua/ltm.c"
	"src/3rdparty/lua/lundump.c"
	"src/3rdparty/lua/lutf8lib.c"
	"src/3rdparty/lua/lvm.c"
	"src/3rdparty/lua/lzio.c"
)

# We will be building Box2D manually, as it is hacked to the utmost.

set(HYPERSOMNIA_BOX2D_CPPS
	"src/3rdparty/Box2D/Collision/b2BroadPhase.cpp"
	"src/3rdparty/Box2D/Collision/b2CollideCircle.cpp"
	"src/3rdparty/Box2D/Collision/b2CollideEdge.cpp"
	"src/3rdparty/Box2D/Collision/b2CollidePolygon.cpp"
	"src/3rdparty/Box2D/Collision/b2Collision.cpp"
	"src/3rdparty/Box2D/Collision/b2Distance.cpp"
	"src/3rdparty/Box2D/Collision/b2DynamicTree.cpp"
	"src/3rdparty/Box2D/Collision/b2TimeOfImpact.cpp"
	"src/3rdparty/Box2D/Collision/Shapes/b2ChainShape.cpp"
	"src/3rdparty/Box2D/Collision/Shapes/b2CircleShape.cpp"
	"src/3rdparty/Box2D/Collision/Shapes/b2EdgeShape.cpp"
	"src/3rdparty/Box2D/Collision/Shapes/b2PolygonShape.cpp"
	"src/3rdparty/Box2D/Common/b2BlockAllocator.cpp"
	"src/3rdparty/Box2D/Common/b2Draw.cpp"
	"src/3rdparty/Box2D/Common/b2Math.cpp"
	"src/3rdparty/Box2D/Common/b2Settings.cpp"
	"src/3rdparty/Box2D/Common/b2StackAllocator.cpp"
	"src/3rdparty/Box2D/Common/b2Timer.cpp"
	"src/3rdparty/Box2D/Dynamics/b2Body.cpp"
	"src/3rdparty/Box2D/Dynamics/b2ContactManager.cpp"
	"src/3rdparty/Box2D/Dynamics/b2Fixture.cpp"
	"src/3rdparty/Box2D/Dynamics/b2Island.cpp"
	"src/3rdparty/Box2D/Dynamics/b2World.cpp"
	"src/3rdparty/Box2D/Dynamics/b2WorldCallbacks.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2CircleContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2Contact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2ContactSolver.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Contacts/b2PolygonContact.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2DistanceJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2FrictionJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2GearJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2Joint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2MotorJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2MouseJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2PulleyJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2RopeJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2WeldJoint.cpp"
	"src/3rdparty/Box2D/Dynamics/Joints/b2WheelJoint.cpp"
	"src/3rdparty/Box2D/Rope/b2Rope.cpp"
)

# Test scene sources whose inclusion is conditional on the value of BUILD_TEST_SCENES.

set(HYPERSOMNIA_TEST_SCENES_CPPS
	"src/test_scenes/ingredients/artificial_life.cpp"
	"src/test_scenes/ingredients/backpack.cpp"
	"src/test_scenes/ingredients/car.cpp"
	"src/test_scenes/ingredients/guns.cpp"
	"src/test_scenes/ingredients/ingredients.cpp"
	"src/test_scenes/ingredients/melee_weapons.cpp"
	"src/test_scenes/ingredients/obstacles.cpp"
	"src/test_scenes/ingredients/decorations.cpp"
	"src/test_scenes/ingredients/wsad_player.cpp"
	"src/test_scenes/ingredients/grenades.cpp"
	
	"src/test_scenes/test_scene_images.cpp"
	"src/test_scenes/test_scene_particle_effects.cpp"
	"src/test_scenes/test_scene_physical_materials.cpp"
	"src/test_scenes/test_scene_sentience_properties.cpp"
	"src/test_scenes/test_scene_animations.cpp"
	"src/test_scenes/test_scene_sounds.cpp"
	"src/test_scenes/test_scene_recoil_players.cpp"

	"src/test_scenes/test_scene_flavours.cpp"
	"src/test_scenes/scenes/minimal_scene.cpp"

	"src/test_scenes/scenes/minimal_scene.cpp"
	"src/test_scenes/scenes/testbed.cpp"
)

# Order is important inasmuch as multi-threaded builds are concerned.
# Put intensive cpps to the very beginning.

# These cpps will be included as the first to speed up multithreaded build times.

set(HYPERSOMNIA_CPU_INTENSIVE_CPPS
	# This should be the first one to build to catch errors early
	"src/main.cpp"
	"src/work.cpp"
	"src/application/main/self_updater.cpp"
	"src/view/rendering_scripts/illuminated_rendering.cpp"
	"src/application/tests_of_traits.cpp"

)

if(BUILD_DEBUGGER_SETUP)
set(HYPERSOMNIA_DEBUGGER_SETUP_CPPS
	"src/application/setups/debugger/gui/debugger_fae_gui.cpp"
	"src/application/setups/debugger/debugger_setup.cpp"
	"src/application/setups/debugger/gui/debugger_common_state_gui.cpp"
	"src/application/setups/debugger/commands/change_property_command.cpp"
	"src/application/setups/debugger/gui/debugger_history_gui.cpp"
	"src/application/setups/debugger/gui/debugger_go_to_gui.cpp"
	"src/application/setups/debugger/debugger_camera.cpp"
	"src/application/setups/debugger/debugger_command_input.cpp"
	"src/application/setups/debugger/commands/fill_with_test_scene_command.cpp"
	"src/application/setups/debugger/commands/delete_entities_command.cpp"
	"src/application/setups/debugger/commands/move_entities_command.cpp"
	"src/application/setups/debugger/gui/debugger_entity_selector.cpp"
	"src/application/setups/debugger/property_debugger/fae/fae_tree_structs.cpp"
	"src/application/setups/debugger/commands/paste_entities_command.cpp"
	"src/application/setups/debugger/commands/duplicate_entities_command.cpp"
	"src/application/setups/debugger/commands/change_grouping_command.cpp"
	"src/application/setups/debugger/debugger_selection_groups.cpp"
	"src/application/setups/debugger/gui/debugger_selection_groups_gui.cpp"
	"src/application/setups/debugger/gui/debugger_summary_gui.cpp"
	"src/application/setups/debugger/gui/debugger_entity_mover.cpp"
	"src/application/setups/debugger/debugger_folder.cpp"
	"src/application/setups/debugger/debugger_recent_paths.cpp"
	"src/application/setups/debugger/gui/debugger_layers_gui.cpp"
	"src/application/setups/debugger/debugger_autosave.cpp"
	"src/application/setups/debugger/debugger_paths.cpp"
	"src/application/setups/debugger/debugger_history.cpp"
	"src/application/setups/debugger/debugger_view.cpp"
	"src/application/setups/debugger/gui/debugger_pathed_asset_gui.cpp"
	"src/application/setups/debugger/commands/asset_commands.cpp"
	"src/application/setups/debugger/gui/asset_path_browser_settings.cpp"
	"src/application/setups/debugger/commands/flavour_commands.cpp"
	"src/application/setups/debugger/gui/debugger_unpathed_asset_gui.cpp"
	"src/application/setups/debugger/property_debugger/widgets/frames_prologue_widget.cpp"
	"src/application/setups/debugger/property_debugger/property_debugger_structs.cpp"
	"src/application/setups/debugger/property_debugger/update_size_if_tex_changed.cpp"
	"src/application/setups/debugger/gui/debugger_player_gui.cpp"
	"src/application/setups/debugger/gui/debugger_modes_gui.cpp"
	"src/application/setups/debugger/debugger_player.cpp"
	"src/application/setups/debugger/gui/debugger_tutorial_gui.cpp"
)
else()
	set(HYPERSOMNIA_DEBUGGER_SETUP_CPPS "")
endif()

macro(file_flag path flags)
	set_source_files_properties(${path} PROPERTIES COMPILE_FLAGS ${flags})
endmacro()

if(BUILD_NETWORKING)
	list(APPEND HYPERSOMNIA_CPU_INTENSIVE_CPPS
		"src/application/setups/server/server_setup.cpp"
		"src/application/setups/client/client_setup.cpp"
		"src/application/network/network_adapters.cpp"
		"src/augs/network/network_types.cpp"
	)
endif()

set(HYPERSOMNIA_CODEBASE_CPPS
	${HYPERSOMNIA_CPU_INTENSIVE_CPPS}
	${HYPERSOMNIA_DEBUGGER_SETUP_CPPS}

	"src/application/setups/editor/commands/toggle_active_commands.cpp"
	"src/application/setups/editor/commands/clone_nodes_command.cpp"
	"src/application/setups/editor/commands/delete_nodes_command.cpp"
	"src/application/setups/editor/mover/editor_node_mover.cpp"
	"src/application/setups/editor/commands/node_transform_commands.cpp"
	"src/application/setups/editor/selector/editor_entity_selector.cpp"
	"src/application/setups/editor/official/create_official_resources.cpp"
	"src/application/setups/editor/editor_camera.cpp"
	"src/augs/misc/secure_hash.cpp"
	"src/application/setups/editor/project/editor_project_readwrite.cpp"
	"src/application/setups/editor/project/editor_project_paths.cpp"
	"src/application/setups/editor/gui/editor_history_gui.cpp"
	"src/augs/string/path_sanitization.cpp"
	"src/augs/misc/imgui/simple_popup.cpp"
	"src/application/config_lua_table.cpp"
	"src/view/audiovisual_state/systems/interpolation_system.cpp"
	"src/view/audiovisual_state/systems/particles_simulation_system.cpp"
	"src/view/audiovisual_state/systems/past_infection_system.cpp"
	"src/view/audiovisual_state/systems/pure_color_highlight_system.cpp"
	"src/view/audiovisual_state/systems/sound_system.cpp"
	"src/view/audiovisual_state/systems/thunder_system.cpp"
	"src/view/audiovisual_state/systems/damage_indication_system.cpp"
	"src/game/stateless_systems/destruction_system.cpp"
	"src/game/stateless_systems/demolitions_system.cpp"
	"src/game/stateless_systems/melee_system.cpp"
	"src/game/stateless_systems/particles_existence_system.cpp"
	"src/game/stateless_systems/sound_existence_system.cpp"
	"src/game/stateless_systems/behaviour_tree_system.cpp"
	"src/game/stateless_systems/car_system.cpp"
	"src/game/stateless_systems/crosshair_system.cpp"
	"src/game/stateless_systems/missile_system.cpp"
	"src/game/stateless_systems/deletion_system.cpp"
	"src/game/stateless_systems/driver_system.cpp"
	"src/game/stateless_systems/force_joint_system.cpp"
	"src/game/stateless_systems/gun_system.cpp"
	"src/game/stateless_systems/input_system.cpp"
	"src/game/stateless_systems/intent_contextualization_system.cpp"
	"src/game/stateless_systems/item_system.cpp"
	"src/game/stateless_systems/movement_system.cpp"
	"src/game/stateless_systems/pathfinding_system.cpp"
	"src/game/stateless_systems/sentience_system.cpp"
	"src/game/stateless_systems/trace_system.cpp"
	"src/game/stateless_systems/visibility_system.cpp"
	"src/game/stateless_systems/physics_system.cpp"
	"src/game/stateless_systems/portal_system.cpp"
	"src/view/viewables/image_definition.cpp"
	"src/view/viewables/particle_effect.cpp"
	"src/view/audiovisual_state/aabb_highlighter.cpp"
	"src/view/game_gui/elements/action_button.cpp"
	"src/view/game_gui/elements/character_gui.cpp"
	"src/view/game_gui/elements/drag_and_drop.cpp"
	"src/view/game_gui/elements/gui_grid.cpp"
	"src/view/game_gui/elements/gui_positioning.cpp"
	"src/view/game_gui/elements/hotbar_button.cpp"
	"src/view/game_gui/elements/item_button.cpp"
	"src/view/game_gui/elements/pixel_line_connector.cpp"
	"src/view/game_gui/elements/drag_and_drop_target_drop_item.cpp"
	"src/view/game_gui/elements/game_gui_root.cpp"
	"src/view/game_gui/elements/value_bar.cpp"
	"src/view/game_gui/elements/slot_button.cpp"
	"src/view/audiovisual_state/systems/exploding_ring_system.cpp"
	"src/view/game_gui/game_gui_system.cpp"
	"src/view/audiovisual_state/systems/light_system.cpp"
	"src/view/rendering_scripts/draw_sentiences_hud.cpp"
	"src/view/rendering_scripts/draw_explosion_body_highlights.cpp"
	"src/view/rendering_scripts/draw_crosshair_lasers.cpp"
	"src/view/rendering_scripts/draw_circular_progresses.cpp"
	"src/view/audiovisual_state/world_camera.cpp"
	"src/view/necessary_resources.cpp"
	"src/view/audiovisual_state/audiovisual_state.cpp"
	"src/view/viewables/loaded_sounds_map.cpp"
	"src/view/viewables/standard_atlas_distribution.cpp"
	"src/view/viewables/all_viewables_defs.cpp"
	"src/view/audiovisual_state/audiovisual_profiler.cpp"
	"src/view/viewables/image_cache.cpp"
	"src/view/frame_profiler.cpp"
	"src/view/viewables/regeneration/desaturations.cpp"
	"src/view/viewables/regeneration/buttons_with_corners.cpp"
	"src/view/viewables/regeneration/images_from_commands.cpp"
	"src/view/viewables/regeneration/neon_maps.cpp"
	"src/augs/graphics/texture.cpp"
	"src/augs/gui/button_corners.cpp"
	"src/augs/audio/audio_context.cpp"
	"src/augs/audio/sound_buffer.cpp"
	"src/augs/audio/sound_source.cpp"
	"src/augs/ensure.cpp"
	"src/augs/drawing/drawing.cpp"
	"src/augs/gui/clipboard.cpp"
	"src/augs/gui/gui_traversal_structs.cpp"
	"src/augs/gui/formatted_string.cpp"
	"src/augs/gui/rect.cpp"
	"src/augs/image/font.cpp"
	"src/augs/image/image.cpp"
	"src/augs/window_framework/shell.cpp"
	"src/augs/misc/action_list/action_list.cpp"
	"src/augs/misc/enum/enum_bitset.cpp"
	"src/augs/misc/readable_bytesize.cpp"
	"src/augs/misc/action_list/standard_actions.cpp"
	"src/augs/readwrite/memory_stream.cpp"
	"src/augs/misc/time_utils.cpp"
	"src/augs/string/typesafe_sprintf.cpp"
	"src/augs/string/typesafe_sscanf.cpp"
	"src/augs/texture_atlas/bake_fresh_atlas.cpp"
	"src/game/assets/animation.cpp"
	"src/game/assets/behaviour_tree.cpp"
	"src/game/assets/physical_material.cpp"
	"src/game/components/flags_component.cpp"
	"src/game/components/light_component.cpp"
	"src/game/components/rigid_body_component.cpp"
	"src/game/components/sentience_component.cpp"
	"src/game/detail/standard_explosion.cpp"
	"src/game/detail/inventory/inventory_slot.cpp"
	"src/game/detail/inventory/inventory_slot_id.cpp"
	"src/game/detail/inventory/inventory_utils.cpp"
	"src/game/detail/physics/contact_listener.cpp"
	"src/game/detail/physics/physics_friction_fields.cpp"
	"src/game/detail/physics/ray_casts.cpp"
	"src/game/detail/physics/physics_scripts.cpp"
	"src/augs/misc/value_meter.cpp"
	"src/game/detail/visible_entities.cpp"
	"src/game/detail/inventory/wielding_result.cpp"
	"src/game/enums/attitude_type.cpp"
	"src/game/enums/item_category.cpp"
	"src/game/enums/slot_physical_behaviour.cpp"
	"src/game/detail/ai/create_standard_behaviour_trees.cpp"
	"src/game/inferred_caches/physics_world_cache.cpp"
	"src/game/inferred_caches/tree_of_npo_cache.cpp"
	"src/game/other_unit_tests.cpp"
	"src/game/cosmos/cosmic_entropy.cpp"
	"src/game/cosmos/data_living_one_step.cpp"
	"src/augs/filesystem/directory.cpp"
	"src/augs/gui/appearance_detector.cpp"
	"src/augs/misc/timing/delta.cpp"
	"src/augs/misc/timing/stepped_timing.cpp"
	"src/augs/window_framework/platform_utils.cpp"
	"src/game/components/car_component.cpp"
	"src/game/components/container_component.cpp"
	"src/game/components/missile_component.cpp"
	"src/game/components/movement_component.cpp"
	"src/game/components/pathfinding_component.cpp"
	"src/game/cosmos/cosmos.cpp"
	"src/game/detail/ai/behaviours.cpp"
	"src/game/detail/ai/behaviours/explore_in_search_for_last_seen_target.cpp"
	"src/game/detail/ai/behaviours/immediate_evasion.cpp"
	"src/game/detail/ai/behaviours/minimize_recoil_through_movement.cpp"
	"src/game/detail/ai/behaviours/navigate_to_last_seen_position_of_target.cpp"
	"src/game/detail/ai/behaviours/pull_trigger.cpp"
	"src/game/detail/ai/behaviours/target_closest_enemy.cpp"
	"src/game/detail/entity_scripts.cpp"
	"src/game/enums/filters.cpp"
	"src/game/detail/describers.cpp"
	"src/game/detail/names_and_descriptions.cpp"
	"src/game/cosmos/cosmos_solvable_significant.cpp"
	"src/application/setups/test_scene_setup.cpp"
	"src/application/setups/main_menu_setup.cpp"
	"src/application/web_daemon/session_report.cpp"
	"src/augs/global_libraries.cpp"
	"src/augs/graphics/fbo.cpp"
	"src/augs/graphics/rgba.cpp"
	"src/augs/graphics/renderer.cpp"
	"src/augs/graphics/renderer_backend.cpp"
	"src/augs/graphics/shader.cpp"
	"src/augs/graphics/vertex.cpp"
	"src/augs/audio/audio_backend.cpp"
	"src/augs/gui/dragger.cpp"
	"src/augs/gui/rect_world.cpp"
	"src/augs/gui/text/caret.cpp"
	"src/augs/gui/text/drafter.cpp"
	"src/augs/gui/text/draft_redrawer.cpp"
	"src/augs/gui/text/printer.cpp"
	"src/augs/gui/text/word_separator.cpp"
	"src/augs/math/rects.cpp"
	"src/augs/math/math.cpp"
	"src/augs/misc/timing/fixed_delta_timer.cpp"
	"src/augs/misc/randomization.cpp"
	"src/augs/misc/smooth_value_field.cpp"
	"src/augs/misc/timing/timer.cpp"
	"src/augs/log.cpp"
	"src/augs/window_framework/event.cpp"
	"src/augs/window_framework/window.cpp"
	"src/augs/audio/sound_data.cpp"
	"src/game/inferred_caches/relational_cache.cpp"
	"src/game/components/motor_joint_component.cpp"
	"src/augs/misc/enum/enum_boolset.cpp"
	"src/augs/string/string_templates.cpp"
	"src/game/components/sender_component.cpp"
	"src/game/inferred_caches/flavour_id_cache.cpp"
	"src/game/detail/spells/electric_shield.cpp"
	"src/game/detail/spells/electric_triad.cpp"
	"src/game/detail/spells/fury_of_the_aeons.cpp"
	"src/game/detail/spells/haste.cpp"
	"src/game/detail/spells/ultimate_wrath_of_the_aeons.cpp"
	"src/game/detail/spells/exaltation.cpp"
	"src/game/detail/spells/echoes_of_the_higher_realms.cpp"
	"src/augs/misc/lua/lua_utils.cpp"
	"src/test_scenes/test_scenes_content.cpp"
	"src/augs/misc/imgui/imgui_utils.cpp"
	"src/augs/misc/imgui/imgui_control_wrappers.cpp"
	"src/application/gui/settings_gui.cpp"
	"src/application/gui/start_client_gui.cpp"
	"src/application/gui/start_server_gui.cpp"
	"src/game/assets/recoil_player.cpp"
	"src/augs/gui/bbcode.cpp"
	"src/augs/unit_tests.cpp"
	"src/game/cosmos/cosmic_profiler.cpp"
	"src/application/main/draw_debug_lines.cpp"
	"src/application/main/draw_debug_details.cpp"
	"src/application/main/imgui_pass.cpp"
	"src/application/main/release_flags.cpp"
	"src/game/debug_drawing_settings.cpp"
	"src/application/session_profiler.cpp"
	"src/application/intercosm.cpp"
	"src/augs/misc/imgui/imgui_controls.cpp"
	"src/augs/readwrite/readwrite_tests.cpp"
	"src/game/components/trace_component.cpp"
	"src/game/cosmos/solvers/standard_solver.cpp"
	"src/game/cosmos/cosmos_solvable.cpp"
	"src/game/cosmos/cosmos_common.cpp"
	"src/game/detail/inventory/perform_transfer.cpp"
	"src/game/cosmos/cosmic_functions.cpp"
	"src/game/detail/view_input/particle_effect_input.cpp"
	"src/game/detail/view_input/sound_effect_input.cpp"
	"src/game/detail/spells/spell_logic_input.cpp"
	"src/game/inferred_caches/processing_lists_cache.cpp"
	"src/game/cosmos/entity_id.cpp"
	"src/augs/misc/children_vector_tracker.cpp"
	"src/augs/templates/container_templates.cpp"
	"src/augs/templates/history.cpp"
	"src/game/cosmos/state_tests.cpp"
	"src/build_info.cpp"
	"src/augs/misc/pool/pool.cpp"
	"src/augs/math/snapping_grid.cpp"
	"src/game/detail/sentience_shake.cpp"
	"src/augs/string/get_type_name.cpp"
	"src/augs/misc/imgui/path_tree_structs.cpp"
	"src/augs/texture_atlas/atlas_profiler.cpp"
	"src/augs/graphics/pbo.cpp"
	"src/view/viewables/streaming/viewables_streaming.cpp"
	"src/view/viewables/streaming/viewables_streaming_profiler.cpp"
	"src/game/stateless_systems/movement_path_system.cpp"
	"src/game/stateless_systems/animation_system.cpp"
	"src/game/detail/organisms/startle_nearbly_organisms.cpp"
	"src/game/stateless_systems/remnant_system.cpp"
	"src/view/audiovisual_state/systems/randomizing_system.cpp"
	"src/game/modes/test_mode.cpp"
	"src/game/components/hand_fuse_component.cpp"
	"src/game/debug_draw.cpp"
	"src/game/detail/explosive/detonate.cpp"
	"src/game/modes/arena_mode.cpp"
	"src/view/mode_gui/arena/arena_mode_gui.cpp"
	"src/view/asset_funcs.cpp"
	"src/view/mode_gui/arena/arena_scoreboard_gui.cpp"
	"src/view/mode_gui/arena/arena_choose_team_gui.cpp"
	"src/game/detail/sentience/sentience_logic.cpp"
	"src/game/cosmos/cosmos_global_solvable.cpp"
	"src/augs/misc/enum/enum_map.cpp"
	"src/view/mode_gui/arena/arena_buy_menu_gui.cpp"
	"src/game/detail/flavour_scripts.cpp"
	"src/game/modes/mode_entropy.cpp"
	"src/application/input/adjust_game_motions.cpp"
	"src/application/network/simulation_receiver.cpp"
	"src/application/arena/arena_paths.cpp"
	"src/application/arena/intercosm_paths.cpp"
	"src/augs/misc/compress.cpp"
	"src/fp_consistency_tests.cpp"
	"src/view/mode_gui/arena/arena_spectator_gui.cpp"
	"src/game/inferred_caches/organism_cache.cpp"
	"src/view/viewables/avatar_atlas.cpp"
	"src/augs/window_framework/create_process.cpp"
	"src/application/gui/client/chat_gui.cpp"
	"src/application/gui/client/client_gui_state.cpp"
	"src/application/gui/browse_servers_gui.cpp"
	"src/application/masterserver/masterserver.cpp"
	"src/application/nat/nat_detection_session.cpp"
	"src/application/nat/nat_traversal_session.cpp"
	"src/application/setups/server/server_nat_traversal.cpp"
	"src/application/main/miniature_generator.cpp"
	"src/application/setups/editor/editor_setup.cpp"
	"src/application/setups/editor/editor_setup_imgui.cpp"
	"src/application/setups/editor/gui/editor_inspector_gui.cpp"
	"src/application/setups/editor/gui/editor_filesystem_gui.cpp"
	"src/application/setups/editor/gui/editor_layers_gui.cpp"
	"src/application/setups/project_selector/project_selector_setup.cpp"
	"src/view/hud_messages/hud_messages_gui.cpp"
	"src/application/setups/editor/gui/editor_toolbar_gui.cpp"
	"src/application/setups/client/arena_downloading_session.cpp"
	"src/application/setups/client/https_file_downloader.cpp"
	"src/application/gui/map_catalogue_gui.cpp"
)

# The rest of 3rdparty libraries with minimal amount of source files.

set(HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS
	"src/3rdparty/lodepng/lodepng.cpp"
	"src/3rdparty/polypartition/src/polypartition.cpp"
	"src/3rdparty/imgui/imgui.cpp"
	"src/3rdparty/imgui/imgui_widgets.cpp"
	"src/3rdparty/imgui/imgui_tables.cpp"
	"src/3rdparty/imgui/imgui_draw.cpp"
	"src/3rdparty/imgui/imgui_demo.cpp"
	"src/augs/misc/imgui/addons/imguitabwindow/imguitabwindow.cpp"

	"src/3rdparty/crc32/crc32.c"
)

file_flag("src/3rdparty/imgui/imgui.cpp" "-Wno-error")
file_flag("src/augs/misc/imgui/addons/imguitabwindow/imguitabwindow.cpp" "-Wno-error")

# These headers will be input to the Introspector-generator.

file(GLOB_RECURSE HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES
    "src/hypersomnia_version.h"
    "src/fp_consistency_tests.h"
	"src/augs/*.h"
	"src/game/*.h"
	"src/view/*.h"
	"src/test_scenes/*.h"
	"src/application/*.h"
	"src/3rdparty/Box2D/Common/*.h"
	"src/3rdparty/Box2D/Dynamics/*.h"

	"src/application/tests_of_traits.cpp"
	"src/augs/templates/introspect.cpp"
)

# Source files that are included conditionally.
# Conditional inclusion of source files happens here.

if(BUILD_WINDOW_FRAMEWORK)
	if(USE_GLFW)
		message("Using GLFW as the window framework.")

		list(APPEND HYPERSOMNIA_CODEBASE_CPPS
			"src/augs/window_framework/window_glfw.cpp"
			"src/augs/window_framework/translate_glfw_enums.cpp"
		)
	elseif(MSVC)
		list(APPEND HYPERSOMNIA_CODEBASE_CPPS
			"src/augs/window_framework/window_winapi.cpp"
			"src/augs/window_framework/translate_winapi_enums.cpp"
		)
	elseif(UNIX)
		list(APPEND HYPERSOMNIA_CODEBASE_CPPS
			"src/augs/window_framework/window_x.cpp"
			"src/augs/window_framework/translate_x_enums.cpp"
			"src/augs/window_framework/print_x_devices.cpp"
		)
	endif()
else()
	list(APPEND HYPERSOMNIA_CODEBASE_CPPS
		"src/augs/window_framework/window_stubs.cpp"
	)
endif()

if(BUILD_OPENGL)
	list(APPEND HYPERSOMNIA_CODEBASE_CPPS
		"src/augs/graphics/OpenGL_error.cpp"
	)
endif()

if(BUILD_OPENAL)
	list(APPEND HYPERSOMNIA_CODEBASE_CPPS
		"src/augs/audio/OpenAL_error.cpp"
	)
endif()

if(BUILD_TEST_SCENES)
	list(APPEND HYPERSOMNIA_CODEBASE_CPPS
		${HYPERSOMNIA_TEST_SCENES_CPPS}
	)

	message("Building of test scenes is enabled.")
endif()

# natvis files for easier debugging in Visual Studio.

if(MSVC_SPECIFIC)
	set(HYPERSOMNIA_NATVIS_FILES
		"src/game/cosmos/entity_handle.natvis"
		"src/game/detail/inventory/inventory_slot_handle.natvis"
		"src/augs/misc/pool/pooled_object_id.natvis"
	)
endif()

if(BUILD_GLAD)
	list(APPEND HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS
		"src/3rdparty/glad/glad.c"
	)
endif()

if(BUILD_NETWORKING)
	list(APPEND HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS
		"src/3rdparty/yojimbo/yojimbo.cpp"
		"src/3rdparty/yojimbo/tlsf/tlsf.c"
		"src/3rdparty/yojimbo/reliable.io/reliable.c"
	)

	# "src/3rdparty/yojimbo/tlsf/tlsf.h"
	# "src/3rdparty/yojimbo/yojimbo.h"
	# "src/3rdparty/yojimbo/netcode.io/netcode.h"
	# "src/3rdparty/yojimbo/reliable.io/reliable.h" 
endif()

# Project resource files, e.g. an icon.

if(ADD_APPLICATION_ICON)
	if(MSVC)
		set(HYPERSOMNIA_RC_FILES
			"${CMAKE_MODULE_PATH}/Hypersomnia.rc"
		)
	endif()
endif()

### SPECIFICATION OF ALL SOURCE FILES END HERE ###
##################################################

if(BUILD_LUA_AS_CPP)
	set_source_files_properties(${LUA_CPPS} PROPERTIES LANGUAGE CXX)
endif()

# We arrange the directories for the generated sources paths.

set(GENERATORS_OUTPUT_PATH "${PROJECT_BINARY_DIR}/generators_output")
set(GENERATED_SOURCES_PATH "${GENERATORS_OUTPUT_PATH}/generated")

file(MAKE_DIRECTORY ${GENERATED_SOURCES_PATH})

include_directories(${GENERATORS_OUTPUT_PATH})

set(GENERATED_INTROSPECTORS_HEADER "${GENERATED_SOURCES_PATH}/introspectors.h")

# Before Hypersomnia builds,
# Introspector-generator must first generate the type information.

# We build the generator of type information required for introspection.

add_subdirectory("${CMAKE_MODULE_PATH}/Introspector-generator")

# We prepare a correct input configuration file for Introspector-generator.

# CMake separates elements of lists with a semicolon, but the Introspector-generator accepts lists separated by newlines.
# We must correct that.

set(INTROSPECTOR_GENERATOR_INPUT_CONFIG_PATH_TEMPLATE
	"src/introspector_generator_input.cfg.in"
)

set(INTROSPECTOR_GENERATOR_INPUT_CONFIG_PATH
	"${PROJECT_BINARY_DIR}/introspector_generator_input.cfg"
)

# The correct input configuration file shall contain paths to all headers for whom the generator should create introspectors,
# and should also contain the correct output file path.
# Thus, that call should substitute:
# HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES_NEWLINE_SEPARATED 
# and GENERATED_SOURCES_PATH for the output file path.

string (REPLACE ";" "\n" 
	HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES_NEWLINE_SEPARATED 
	"${HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES}"
)

configure_file(
	${INTROSPECTOR_GENERATOR_INPUT_CONFIG_PATH_TEMPLATE}
	${INTROSPECTOR_GENERATOR_INPUT_CONFIG_PATH}
	@ONLY
)

add_custom_command(
	OUTPUT ${GENERATED_INTROSPECTORS_HEADER}
	COMMAND Introspector-generator ${INTROSPECTOR_GENERATOR_INPUT_CONFIG_PATH}
	COMMENT "Generating type information for introspection..."
	DEPENDS ${HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES}
)

# Before Hypersomnia builds,
# version_file_generator must first query git for information about the most recent commit.

set(HYPERSOMNIA_VERSION_FILE_INPUT_PATH
	"${PROJECT_SOURCE_DIR}/src/hypersomnia_version.cpp.in"
)

set(HYPERSOMNIA_VERSION_FILE_CPP
	"${GENERATED_SOURCES_PATH}/hypersomnia_version.cpp"
)

# So from now on, we will be using Git to obtain some information about commits.

find_package(Git)

message("Git path: ${GIT_EXECUTABLE}")

if(NOT (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git"))
	set(BUILD_VERSION_FILE_GENERATOR FALSE)
	message("Git not found. Some functionality will be disabled.")
endif()

if(BUILD_VERSION_FILE_GENERATOR)
	add_subdirectory("${CMAKE_MODULE_PATH}/version_file_generator")

	add_custom_command(
		OUTPUT ${HYPERSOMNIA_VERSION_FILE_CPP}
		COMMAND version_file_generator ${GIT_EXECUTABLE} ${HYPERSOMNIA_VERSION_FILE_INPUT_PATH} ${HYPERSOMNIA_VERSION_FILE_CPP}
		COMMENT "Generating version files..."
	)
else()
	message("Version file generator will not be built. Copying ${HYPERSOMNIA_VERSION_FILE_INPUT_PATH} into ${HYPERSOMNIA_VERSION_FILE_CPP}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${HYPERSOMNIA_VERSION_FILE_INPUT_PATH} ${HYPERSOMNIA_VERSION_FILE_CPP})
endif()

macro(prefer_static_libraries)
	if (UNIX)
		message("These libraries will be linked statically.")

		message("Default CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")

		list( REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES   ".a" )
		list( INSERT      CMAKE_FIND_LIBRARY_SUFFIXES 0 ".a" )

		message("CMAKE_FIND_LIBRARY_SUFFIXES after adjustment: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
	endif()
endmacro()

macro(prefer_shared_libraries)
	if (UNIX)
		message("These libraries will be linked dynamically.")

		message("Default CMAKE_FIND_LIBRARY_SUFFIXES: ${CMAKE_FIND_LIBRARY_SUFFIXES}")

		list( REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES   ".so" )
		list( INSERT      CMAKE_FIND_LIBRARY_SUFFIXES 0 ".so" )

		message("CMAKE_FIND_LIBRARY_SUFFIXES after adjustment: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
	endif()
endmacro()

# We add headers to aid IntelliSense.

set(HYPERSOMNIA_HEADERS
	${HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES}
	${GENERATED_INTROSPECTORS_HEADER}
)

# We configure include directories for Hypersomnia codebase.

set(HYPERSOMNIA_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/3rdparty"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/lua"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/sol2/include"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/imgui"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/yojimbo/reliable.io"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/yojimbo/netcode.io"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/rapidjson/include"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/blake"
	# Only for v3
	# "${PROJECT_SOURCE_DIR}/src/3rdparty/Catch/src"
)

if (MSVC)
	list(APPEND HYPERSOMNIA_INCLUDE_DIRS
		"${PROJECT_SOURCE_DIR}/src/3rdparty/yojimbo/windows"
	)
endif()

# We configure additional flags for the compiler.

if(GCC OR CLANG)
	# Architecture switches
	if(${ARCHITECTURE} STREQUAL "x64")
		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -m64")
		set(HYPERSOMNIA_C_FLAGS "${HYPERSOMNIA_C_FLAGS} -m64")
	endif()

	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} ${XCLANG_STR} -fno-threadsafe-statics")

	message("-O0 will be specified for Debug build.")
	set(HYPERSOMNIA_CXX_DEBUG_FLAGS "${HYPERSOMNIA_CXX_DEBUG_FLAGS} ${XCLANG_STR} -O0")
	set(HYPERSOMNIA_C_DEBUG_FLAGS "${HYPERSOMNIA_C_DEBUG_FLAGS} ${XCLANG_STR} -O0")

	if(NOT MSVC)
		# Generate debug information (we don't care about C libraries at this point)
		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -g")
	endif()
endif()

if(CLANG)
	if(NOT MSVC)
		set(CHOSEN_WARNINGS "-Wall -Werror -Wextra")
	endif()

	# -Wall and -Wextra cover everything already
	set(WARNINGS_FOR_OUR_CODE_ONLY " ")

	set(DISABLED_WARNINGS "-Wno-error=unused-command-line-argument -Wno-error=missing-braces")

	if (APPLE)
		set(DISABLED_WARNINGS "${DISABLED_WARNINGS} -Wno-error=deprecated-declarations")
	endif()

	# Latest features of C++17 will be enabled.
	set(HYPERSOMNIA_CXX_FLAGS 
		"${HYPERSOMNIA_CXX_FLAGS} ${CHOSEN_WARNINGS}  -fcolor-diagnostics ${DISABLED_WARNINGS}"
	)

	if(NOT MSVC)
		set(HYPERSOMNIA_CXX_FLAGS 
			"${HYPERSOMNIA_CXX_FLAGS} -ftemplate-backtrace-limit=0"
		)
	endif()

	set_source_files_properties(${HYPERSOMNIA_CODEBASE_CPPS} PROPERTIES COMPILE_FLAGS ${WARNINGS_FOR_OUR_CODE_ONLY})

	file_flag("src/augs/image/image.cpp" "-Wno-error")
	file_flag("src/augs/network/network_types.cpp" "-Wno-error=deprecated-declarations")
	file_flag("src/3rdparty/yojimbo/yojimbo.cpp" "-Wno-error=deprecated-declarations")
	file_flag("src/3rdparty/yojimbo/netcode.io/netcode.c" "-Wno-error=deprecated-declarations")
	file_flag("src/3rdparty/yojimbo/reliable.io/reliable.c" "-Wno-error=deprecated-declarations")
endif()

if(APPLE)
	file_flag("src/augs/network/network_types.cpp" "-Wno-error")
	file_flag("src/3rdparty/yojimbo/yojimbo.cpp" "-Wno-error")
	file_flag("src/3rdparty/yojimbo/netcode.io/netcode.c" "-Wno-error")
	file_flag("src/3rdparty/yojimbo/reliable.io/reliable.c" "-Wno-error")
endif()

if(GCC)

# TODO: -Wunused-value fails with SFINAE
	# We could use for example (void)T::value
# TODO: -Wduplicated-branch fails with some of our custom type traits

	set(CHOSEN_WARNINGS 
"-Wdangling-else \
-Wbool-operation \
-Wbool-compare \
-Wtautological-compare \
-Wpointer-arith \
-Wpointer-compare \
-Wtype-limits \
-Wlogical-op \
-Wlogical-not-parentheses \
-Wdelete-incomplete \
-Wplacement-new \
-Wreturn-type \
-Wswitch \
-Wswitch-unreachable \
-Wparentheses \
-Wsequence-point \
-Wduplicated-cond \
-Wtrigraphs \
-Winit-self"
	)

	set(WARNINGS_FOR_OUR_CODE_ONLY "-Wall -Wextra -Werror -Wno-unused-value")

	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} ${CHOSEN_WARNINGS} -ftemplate-backtrace-limit=0 -fdiagnostics-color")
	set_source_files_properties(${HYPERSOMNIA_CODEBASE_CPPS} PROPERTIES COMPILE_FLAGS ${WARNINGS_FOR_OUR_CODE_ONLY})
endif()

if(MSVC_SPECIFIC)
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} /MP /GL")
endif()

# Latest features of C++17 will be enabled.

if(MSVC)
	# Inline a lot. Default is /Ob1.
	string(REPLACE "/Ob1" "/Ob2" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} /std:c++latest /fp:strict /bigobj /permissive-")
else()
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -std=c++20")
endif()

if (PREFER_LIBCXX AND CLANG AND NOT MSVC)
	set(USE_LIBCXX ON)
endif()

if(CLANG)
	if(USE_LIBCXX)
		message("Chosen standard library: libc++")
		# Looks like stdlib=libc++ is duplicated for the linker as well,
		# so we only have to add it to CXX.
		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -stdlib=libc++")
	elseif(UNIX)
		message("Chosen standard library: libstdc++")

		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -stdlib=libstdc++")
	endif()

	if (ENABLE_THREADSANITIZER)
		message("Building with thread sanitizer.")

		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -fsanitize=thread")
	endif()

	if (ENABLE_ADDRESSSANITIZER)
		message("Building with address sanitizer.")

		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -fsanitize=address")
	endif()
endif()

if (NOT USE_O3)
	message("Forcing use of O2.")

	string(REPLACE "O3" "O2" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
	message("Forcing use of O3.")
	string(REPLACE "O2" "O3" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()

if (LINK_TIME_OPTIMIZATION)
	if(CLANG)
		set(HYPERSOMNIA_CXX_RELEASE_FLAGS "-flto -fwhole-program-vtables")
		set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "-flto -fwhole-program-vtables")
	endif()
endif()

if(ADD_MCMODEL_LARGE_FLAG)
	message("-mcmodel=large -no-pie flag will be specified.")
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -mcmodel=large -no-pie")
endif()

if(UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

if(LINUX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -unwindlib=libgcc")
endif()

# We configure additional flags for linker.

if (CLANG AND NOT MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FUSE_LD_FLAG}")
endif()

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if (MSVC_SPECIFIC)
	if (LINK_TIME_OPTIMIZATION)
		set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "/LTCG")
	endif()
endif()

if (MSVC)
	set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "/INCREMENTAL:NO")
	set(HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS "/INCREMENTAL:NO")
	
	if (GENERATE_DEBUG_INFORMATION)
		set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "${HYPERSOMNIA_LINKER_RELEASE_FLAGS} /DEBUG")
	endif()

	set(HYPERSOMNIA_LINKER_DEBUG_FLAGS "${HYPERSOMNIA_LINKER_DEBUG_FLAGS} /OPT:NOREF")
	set(HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS "${HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS} /OPT:ICF /OPT:REF")
	set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "${HYPERSOMNIA_LINKER_RELEASE_FLAGS} /OPT:ICF /OPT:REF")
endif()

if (GENERATE_DEBUG_INFORMATION)
	if (MSVC)
		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} /Zi")
	endif()
endif()

# We configure some preprocessor defines.

add_definitions(-DSTREFLOP_SSE)
add_definitions(-DLIBM_COMPILING_FLT32)
add_definitions(-DIMGUI_USER_CONFIG="3rdparty/imconfig.h")

if(UNIX)
	add_definitions(-DLUA_USE_LINUX)
endif()

if(BUILD_LUA_AS_CPP)
	# Lua is configured to compile as C++, of which sol2 library must necessarily know.
	add_definitions(-DSOL_USING_CXX_LUA)
endif()

# We must wait till SOL will fixes these warnings.

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

# We must wait till Catch fixes this warning.

add_definitions(-D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING)

if(MSVC)
	add_definitions(-DPLATFORM_WINDOWS=1)

	add_definitions(
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS
		-D_SCL_SECURE_NO_WARNINGS
		-DUNICODE
	  )
elseif(UNIX)
    add_definitions(-DPLATFORM_UNIX=1)

	if(APPLE)
		add_definitions(-DPLATFORM_MACOS=1)
	else()
		add_definitions(-DPLATFORM_LINUX=1)
	endif()
endif()

add_definitions(-DYOJIMBO_WITH_MBEDTLS=0)

# Additionally, we mark Release as the production build.

set(HYPERSOMNIA_CXX_RELEASE_FLAGS
 "${HYPERSOMNIA_CXX_RELEASE_FLAGS} -DIS_PRODUCTION_BUILD=1"
)

# We apply the variables to the actual flags.

set(CMAKE_EXE_LINKER_FLAGS_DEBUG 
 "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${HYPERSOMNIA_LINKER_DEBUG_FLAGS}"
)

set(CMAKE_EXE_LINKER_FLAGS_RELEASE 
 "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${HYPERSOMNIA_LINKER_RELEASE_FLAGS}"
)

set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO 
 "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS}"
)

set(CMAKE_C_FLAGS 
 "${CMAKE_C_FLAGS} ${HYPERSOMNIA_C_FLAGS}"
)

set(CMAKE_C_FLAGS_DEBUG
 "${CMAKE_C_FLAGS_DEBUG} ${HYPERSOMNIA_C_DEBUG_FLAGS}"
)

set(CMAKE_CXX_FLAGS 
 "${CMAKE_CXX_FLAGS} ${HYPERSOMNIA_CXX_FLAGS}"
)

set(CMAKE_CXX_FLAGS_DEBUG 
 "${CMAKE_CXX_FLAGS_DEBUG} ${HYPERSOMNIA_CXX_DEBUG_FLAGS}"
)

set(CMAKE_CXX_FLAGS_RELEASE 
 "${CMAKE_CXX_FLAGS_RELEASE} ${HYPERSOMNIA_CXX_RELEASE_FLAGS}"
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
 "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${HYPERSOMNIA_CXX_RELWITHDEBINFO_FLAGS}"
)

message("GENERATE_DEBUG_INFORMATION: ${GENERATE_DEBUG_INFORMATION}")
message("BUILD_DEBUGGER_SETUP: ${BUILD_DEBUGGER_SETUP}")

if(GENERATE_DEBUG_INFORMATION)
else()
	message("Removing -g flag.")
	string(REPLACE "-g" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	string(REPLACE "-g" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})

	string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	string(REPLACE "-g" "" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

	string(REPLACE "-g" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	string(REPLACE "-g" "" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

	string(REPLACE "-g" "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
	string(REPLACE "-g" "" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

	if(MSVC)
		string(REPLACE "/debug" "" CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
		string(REPLACE "/debug" "" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
	endif()
endif()

# We configure MSVC to use static runtime libraries ("/MT" switch),
# instead of DLL ones.
# The definition of this function is found in cmake/configure_runtime_library.cmake

include(configure_runtime_library)
configure_msvc_runtime()

# We build Lua manually as there is no up-to-date CMakeLists for Lua.

add_library(lua ${LUA_CPPS})

# As some libraries do not use the preferred target_include_directories,
# we must manually acquire the directories that they add via include_directories.

include(add_include_dirs_of)

if(BUILD_SOUND_FORMAT_DECODERS)
	# We build OGG.
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/ogg")
	
	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(ogg PUBLIC -mcmodel=large -no-pie)
	endif()

	add_include_dirs_of("ogg")
	
	# We build Vorbis. It must know the paths for OGG that we have just built.
	
	set(OGG_ROOT "${PROJECT_SOURCE_DIR}/src/3rdparty/ogg")
	set(OGG_LIBRARIES ogg)
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/vorbis")
	
	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(vorbis PUBLIC -mcmodel=large -no-pie)
	endif()

	add_include_dirs_of("vorbis")
endif()

if(BUILD_OPENAL)
	# We build OpenAL Soft. We need to set some variables beforehand.
	
	set(LIBTYPE "STATIC")
	set(FORCE_STATIC_VCRT TRUE CACHE BOOL "/MT for static VC runtimes" FORCE)
	
	# To avoid linker errors, we also need to tell OpenAL Soft we will be using it statically.

	add_definitions(-DAL_LIBTYPE_STATIC)

	set(ALSOFT_UTILS FALSE CACHE BOOL "Build and install utility programs" FORCE)
	set(ALSOFT_EXAMPLES FALSE CACHE BOOL "Build and install example programs" FORCE)
	set(ALSOFT_TESTS FALSE CACHE BOOL "Build and install test programs" FORCE)

	if(CLANG AND NOT MSVC)
		string(REPLACE " -Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endif()

	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/openal-soft")

	if(CLANG AND NOT MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	endif()

	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(OpenAL PUBLIC -mcmodel=large -no-pie)
	endif()

	add_include_dirs_of("OpenAL")
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/streflop")

set(BLAKE3_PATH "${PROJECT_SOURCE_DIR}/src/3rdparty/blake3")

IF (MSVC)
set(BLAKE3_SRC
    ${BLAKE3_PATH}/blake3.c
    ${BLAKE3_PATH}/blake3_portable.c
    ${BLAKE3_PATH}/blake3_dispatch.c
    ${BLAKE3_PATH}/blake3_avx2_x86-64_windows_gnu.S
    ${BLAKE3_PATH}/blake3_avx512_x86-64_windows_gnu.S
    ${BLAKE3_PATH}/blake3_sse41_x86-64_windows_gnu.S
    ${BLAKE3_PATH}/blake3_sse2_x86-64_windows_gnu.S
)
ELSEIF(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
set(BLAKE3_SRC
    ${BLAKE3_PATH}/blake3.c
    ${BLAKE3_PATH}/blake3_portable.c
    ${BLAKE3_PATH}/blake3_dispatch.c
    ${BLAKE3_PATH}/blake3_avx2_x86-64_unix.S
    ${BLAKE3_PATH}/blake3_avx512_x86-64_unix.S
    ${BLAKE3_PATH}/blake3_sse41_x86-64_unix.S
    ${BLAKE3_PATH}/blake3_sse2_x86-64_unix.S
)
ELSE()
set(BLAKE3_SRC
    ${BLAKE3_PATH}/blake3.c
    ${BLAKE3_PATH}/blake3_portable.c
    ${BLAKE3_PATH}/blake3_dispatch.c
)
ENDIF()

add_library(blake3 STATIC ${BLAKE3_SRC})

# We build the remaining libraries.

if(BUILD_NETWORKING)
	# add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/enet")

	#if(ADD_MCMODEL_LARGE_FLAG)
	#target_compile_options(enet PUBLIC -mcmodel=large -no-pie)
	#endif()
	
	#add_include_dirs_of("enet")
endif()

if(BUILD_FREETYPE)
	message("Building Freetype (no harfbuzz/png/bzip2/brotli/zlib).")
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/freetype2")

	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(freetype PUBLIC -mcmodel=large -no-pie)
	endif()

	add_include_dirs_of("freetype")
endif()


include_directories(${HYPERSOMNIA_INCLUDE_DIRS})

# We build the source code generators.


# We configure the C++ defines in accordance with the currently set options.

if(BUILD_OPENAL) 
	add_definitions(-DBUILD_OPENAL=1 -DAL_ALEXT_PROTOTYPES)
endif()

if(BUILD_OPENGL) 
	add_definitions(-DBUILD_OPENGL=1)
endif()

if(BUILD_OPENSSL) 
	add_definitions(-DBUILD_OPENSSL=1)
endif()

if(BUILD_SOUND_FORMAT_DECODERS) 
	add_definitions(-DBUILD_SOUND_FORMAT_DECODERS=1)
endif()

if(BUILD_NETWORKING) 
	add_definitions(-DBUILD_NETWORKING=1)
endif()

if(BUILD_FREETYPE) 
	add_definitions(-DBUILD_FREETYPE=1)
endif()

if(BUILD_UNIT_TESTS) 
	add_definitions(-DBUILD_UNIT_TESTS=1)
endif()

if(BUILD_TEST_SCENES) 
	add_definitions(-DBUILD_TEST_SCENES=1)
endif()

if(BUILD_WINDOW_FRAMEWORK)
	add_definitions(-DBUILD_WINDOW_FRAMEWORK=1)
endif()

if(USE_GLFW)
	add_definitions(-DUSE_GLFW=1)
endif()

if(BUILD_VERSION_FILE_GENERATOR) 
	add_definitions(-DWAS_VERSION_GENERATOR_BUILT=1)
endif()

if(ADD_APPLICATION_ICON)
	add_definitions(-DADD_APPLICATION_ICON)
endif()

if(BUILD_DEBUGGER_SETUP)
	add_definitions(-DBUILD_DEBUGGER_SETUP)
	add_definitions(-DBUILD_PROPERTY_DEBUGGER)
endif()

if(BUILD_STENCIL_BUFFER)
	add_definitions(-DBUILD_STENCIL_BUFFER)
endif()

# We configure additional user options for building the game.

if (STATICALLY_ALLOCATE_ENTITIES)
	add_definitions(-DSTATICALLY_ALLOCATE_ENTITIES=1)
endif()

if (STATICALLY_ALLOCATE_ENTITY_FLAVOURS)
	add_definitions(-DSTATICALLY_ALLOCATE_ENTITY_FLAVOURS=1)
endif()

add_definitions(-DBUILD_IN_CONSOLE_MODE=${BUILD_IN_CONSOLE_MODE})

add_definitions(-DDETAIL_DIR="${HYPERSOMNIA_DETAIL_DIR}")
add_definitions(-DOFFICIAL_CONTENT_DIR="${HYPERSOMNIA_OFFICIAL_CONTENT_DIR}")
add_definitions(-DGENERATED_FILES_DIR="${HYPERSOMNIA_GENERATED_FILES_DIR}")
add_definitions(-DUSER_FILES_DIR="${HYPERSOMNIA_USER_FILES_DIR}")
add_definitions(-DLOG_FILES_DIR="${HYPERSOMNIA_LOG_FILES_DIR}")

# We build the main Hypersomnia executable.


set(HYPERSOMNIA_CPPS
	${HYPERSOMNIA_VERSION_FILE_CPP}
	${HYPERSOMNIA_CODEBASE_CPPS}
	${HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS}
	# Box2d sources go to the very end because they build with speed of light
	${HYPERSOMNIA_BOX2D_CPPS}
)

set(HYPERSOMNIA_SRCS
	${HYPERSOMNIA_CPPS}
	${HYPERSOMNIA_RC_FILES}
	${HYPERSOMNIA_NATVIS_FILES}
	${HYPERSOMNIA_HEADERS}
)

if(MSVC)
	if(BUILD_IN_CONSOLE_MODE)
		add_executable(Hypersomnia ${HYPERSOMNIA_SRCS})
	else()
		add_executable(Hypersomnia WIN32 ${HYPERSOMNIA_SRCS})
	endif()
else()
	add_executable(Hypersomnia ${HYPERSOMNIA_SRCS})
endif()

# We define libraries upon which Hypersomnia depends.

set(HYPERSOMNIA_LIBS lua streflop blake3)

if(BUILD_OPENGL)
	find_package(OpenGL REQUIRED)

	target_link_libraries(Hypersomnia OpenGL::GL)
	target_include_directories(Hypersomnia PUBLIC OpenGL::GL)

	message("OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
endif()

if(BUILD_WINDOW_FRAMEWORK)
	if(USE_GLFW)
		message("Linking GLFW statically.")

		set(GLFW_ROOT "${PROJECT_SOURCE_DIR}/src/3rdparty/glfw")
		set(GLFW_HINTS "${GLFW_ROOT}/lib-macos")

		prefer_static_libraries()

		find_library(GLFW NAMES glfw3 HINTS ${GLFW_HINTS})
		list(APPEND HYPERSOMNIA_LIBS ${GLFW})
		message("FOUND GLFW LIB: ${GLFW}")

		prefer_shared_libraries()

		include_directories("${GLFW_ROOT}/include")
	elseif(UNIX)
		find_package(X11 REQUIRED)
		list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/extra-cmake-modules/find-modules")
		find_package(XCB REQUIRED COMPONENTS XCB GLX )

		prefer_static_libraries()
		find_package(XCB REQUIRED COMPONENTS KEYSYMS)
		prefer_shared_libraries()

		find_package(X11_XCB REQUIRED)

		include_directories(${X11_INCLUDE_DIR})
		include_directories(${XCB_INCLUDE_DIR})
		include_directories(${X11_XCB_INCLUDE_DIR})

		list(APPEND HYPERSOMNIA_LIBS ${X11_LIBRARIES})
		list(APPEND HYPERSOMNIA_LIBS ${XCB_LIBRARIES})
		list(APPEND HYPERSOMNIA_LIBS ${X11_XCB_LIBRARIES})
	endif()
endif()

if(BUILD_OPENAL)
	list(APPEND HYPERSOMNIA_LIBS OpenAL)
endif()

if(BUILD_NETWORKING)

endif()

if(BUILD_FREETYPE)
	list(APPEND HYPERSOMNIA_LIBS freetype)
endif()

if(BUILD_SOUND_FORMAT_DECODERS)
	list(APPEND HYPERSOMNIA_LIBS vorbisfile)
endif()

if(MSVC)
	if(BUILD_NETWORKING)
		message("Appending winsock libraries")

		list(APPEND HYPERSOMNIA_LIBS
			"Ws2_32.lib"
			"winmm.lib"
		)
	endif()
elseif(CLANG)
	if(USE_LIBCXX)
		if(STATIC_LINK_STDLIB)
			prefer_static_libraries()

			find_library(CPP c++)
			find_library(CPPFS c++fs)
			find_library(CPPEXPERIMENTAL c++experimental)
			find_library(CPPABI c++abi)

			if (NOT CPPFS)
				message("CPPFS not found. Linking via c++fs flag.")
				set(CPPFS c++fs)
			else()
				message("Static cppfs found.")
			endif()

			if (NOT CPPEXPERIMENTAL)
				message("CPPEXPERIMENTAL not found. Linking via c++experimental flag.")
				set(CPPEXPERIMENTAL c++experimental)
			else()
				message("Static cppexperimental found.")
			endif()

			set(HYPERSOMNIA_LIBS ${CPP} ${CPPABI} ${CPPFS} ${CPPEXPERIMENTAL} ${HYPERSOMNIA_LIBS})
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

			prefer_shared_libraries()
		else()
			if (CLANG_VERSION_STRING VERSION_LESS 9.0)
				message("Appending c++fs.")
				list(APPEND HYPERSOMNIA_LIBS c++fs)
			else()
				message("Omitting c++fs.")
			endif()
		endif()
	endif()
elseif(GCC)
	list(APPEND HYPERSOMNIA_LIBS stdc++fs)
endif()

if (UNIX)
	list(APPEND HYPERSOMNIA_LIBS dl)
endif()

if (BUILD_WINDOW_FRAMEWORK AND LINUX)
	list(APPEND HYPERSOMNIA_LIBS Xi)
endif()

if (BUILD_NETWORKING)
	if (MSVC)
		set(SODIUM_HINTS "${PROJECT_SOURCE_DIR}/src/3rdparty/yojimbo/windows")
	else()
		set(SODIUM_HINTS "")
	endif()

	prefer_static_libraries()

	find_library(SODIUM NAMES sodium HINTS ${SODIUM_HINTS})

	prefer_shared_libraries()

	list(APPEND HYPERSOMNIA_LIBS ${SODIUM})
endif()

if (BUILD_OPENSSL)
	message("Building OpenSSL.")

	# Static linking of SSL makes compilation on Linux a pain.
	# Let's do it for Windows and Apple only

	if (NOT LINUX)
		set(OPENSSL_USE_STATIC_LIBS TRUE)
	endif()

	if (MSVC)
		set(OPENSSL_MSVC_STATIC_RT TRUE)
	endif()

	find_package(OpenSSL REQUIRED)
	list(APPEND HYPERSOMNIA_LIBS OpenSSL::SSL OpenSSL::Crypto)

	if (MSVC)
		list(APPEND HYPERSOMNIA_LIBS crypt32.lib)
	endif()

	message("Found: ${OPENSSL_FOUND})")
	message("Include dir: ${OPENSSL_INCLUDE_DIR})")
	message("Crypto library: ${OPENSSL_CRYPTO_LIBRARY})")
	message("SSL library: ${OPENSSL_SSL_LIBRARY})")
	message("Libraries: ${OPENSSL_LIBRARIES})")
	message("Version: ${OPENSSL_VERSION})")
endif()

set_target_properties(Hypersomnia PROPERTIES DEBUG_POSTFIX "")

string(REPLACE ";" "\n" READABLE_ALL_LIBS_STR "${HYPERSOMNIA_LIBS}")
message("All libs:\n${READABLE_ALL_LIBS_STR}")

target_link_libraries(Hypersomnia ${HYPERSOMNIA_LIBS})

if(MSVC_SPECIFIC)
	set_target_properties(Hypersomnia PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Hypersomnia)
endif()

# We add a command to automatically create an archive for every Release build.

# Custom targets for Makefile
if(${CMAKE_GENERATOR} STREQUAL "Unix makefiles" OR ${CMAKE_GENERATOR} STREQUAL "Ninja")
	set(HYPERSOMNIA_WORKING_DIR ${HYPERSOMNIA_EXE_RESOURCES_DIR})

	add_custom_target(run
		COMMAND Hypersomnia --keep-cwd
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)

	add_custom_target(debug
		COMMAND cgdb $<TARGET_FILE:Hypersomnia>
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)

	set(CONQUE_DEBUG_ARGS
		"--remote-send" ":execute \\'ConqueGdbCommand run\\' \\<bar\\> ConqueGdb -cd ${HYPERSOMNIA_WORKING_DIR} -ex run --args $<TARGET_FILE:Hypersomnia>\\<CR\\> "	
	)

	add_custom_target(conque_debug
		COMMAND vim ${CONQUE_DEBUG_ARGS}
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)

	add_custom_target(memdeb
		COMMAND valgrind --log-file="memory_log.txt" --leak-check=yes $<TARGET_FILE:Hypersomnia>
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)

	add_custom_target(tests
		COMMAND Hypersomnia --keep-cwd --unit-tests-only --test-fp-consistency 20000000
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)
endif()	

get_target_property(OUT Hypersomnia LINK_LIBRARIES)
message("ALL linked libraries: ${OUT}")

message("All CXX flags: ${CMAKE_CXX_FLAGS}")
message("Debug CXX flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("RelWithDebInfo CXX flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message("Release CXX flags: ${CMAKE_CXX_FLAGS_RELEASE}")

message("ALl linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
message("Linker Debug flags: ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
message("Linker RelWithDebInfo flags: ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
message("Linker Release flags: ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
