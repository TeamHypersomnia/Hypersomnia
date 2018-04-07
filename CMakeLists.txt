cmake_minimum_required(VERSION 3.2)

project(Hypersomnia)

message("C: ${CMAKE_C_COMPILER}")
message("CXX: ${CMAKE_CXX_COMPILER}")

message("Hypersomnia project source dir: ${PROJECT_SOURCE_DIR}")
message("Hypersomnia project binary dir: ${PROJECT_BINARY_DIR}")
message("Hypersomnia current binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

# Determine the used compiler.
# In case of MSVC, CMake sets it to ON on its own.

# Important note:
# UNIX build is currently hardcoded to be dynamically linked.
# MSVC build is currently hardcoded to be statically linked.

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message("Building with Clang.")
	set(CLANG ON)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("Building with gcc.")
	set(GCC ON)
endif()

if(UNIX)
	message("Building for UNIX systems.")

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

# Various feature switches.
# If it is your first time building Hypersomnia and you get some errors,
# try turning all of these off to build the minimal possible working Hypersomnia runtime. Then, try to work from there.
# The game should still run, even not having these features, although obviously, you won't see or hear anything.
# In the future, the switches will also differ depending on whether the target application is a client or a server.
# If you're a developer, you might also want to disable some of these to have faster builds for quicker iteration.

if(HYPERSOMNIA_BARE)
	message("Building Hypersomnia without... anything")
	
	set(DEFAULT_OPT OFF)
else()
	set(DEFAULT_OPT ON)
endif()

# Building switches for functionality using 3rdparty libraries.

option(BUILD_OPENAL "Build OpenAL Soft." ${DEFAULT_OPT})
option(BUILD_OPENGL "Build OpenGL-related code." ${DEFAULT_OPT})
option(BUILD_ENET "Build ENet networking library." ${DEFAULT_OPT})
option(BUILD_FREETYPE "Build FreeType library responsible for loading fonts." ${DEFAULT_OPT})
option(BUILD_WINDOW_FRAMEWORK "Build code specific to a given platfom, e.g. window management done by WinAPI." ${DEFAULT_OPT})
option(BUILD_HTTP_REQUESTS "Build simple http requests, e.g. for latest news bar in the main menu." ${DEFAULT_OPT})
option(BUILD_UNIT_TESTS "Build unit tests that are run on game startup." ${DEFAULT_OPT})
option(BUILD_SOUND_FORMAT_DECODERS "Build ogg and vorbis. If this is off, there is also no point in enabling OpenAL." ${DEFAULT_OPT})

# Switches for functionality originating from the Hypersomnia codebase.
option(BUILD_VERSION_FILE_GENERATOR "Build version file generator to generate commit information." ${DEFAULT_OPT})
option(BUILD_TEST_SCENES "Build unscripted test scenes hardcoded in C++." ${DEFAULT_OPT})
option(BUILD_PROPERTY_EDITOR "Build property editor code for the editor setup. Due to hardcore templates, this takes amazingly long time to build, so it makes sense to turn it off sometimes." ${DEFAULT_OPT})

# Various other switches.
option(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE "Add command to archive builds in Release." ${DEFAULT_OPT})
option(ADD_APPLICATION_ICON "Add application icon." ${DEFAULT_OPT})

# Optimization switches.
option(ENABLE_LINK_TIME_OPTIMIZATION "Some ultra extra optimizations for Release builds. Takes a lot longer to build." OFF)

# Other variables.

# Where the game will save logs.
# Deleting this folder will have no effect on the game.

set(HYPERSOMNIA_LOG_FILES_DIR "cache/log/")

# Where the game will save neon maps and other regenerable content.
# Deleting this folder will have no effect on the game, except that some resources might take time to generate again.

set(HYPERSOMNIA_GENERATED_FILES_DIR "cache/gen/")

# Where the game will save untitled works from editor, gui layouts, user config etc.
# Deleting this folder might possibly result in data loss, e.g. the untitled works and some user settings.

set(HYPERSOMNIA_LOCAL_FILES_DIR "cache/usr/")

# If this variable is nonzero, the cosmos will use a statically allocated number
# of entities, drastically improving access performance. Might be very useful for a sever application.
# Pros: 
# 	+ Dereferencing entities with speed of light
# Cons:
#	- Compilation might be longer due to component includes needed everywhere
#	- Can't tweak maximum number of entities at runtime
#	- Even empty game worlds occupy as much space as those filled to the maximum
set(STATICALLY_ALLOCATE_ENTITIES "1" CACHE STRING "Statically allocate entities in the cosmos")

# If this variable is nonzero, the cosmos common will use a statically allocated number
# of entity flavours, drastically improving access performance. Might be very useful for a sever application.
# Pros: 
# 	+ Operating on entities with speed of light
# Cons:
#	- Compilation might be longer due to component includes needed everywhere
#	- Can't tweak maximum number of entity flavours at runtime
#	- Even empty game worlds occupy as much space as those filled to the maximum
set(STATICALLY_ALLOCATE_ENTITY_FLAVOURS "1" CACHE STRING "Statically allocate entity flavours in the cosmos common")

set(STATICALLY_ALLOCATE_ASSETS "1" CACHE STRING "Statically allocate assets using container_sizes.h")
set(STATICALLY_ALLOCATE_BAKED_FONTS "1" CACHE STRING "Statically allocate fonts for faster rendering. One font weights 3 MB.")

set(BUILD_IN_CONSOLE_MODE OFF CACHE STRING "0")

if(UNIX)
	# Set different default options for UNIX:
	# Disable those that are not yet supported

	message("Changing unsupported UNIX defaults to off...")
	
	set(BUILD_HTTP_REQUESTS OFF)
	set(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE OFF)
endif()

# Here we set some internal flags for programmers' use. 
# Don't set them manually without a good reason.

if (MSVC OR CLANG) 
	set(ADD_MCMODEL_LARGE_FLAG OFF CACHE STRING "Needed to suppress some crazy linker errors")
elseif (GCC)
	set(ADD_MCMODEL_LARGE_FLAG ON CACHE STRING "Needed to suppress some crazy linker errors")
endif()

set (BUILD_LUA_AS_CPP OFF)
# End of internal flags

if(NOT BUILD_WINDOW_FRAMEWORK)
	set(BUILD_IN_CONSOLE_MODE "1")
endif()

set(BUILD_GLAD ${BUILD_OPENGL})

if(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE)
	if (NOT HYPERSOMNIA_RUNTIME_ARCHIVE_DIR)
		# By default, we set output directory for archives to completely outside of the source tree,
		# because it might be useful to preserve some older builds even if the repository is deleted.
	
		set(HYPERSOMNIA_RUNTIME_ARCHIVE_DIR
			"${PROJECT_SOURCE_DIR}/../build_database"
		)
	
		file(MAKE_DIRECTORY ${HYPERSOMNIA_RUNTIME_ARCHIVE_DIR})

		message("Built Hypersomnia archives will be copied to ${HYPERSOMNIA_RUNTIME_ARCHIVE_DIR}")
	endif()
endif()

# Used by archiver and to set the working directory under VS.

set(RESOURCES_FOLDER_NAME "hypersomnia")
set(HYPERSOMNIA_EXE_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/${RESOURCES_FOLDER_NAME}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

if(BUILD_LUA_AS_CPP)
	set_source_files_properties(${LUA_CPPS} PROPERTIES LANGUAGE CXX)
endif()

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
	"src/test_scenes/ingredients/wsad_player.cpp"
	"src/test_scenes/ingredients/grenades.cpp"
	
	"src/test_scenes/test_scene_images.cpp"
	"src/test_scenes/test_scene_particle_effects.cpp"
	"src/test_scenes/test_scene_physical_materials.cpp"
	"src/test_scenes/test_scene_sentience_properties.cpp"
	"src/test_scenes/test_scene_animations.cpp"
	"src/test_scenes/test_scene_sound_buffers.cpp"
	"src/test_scenes/test_scene_recoil_players.cpp"

	"src/test_scenes/test_scene_flavours.cpp"
	"src/test_scenes/scenes/minimal_scene.cpp"

	"src/test_scenes/scenes/minimal_scene.cpp"
	"src/test_scenes/scenes/testbed.cpp"
)

# We don't care about alphabetical order. Put new files to the very bottom.

set(HYPERSOMNIA_CODEBASE_CPPS
	# This should be the first one to build to catch errors early
	"src/game/tests_of_traits.cpp"

	"src/application/config_lua_table.cpp"
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
	"src/augs/misc/http/http_requests.cpp"
	"src/augs/misc/readable_bytesize.cpp"
	"src/augs/misc/action_list/standard_actions.cpp"
	"src/augs/readwrite/memory_stream.cpp"
	"src/augs/misc/time_utils.cpp"
	"src/augs/misc/typesafe_sprintf.cpp"
	"src/augs/misc/typesafe_sscanf.cpp"
	"src/augs/network/network_client.cpp"
	"src/augs/network/network_types.cpp"
	"src/augs/network/network_server.cpp"
	"src/augs/network/enet_raii.cpp"
	"src/augs/network/reliable_channel.cpp"
	"src/augs/texture_atlas/texture_atlas_entry.cpp"
	"src/augs/texture_atlas/texture_atlas.cpp"
	"src/game/assets/animation.cpp"
	"src/game/assets/behaviour_tree.cpp"
	"src/view/viewables/regeneration/image_loadables.cpp"
	"src/view/viewables/particle_effect.cpp"
	"src/game/assets/physical_material.cpp"
	"src/game/components/flags_component.cpp"
	"src/game/components/light_component.cpp"
	"src/game/components/rigid_body_component.cpp"
	"src/game/components/sentience_component.cpp"
	"src/game/detail/explosions.cpp"
	"src/game/detail/hand_fuse_logic.cpp"
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
	"src/game/detail/inventory/inventory_slot.cpp"
	"src/game/detail/inventory/inventory_slot_id.cpp"
	"src/game/detail/inventory/inventory_utils.cpp"
	"src/view/viewables/particle_types.cpp"
	"src/game/detail/physics/contact_listener.cpp"
	"src/game/detail/physics/physics_friction_fields.cpp"
	"src/game/detail/physics/ray_casts.cpp"
	"src/game/detail/physics/physics_scripts.cpp"
	"src/augs/misc/value_meter.cpp"
	"src/game/detail/visible_entities.cpp"
	"src/game/detail/inventory/wielding_result.cpp"
	"src/game/enums/attitude_type.cpp"
	"src/game/enums/item_category.cpp"
	"src/game/enums/item_transfer_result_type.cpp"
	"src/game/enums/slot_physical_behaviour.cpp"
	"src/game/detail/ai/create_standard_behaviour_trees.cpp"
	"src/view/audiovisual_state/systems/exploding_ring_system.cpp"
	"src/view/game_gui/game_gui_system.cpp"
	"src/view/audiovisual_state/systems/interpolation_system.cpp"
	"src/view/audiovisual_state/systems/particles_simulation_system.cpp"
	"src/view/audiovisual_state/systems/past_infection_system.cpp"
	"src/view/audiovisual_state/systems/pure_color_highlight_system.cpp"
	"src/view/audiovisual_state/systems/sound_system.cpp"
	"src/view/audiovisual_state/systems/thunder_system.cpp"
	"src/view/audiovisual_state/systems/flying_number_indicator_system.cpp"
	"src/view/audiovisual_state/systems/wandering_pixels_system.cpp"
	"src/game/inferred_caches/physics_world_cache.cpp"
	"src/game/inferred_caches/tree_of_npo_cache.cpp"
	"src/game/stateless_systems/destruction_system.cpp"
	"src/game/stateless_systems/hand_fuse_system.cpp"
	"src/view/audiovisual_state/systems/light_system.cpp"
	"src/game/stateless_systems/melee_system.cpp"
	"src/game/stateless_systems/particles_existence_system.cpp"
	"src/game/stateless_systems/sound_existence_system.cpp"
	"src/game/other_unit_tests.cpp"
	"src/game/transcendental/cosmic_entropy.cpp"
	"src/game/transcendental/cosmic_movie_director.cpp"
	"src/game/transcendental/data_living_one_step.cpp"
	"src/augs/filesystem/directory.cpp"
	"src/augs/gui/appearance_detector.cpp"
	"src/augs/misc/timing/delta.cpp"
	"src/augs/misc/timing/stepped_timing.cpp"
	"src/augs/window_framework/platform_utils.cpp"
	"src/game/components/animation_component.cpp"
	"src/game/components/car_component.cpp"
	"src/game/components/container_component.cpp"
	"src/game/components/missile_component.cpp"
	"src/game/components/fixtures_component.cpp"
	"src/game/components/item_component.cpp"
	"src/game/components/item_slot_transfers_component.cpp"
	"src/game/components/movement_component.cpp"
	"src/game/components/pathfinding_component.cpp"
	"src/game/transcendental/cosmos.cpp"
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
	"src/game/transcendental/cosmos_solvable_significant.cpp"
	"src/view/network/simulation_broadcast.cpp"
	# "src/view/network/simulation_receiver.cpp"
	"src/game/stateless_systems/animation_system.cpp"
	"src/game/stateless_systems/behaviour_tree_system.cpp"
	"src/game/stateless_systems/car_system.cpp"
	"src/game/stateless_systems/crosshair_system.cpp"
	"src/game/stateless_systems/missile_system.cpp"
	"src/game/stateless_systems/destroy_system.cpp"
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
	"src/view/rendering_scripts/draw_circular_bars.cpp"
	"src/view/rendering_scripts/draw_cast_spells_highlights.cpp"
	"src/view/rendering_scripts/draw_crosshair_lasers.cpp"
	"src/view/rendering_scripts/draw_hud_for_unpinned_explosives.cpp"
	"src/view/audiovisual_state/world_camera.cpp"
	"src/application/setups/test_scene_setup.cpp"
	"src/application/setups/editor/editor_setup.cpp"
	"src/application/setups/editor/editor_folder.cpp"
	"src/application/setups/editor/editor_recent_paths.cpp"
	"src/application/setups/main_menu_setup.cpp"
	"src/application/web_daemon/session_report.cpp"
	"src/augs/global_libraries.cpp"
	"src/augs/graphics/fbo.cpp"
	"src/augs/graphics/rgba.cpp"
	"src/augs/graphics/renderer.cpp"
	"src/augs/graphics/shader.cpp"
	"src/augs/graphics/vertex.cpp"
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
	"src/augs/templates/string_templates.cpp"
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
	"src/view/necessary_resources.cpp"
	"src/augs/misc/imgui/imgui_utils.cpp"
	"src/augs/misc/imgui/imgui_control_wrappers.cpp"
	"src/application/gui/settings_gui.cpp"
	"src/game/assets/recoil_player.cpp"
	"src/augs/gui/bbcode.cpp"
	"src/augs/unit_tests.cpp"
	"src/game/transcendental/cosmic_profiler.cpp"
	"src/view/audiovisual_state/audiovisual_state.cpp"
	"src/view/viewables/loaded_sounds_map.cpp"
	"src/view/viewables/atlas_distributions.cpp"
	"src/application/main/draw_debug_details.cpp"
	"src/application/main/imgui_pass.cpp"
	"src/application/main/release_flags.cpp"
	"src/game/debug_drawing_settings.cpp"
	"src/application/session_profiler.cpp"
	"src/view/viewables/all_viewables_defs.cpp"
	"src/view/audiovisual_state/audiovisual_profiler.cpp"
	"src/application/intercosm.cpp"
	"src/augs/misc/imgui/addons/imguitabwindow/imguitabwindow.cpp"
	"src/augs/misc/imgui/imgui_controls.cpp"
	"src/augs/readwrite/readwrite_tests.cpp"
	"src/game/components/trace_component.cpp"
	"src/game/stateless_systems/physics_system.cpp"
	"src/game/transcendental/standard_solver.cpp"
	"src/game/transcendental/cosmos_solvable.cpp"
	"src/game/transcendental/cosmos_common.cpp"
	"src/game/detail/inventory/perform_transfer.cpp"
	"src/game/transcendental/cosmic_functions.cpp"
	"src/game/detail/view_input/particle_effect_input.cpp"
	"src/game/detail/view_input/sound_effect_input.cpp"
	"src/view/viewables/image_structs.cpp"
	"src/game/detail/spells/spell_logic_input.cpp"
	"src/game/inferred_caches/processing_lists_cache.cpp"
	"src/game/transcendental/entity_id.cpp"
	"src/augs/misc/children_vector_tracker.cpp"
	"src/view/frame_profiler.cpp"
	"src/application/setups/editor/editor_autosave.cpp"
	"src/application/setups/editor/editor_paths.cpp"
	"src/augs/templates/container_templates.cpp"
	"src/application/setups/editor/editor_history.cpp"
	"src/augs/templates/history.cpp"
	"src/game/transcendental/state_tests.cpp"
	"src/build_info.cpp"
	"src/augs/misc/pool/pool.cpp"
	"src/application/setups/editor/gui/editor_history_gui.cpp"
	"src/application/setups/editor/gui/editor_go_to_gui.cpp"
	"src/application/setups/editor/editor_camera.cpp"
	"src/application/setups/editor/editor_popup.cpp"
	"src/application/setups/editor/editor_command_input.cpp"
	"src/application/setups/editor/commands/fill_with_test_scene_command.cpp"
	"src/application/setups/editor/commands/delete_entities_command.cpp"
	"src/application/setups/editor/commands/change_property_command.cpp"
	"src/application/setups/editor/gui/editor_all_entities_gui.cpp"
	"src/augs/templates/introspect.cpp"
	"src/application/setups/editor/gui/editor_common_state_gui.cpp"
	"src/application/setups/editor/commands/move_entities_command.cpp"
	"src/application/setups/editor/gui/editor_entity_selector.cpp"
	"src/application/setups/editor/property_editor/fae_tree_structs.cpp"
	"src/augs/math/snapping_grid.cpp"
	"src/application/setups/editor/editor_view.cpp"
	"src/game/detail/sentience_shake.cpp"
	"src/application/setups/editor/commands/paste_entities_command.cpp"
	"src/application/setups/editor/commands/duplicate_entities_command.cpp"
	"src/application/setups/editor/commands/change_grouping_command.cpp"
	"src/application/setups/editor/editor_selection_groups.cpp"
	"src/application/setups/editor/gui/editor_selection_groups_gui.cpp"
	"src/application/setups/editor/gui/editor_summary_gui.cpp"
	"src/application/setups/editor/gui/editor_entity_mover.cpp"
)

if(BUILD_WINDOW_FRAMEWORK)
	if(MSVC)
		list(APPEND HYPERSOMNIA_CODEBASE_CPPS
			"src/augs/window_framework/window_winapi.cpp"
			"src/augs/window_framework/translate_winapi_enums.cpp"
		)
	elseif(UNIX)
		list(APPEND HYPERSOMNIA_CODEBASE_CPPS
			"src/augs/window_framework/window_x.cpp"
			"src/augs/window_framework/translate_x_enums.cpp"
		)
	endif()
else()
	list(APPEND HYPERSOMNIA_CODEBASE_CPPS
		"src/augs/window_framework/window_stubs.cpp"
	)
endif()

# natvis files for easier debugging in Visual Studio.

if(MSVC)
	set(HYPERSOMNIA_NATVIS_FILES
		"src/game/transcendental/entity_handle.natvis"
		"src/game/detail/inventory/inventory_slot_handle.natvis"
		"src/augs/misc/pool/pooled_object_id.natvis"
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

# The rest of 3rdparty libraries with minimal amount of source files.

set(HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS
	"src/3rdparty/lodepng/lodepng.cpp"
	"src/3rdparty/polypartition/src/polypartition.cpp"
	"src/3rdparty/rectpack2D/src/pack.cpp"
	"src/3rdparty/imgui/imgui.cpp"
	"src/3rdparty/imgui/imgui_draw.cpp"
	"src/3rdparty/imgui/imgui_demo.cpp"
)

if(BUILD_GLAD)
	list(APPEND HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS
		"src/3rdparty/glad/glad.c"
	)
endif()

# Project resource files, e.g. an icon.

if(ADD_APPLICATION_ICON)
	if(MSVC)
		set(HYPERSOMNIA_RC_FILES
			"${CMAKE_MODULE_PATH}/Hypersomnia.rc"
		)
	endif()
endif()

# We arrange the generated sources paths.

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

file(GLOB_RECURSE HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES
    "src/hypersomnia_version.h"
	"src/augs/*.h"
	"src/game/*.h"
	"src/view/*.h"
	"src/application/*.h"
	"src/3rdparty/Box2D/Common/*.h"
	"src/3rdparty/Box2D/Dynamics/*.h"
	"src/test_scenes/test_scene_flavours.h"
	"src/test_scenes/test_scene_settings.h"

	"src/game/tests_of_traits.cpp"
	"src/augs/templates/introspect.cpp"
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
	set(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE FALSE)
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
	"${PROJECT_SOURCE_DIR}/src/3rdparty/sol2/single/sol"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/imgui"
)

if(BUILD_UNIT_TESTS)
	list(APPEND HYPERSOMNIA_INCLUDE_DIRS
		"${PROJECT_SOURCE_DIR}/src/3rdparty/Catch/single_include"
	)
endif()

# We configure additional flags for the compiler.
if(GCC OR CLANG)
	# Architecture switches
	if(${ARCHITECTURE} STREQUAL "x64")
		set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -m64")
		set(HYPERSOMNIA_C_FLAGS "${HYPERSOMNIA_C_FLAGS} -m64")
	endif()

	# Generate debug information (we don't care about C libraries at this point)
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -g")

	message("-O0 will be specified for Debug build.")
	set(HYPERSOMNIA_CXX_DEBUG_FLAGS "${HYPERSOMNIA_CXX_DEBUG_FLAGS} -O0")
	set(HYPERSOMNIA_C_DEBUG_FLAGS "${HYPERSOMNIA_C_DEBUG_FLAGS} -O0")
endif()

if(CLANG)
	set(CHOSEN_WARNINGS " ")
	set(WARNINGS_FOR_OUR_CODE_ONLY " ")

	# Latest features of C++17 will be enabled.
	# Looks like stdlib=libc++ is duplicated for the linker as well,
	# so we only have to add it to CXX.
	set(HYPERSOMNIA_CXX_FLAGS "-std=gnu++1z -stdlib=libc++ ${CHOSEN_WARNINGS}")
	set_source_files_properties(${HYPERSOMNIA_CODEBASE_CPPS} PROPERTIES COMPILE_FLAGS ${WARNINGS_FOR_OUR_CODE_ONLY})
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

	set(WARNINGS_FOR_OUR_CODE_ONLY 
"-Wunused-variable \
-Wuninitialized \
-Wmaybe-uninitialized"
	)

	set(HYPERSOMNIA_CXX_FLAGS "-std=gnu++1z ${CHOSEN_WARNINGS}")
	set_source_files_properties(${HYPERSOMNIA_CODEBASE_CPPS} PROPERTIES COMPILE_FLAGS ${WARNINGS_FOR_OUR_CODE_ONLY})
endif()

if(MSVC)
	# Latest features of C++17 will be enabled.
	set(HYPERSOMNIA_CXX_FLAGS "/std:c++latest /MP /Gm- /fp:strict /bigobj /permissive-")
	set(HYPERSOMNIA_CXX_DEBUG_FLAGS " ")
	set(HYPERSOMNIA_CXX_RELEASE_FLAGS "/Ob2 /GL /Oi")
	set(HYPERSOMNIA_CXX_RELWITHDEBINFO_FLAGS "/Ob2 /GL /Oi")
endif()

if(CLANG)
	if(ENABLE_LINK_TIME_OPTIMIZATION)
		set(HYPERSOMNIA_CXX_RELEASE_FLAGS "-flto -fwhole-program-vtables")
	endif()
endif()

if(ADD_MCMODEL_LARGE_FLAG)
	message("-mcmodel=large -no-pie flag will be specified.")
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} -mcmodel=large -no-pie")
endif()

if(UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

# We configure additional flags for linker.

if (CLANG)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
endif()

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if(CLANG)
	if(ENABLE_LINK_TIME_OPTIMIZATION)
		set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "-flto -fwhole-program-vtables")
	endif()
elseif(MSVC)
	set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "/LTCG /INCREMENTAL:NO /OPT:REF")

	# We enable incremental LTCG for at least one release configuration,
	# because LTCG takes very long even with just a single source file modified.

	set(HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS "/LTCG:incremental /INCREMENTAL:NO")
endif()

# We configure some preprocessor defines.

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

if(UNIX)
    add_definitions(-DPLATFORM_UNIX=1)
elseif(MSVC)
	add_definitions(-DPLATFORM_WINDOWS=1)

	add_definitions(
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS
		-D_SCL_SECURE_NO_WARNINGS
		-DUNICODE
	  )
endif()

# Additionally, we mark Release as the production build.

set(HYPERSOMNIA_CXX_RELEASE_FLAGS
 "${HYPERSOMNIA_CXX_RELEASE_FLAGS} -DIS_PRODUCTION_BUILD=1"
)

# We apply the variables to the actual flags.

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

message("All CXX flags: ${CMAKE_CXX_FLAGS}")
message("Debug CXX flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("Release CXX flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message("ALl linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
message("Linker debug flags: ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
message("Linker release flags: ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

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
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/openal-soft")

	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(OpenAL PUBLIC -mcmodel=large -no-pie)
	endif()

	add_include_dirs_of("OpenAL")
endif()

# We build the remaining libraries.

if(BUILD_ENET)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/enet")

	if(ADD_MCMODEL_LARGE_FLAG)
		target_compile_options(enet PUBLIC -mcmodel=large -no-pie)
	endif()
	
	add_include_dirs_of("enet")
endif()

if(BUILD_FREETYPE)
	set(WITH_ZLIB OFF)
	set(WITH_BZip2 OFF)
	set(WITH_PNG OFF)
	set(WITH_HarfBuzz OFF)
	set(FREETYPE_NO_DIST ON)

	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/freetype")
	
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

if(BUILD_SOUND_FORMAT_DECODERS) 
	add_definitions(-DBUILD_SOUND_FORMAT_DECODERS=1)
endif()

if(BUILD_ENET) 
	add_definitions(-DBUILD_ENET=1)
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

if(BUILD_HTTP_REQUESTS) 
	add_definitions(-DBUILD_HTTP_REQUESTS=1)
endif()

if(BUILD_VERSION_FILE_GENERATOR) 
	add_definitions(-DWAS_VERSION_GENERATOR_BUILT=1)
endif()

if(ADD_APPLICATION_ICON)
	add_definitions(-DADD_APPLICATION_ICON)
endif()

if(BUILD_PROPERTY_EDITOR)
	add_definitions(-DBUILD_PROPERTY_EDITOR)
endif()

# We configure additional user options for building the game.

add_definitions(-DSTATICALLY_ALLOCATE_ENTITIES=${STATICALLY_ALLOCATE_ENTITIES})
add_definitions(-DSTATICALLY_ALLOCATE_ENTITY_FLAVOURS=${STATICALLY_ALLOCATE_ENTITY_FLAVOURS})
add_definitions(-DSTATICALLY_ALLOCATE_ASSETS=${STATICALLY_ALLOCATE_ASSETS})
add_definitions(-DSTATICALLY_ALLOCATE_BAKED_FONTS=${STATICALLY_ALLOCATE_BAKED_FONTS})
add_definitions(-DBUILD_IN_CONSOLE_MODE=${BUILD_IN_CONSOLE_MODE})

add_definitions(-DGENERATED_FILES_DIR="${HYPERSOMNIA_GENERATED_FILES_DIR}")
add_definitions(-DLOCAL_FILES_DIR="${HYPERSOMNIA_LOCAL_FILES_DIR}")
add_definitions(-DLOG_FILES_DIR="${HYPERSOMNIA_LOG_FILES_DIR}")

# We build the main Hypersomnia executable.


set(HYPERSOMNIA_CPPS
	"src/main.cpp"
	"src/help_section.cpp"
	
	${HYPERSOMNIA_TRIVIAL_3RDPARTY_CPPS}
	${HYPERSOMNIA_BOX2D_CPPS}
	${HYPERSOMNIA_CODEBASE_CPPS}
	${HYPERSOMNIA_VERSION_FILE_CPP}
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

set(HYPERSOMNIA_LIBS lua)

if(BUILD_OPENGL)
	message("OPENGL_INCLUDE_DIRS before finding: ${OPENGL_INCLUDE_DIRS}")
	message("OPENGL_LIBRARIES before finding: ${OPENGL_LIBRARIES}")
	find_package(OpenGL REQUIRED)
	message("OPENGL_INCLUDE_DIRS after finding: ${OPENGL_INCLUDE_DIRS}")
	message("OPENGL_LIBRARIES after finding: ${OPENGL_LIBRARIES}")

	include_directories(${OPENGL_INCLUDE_DIRS})

	list(APPEND HYPERSOMNIA_LIBS
		${OPENGL_LIBRARIES}
	)
endif()

if(BUILD_WINDOW_FRAMEWORK)
	if(UNIX)
		find_package(X11 REQUIRED)
		list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/extra-cmake-modules/find-modules")
		find_package(XCB REQUIRED COMPONENTS XCB GLX KEYSYMS)
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

if(BUILD_ENET)
	list(APPEND HYPERSOMNIA_LIBS enet)
endif()

if(BUILD_FREETYPE)
	list(APPEND HYPERSOMNIA_LIBS freetype)
endif()

if(BUILD_SOUND_FORMAT_DECODERS)
	list(APPEND HYPERSOMNIA_LIBS vorbisfile)
endif()

if(MSVC)
	if(BUILD_ENET OR BUILD_HTTP_REQUESTS)
		message("Appending winsock libraries")

		list(APPEND HYPERSOMNIA_LIBS
			"Ws2_32.lib" 		# for ENet
			"winmm.lib"  		# for ENet
		)
	endif()
endif()

if(CLANG)
	list(APPEND HYPERSOMNIA_LIBS c++experimental)
elseif(GCC)
	list(APPEND HYPERSOMNIA_LIBS stdc++fs)
endif()

target_link_libraries(Hypersomnia ${HYPERSOMNIA_LIBS})

set_target_properties(Hypersomnia PROPERTIES DEBUG_POSTFIX "")

if(MSVC)
	set_target_properties(Hypersomnia PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
	
#	set_target_properties(Hypersomnia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
#	set_target_properties(Hypersomnia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
#	set_target_properties(Hypersomnia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
#	set_target_properties(Hypersomnia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${HYPERSOMNIA_EXE_RESOURCES_DIR}")
	
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Hypersomnia)
endif()

# We add a command to automatically create an archive for every Release build.

if(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE)
	add_custom_command(
		TARGET Hypersomnia
		POST_BUILD
		COMMAND ${CMAKE_COMMAND}
			-DCONFIG=$<CONFIGURATION>
			-DEXE_PATH=$<TARGET_FILE:Hypersomnia>
			-DARCHIVE_OUTPUT_DIR=${HYPERSOMNIA_RUNTIME_ARCHIVE_DIR}
			-DRESOURCES_DIR=${HYPERSOMNIA_EXE_RESOURCES_DIR}
			-DPYTHON_EXE=${PYTHON_EXECUTABLE}
			-DGIT_EXE=${GIT_EXECUTABLE}
			-DCREATE_ARCHIVE_PY_PATH=${CMAKE_MODULE_PATH}/create_archive.py.in
			-DRESOURCES_FOLDER_NAME=${RESOURCES_FOLDER_NAME}
			-P ${CMAKE_MODULE_PATH}/create_build_archive.cmake
		DEPENDS ${CMAKE_MODULE_PATH}/create_build_archive.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Archiving the executable and resources..."
		VERBATIM
	)
endif()

# Custom targets for Makefile
if(UNIX)
	set(HYPERSOMNIA_WORKING_DIR ${HYPERSOMNIA_EXE_RESOURCES_DIR})

	add_custom_target(run
		COMMAND Hypersomnia
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
		COMMAND Hypersomnia --unit-tests-only
		DEPENDS Hypersomnia
		WORKING_DIRECTORY ${HYPERSOMNIA_WORKING_DIR} 
	)
endif()	
