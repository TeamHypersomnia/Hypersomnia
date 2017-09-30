cmake_minimum_required(VERSION 3.8)

project(Hypersomnia)

# Various feature switches.
# If it is your first time building Hypersomnia and you get some errors,
# try turning all of these off to build the minimal possible working Hypersomnia runtime. Then, try to work from there.
# The game should still run, even not having these features, although obviously, you won't see/hear anything.
# In the future, the switches will also differ depending on whether the target application is a client or a server.

option(BUILD_SOUND_FORMAT_DECODERS "Build ogg and vorbis. If this is off, there is also no point in enabling OpenAL." ON)
option(BUILD_OPENAL "Build OpenAL Soft." ON)
option(BUILD_OPENGL "Build OpenGL-related code." ON)
option(BUILD_UNIT_TESTS "Build unit tests that are run on game startup." ON)
option(BUILD_VERSION_FILE_GENERATOR "Build version file generator to generate commit information." ON)
option(BUILD_ENET "Build ENet networking library." ON)
option(BUILD_FREETYPE "Build FreeType library responsible for loading fonts." ON)
option(BUILD_HTTP_REQUESTS "Build simple http requests, e.g. for latest news bar in the main menu." ON)
option(BUILD_TEST_SCENES "Build unscripted test scenes hardcoded in C++." ON)
option(BUILD_PLATFORM_CODE "Build code specific to a given platfom, e.g. window management done by WinAPI." ON)
option(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE "Add command to archive builds in Release." ON)
option(ADD_APPLICATION_ICON "Add application icon." ON)

option(HYPERSOMNIA_BARE "Build the minimal possible Hypersomnia runtime. For testing or quick iteration." OFF)

if (HYPERSOMNIA_BARE)
	set(BUILD_SOUND_FORMAT_DECODERS FALSE CACHE BOOL "" FORCE)
	set(BUILD_OPENAL FALSE CACHE BOOL "" FORCE)
	set(BUILD_OPENGL FALSE CACHE BOOL "" FORCE)
	set(BUILD_UNIT_TESTS FALSE CACHE BOOL "" FORCE)
	set(BUILD_VERSION_FILE_GENERATOR FALSE CACHE BOOL "" FORCE)
	set(BUILD_ENET FALSE CACHE BOOL "" FORCE)
	set(BUILD_FREETYPE FALSE CACHE BOOL "" FORCE)
	set(BUILD_HTTP_REQUESTS FALSE CACHE BOOL "" FORCE)
	set(BUILD_TEST_SCENES FALSE CACHE BOOL "" FORCE)
	set(BUILD_PLATFORM_CODE FALSE CACHE BOOL "" FORCE)
	set(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE FALSE CACHE BOOL "" FORCE)
	set(ADD_APPLICATION_ICON FALSE CACHE BOOL "" FORCE)

	message("Building Hypersomnia without... anything")
endif()

if(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE)
	if (NOT HYPERSOMNIA_RUNTIME_ARCHIVE_DIR)
		# By default, we set output directory for archives to completely outside of the source tree,
		# because it might be useful to preserve some older builds even if the repository is deleted.
	
		set(HYPERSOMNIA_RUNTIME_ARCHIVE_DIR
			"${PROJECT_SOURCE_DIR}/../build_database"
		)
	
		file(MAKE_DIRECTORY ${HYPERSOMNIA_RUNTIME_ARCHIVE_DIR})
	endif()
endif()

# Used by archiver and to set the working directory under VS.

set(RESOURCES_FOLDER_NAME "hypersomnia")
set(HYPERSOMNIA_EXE_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/${RESOURCES_FOLDER_NAME}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# We will be building Lua manually as there is no up-to-date CMakeLists for Lua.

set(LUA_SRCS
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

set_source_files_properties(${LUA_SRCS} PROPERTIES LANGUAGE CXX)

# We will be building Box2D manually, as it is hacked to the utmost.

set(HYPERSOMNIA_BOX2D_SRCS
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

# Test scene sources that whose inclusion is conditional on the value of BUILD_TEST_SCENES.

set(HYPERSOMNIA_TEST_SCENES_SRCS
	"src/test_scenes/ingredients/artificial_life.cpp"
	"src/test_scenes/ingredients/backpack.cpp"
	"src/test_scenes/ingredients/box_physics.cpp"
	"src/test_scenes/ingredients/car.cpp"
	"src/test_scenes/ingredients/character_inventory.cpp"
	"src/test_scenes/ingredients/guns.cpp"
	"src/test_scenes/ingredients/ingredients.cpp"
	"src/test_scenes/ingredients/melee_weapons.cpp"
	"src/test_scenes/ingredients/motorcycle.cpp"
	"src/test_scenes/ingredients/obstacles.cpp"
	"src/test_scenes/ingredients/sprite.cpp"
	"src/test_scenes/ingredients/wsad_player.cpp"
	"src/test_scenes/ingredients/grenades.cpp"
	
	"src/test_scenes/test_scene_images.cpp"
	"src/test_scenes/test_scene_particle_effects.cpp"
	"src/test_scenes/test_scene_physical_materials.cpp"
	"src/test_scenes/test_scene_sentience_properties.cpp"
	"src/test_scenes/test_scene_animations.cpp"
	"src/test_scenes/test_scene_sound_buffers.cpp"
	"src/test_scenes/test_scene_recoil_players.cpp"

	"src/test_scenes/scenes/minimal_scene.cpp"
	"src/test_scenes/scenes/testbed.cpp"
)

# We don't care about alphabetical order. Put new files to the very bottom.

set(HYPERSOMNIA_CODEBASE_SRCS
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
	"src/augs/window_framework/log_color.cpp"
	"src/augs/misc/action_list.cpp"
	"src/augs/misc/enum_bitset.cpp"
	"src/augs/misc/http_requests.cpp"
	"src/augs/misc/machine_entropy.cpp"
	"src/augs/misc/pooled_object_id.cpp"
	"src/augs/misc/readable_bytesize.cpp"
	"src/augs/misc/standard_actions.cpp"
	"src/augs/misc/streams.cpp"
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
	"src/augs/window_framework/translate_windows_enums.cpp"
	"src/game/assets/animation.cpp"
	"src/game/assets/behaviour_tree.cpp"
	"src/view/viewables/regeneration/game_image_loadables.cpp"
	"src/view/viewables/particle_effect.cpp"
	"src/game/assets/physical_material.cpp"
	"src/game/components/all_inferred_state_component.cpp"
	"src/game/components/flags_component.cpp"
	"src/game/components/light_component.cpp"
	"src/game/components/rigid_body_component.cpp"
	"src/game/components/sentience_component.cpp"
	"src/game/components/sound_existence_component.cpp"
	"src/game/components/tree_of_npo_node_component.cpp"
	"src/game/detail/entity_handle_mixins/inventory_mixin.cpp"
	"src/game/detail/entity_handle_mixins/physics_mixin.cpp"
	"src/game/detail/entity_handle_mixins/relations_mixin.cpp"
	"src/game/detail/entity_handle_mixins/spatial_properties_mixin.cpp"
	"src/game/detail/explosions.cpp"
	"src/game/detail/hand_fuse_logic.cpp"
	"src/view/audiovisual_state/aabb_highlighter.cpp"
	"src/view/game_gui/elements/action_button.cpp"
	"src/view/game_gui/elements/character_gui.cpp"
	"src/view/game_gui/elements/drag_and_drop.cpp"
	"src/view/game_gui/elements/grid.cpp"
	"src/view/game_gui/elements/gui_positioning.cpp"
	"src/view/game_gui/elements/hotbar_button.cpp"
	"src/view/game_gui/elements/item_button.cpp"
	"src/view/game_gui/elements/pixel_line_connector.cpp"
	"src/view/game_gui/elements/drag_and_drop_target_drop_item.cpp"
	"src/view/game_gui/elements/game_gui_root.cpp"
	"src/view/game_gui/elements/value_bar.cpp"
	"src/view/game_gui/elements/slot_button.cpp"
	"src/game/detail/inventory/inventory_slot.cpp"
	"src/game/detail/inventory/inventory_slot_handle.cpp"
	"src/game/detail/inventory/inventory_slot_id.cpp"
	"src/game/detail/inventory/inventory_utils.cpp"
	"src/view/viewables/particle_types.cpp"
	"src/game/detail/physics/physics_events.cpp"
	"src/game/detail/physics/physics_friction_fields.cpp"
	"src/game/detail/physics/physics_queries.cpp"
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
	"src/game/inferential_systems/physics_system.cpp"
	"src/game/inferential_systems/processing_lists_system.cpp"
	"src/game/inferential_systems/tree_of_npo_system.cpp"
	"src/game/stateless_systems/destruction_system.cpp"
	"src/game/stateless_systems/hand_fuse_system.cpp"
	"src/view/audiovisual_state/systems/light_system.cpp"
	"src/game/stateless_systems/melee_system.cpp"
	"src/game/stateless_systems/particles_existence_system.cpp"
	"src/game/stateless_systems/sound_existence_system.cpp"
	"src/game/tests_of_traits.cpp"
	"src/game/other_unit_tests.cpp"
	"src/game/transcendental/cosmic_delta.cpp"
	"src/game/transcendental/cosmic_delta_tests.cpp"
	"src/game/transcendental/cosmic_entropy.cpp"
	"src/game/components/processing_component.cpp"
	"src/game/transcendental/cosmic_movie_director.cpp"
	"src/game/transcendental/data_living_one_step.cpp"
	"src/game/transcendental/entity_handle.cpp"
	"src/augs/filesystem/directory.cpp"
	"src/augs/gui/appearance_detector.cpp"
	"src/augs/misc/delta.cpp"
	"src/augs/misc/stepped_timing.cpp"
	"src/augs/templates/templates.cpp"
	"src/augs/window_framework/colored_print.cpp"
	"src/augs/window_framework/platform_utils.cpp"
	"src/game/components/animation_component.cpp"
	"src/game/components/car_component.cpp"
	"src/game/components/container_component.cpp"
	"src/game/components/crosshair_component.cpp"
	"src/game/components/missile_component.cpp"
	"src/game/components/fixtures_component.cpp"
	"src/game/components/item_component.cpp"
	"src/game/components/item_slot_transfers_component.cpp"
	"src/game/components/movement_component.cpp"
	"src/game/components/name_component.cpp"
	"src/game/components/particles_existence_component.cpp"
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
	"src/game/transcendental/logic_step.cpp"
	"src/game/transcendental/cosmos_significant_state.cpp"
	"src/view/network/simulation_broadcast.cpp"
	"src/view/network/simulation_receiver.cpp"
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
	"src/game/stateless_systems/position_copying_system.cpp"
	"src/game/stateless_systems/rotation_copying_system.cpp"
	"src/game/stateless_systems/sentience_system.cpp"
	"src/game/stateless_systems/trace_system.cpp"
	"src/game/stateless_systems/visibility_system.cpp"
	"src/view/rendering_scripts/draw_circular_bars.cpp"
	"src/view/rendering_scripts/draw_cast_spells_highlights.cpp"
	"src/view/rendering_scripts/draw_crosshair_lasers.cpp"
	"src/view/rendering_scripts/draw_hud_for_released_explosives.cpp"
	"src/view/rendering_scripts/illuminated_rendering.cpp"
	"src/view/audiovisual_state/world_camera.cpp"
#	"src/application/setups/server_setup.cpp" #
#	"src/application/setups/setup_base.cpp" #
#	"src/application/setups/two_clients_and_server_setup.cpp" #
	"src/application/setups/test_scene_setup.cpp"
	"src/application/setups/editor_setup.cpp"
#	"src/application/setups/choreographic_setup.cpp" #
#	"src/application/setups/director_setup.cpp" #
	"src/application/setups/main_menu_setup.cpp"
#	"src/application/setups/client_setup.cpp" #
#	"src/application/setups/determinism_test_setup.cpp" #
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
	"src/augs/math/vec2.cpp"
	"src/augs/misc/fixed_delta_timer.cpp"
	"src/augs/misc/randomization.cpp"
	"src/augs/misc/smooth_value_field.cpp"
	"src/augs/misc/timer.cpp"
	"src/augs/misc/value_animator.cpp"
	"src/augs/log.cpp"
	"src/augs/window_framework/event.cpp"
	"src/augs/window_framework/window.cpp"
	"src/augs/audio/sound_data.cpp"
	"src/game/inferential_systems/relational_system.cpp"
	"src/game/components/motor_joint_component.cpp"
	"src/augs/misc/enum_boolset.cpp"
	"src/augs/templates/string_templates.cpp"
	"src/game/components/sender_component.cpp"
	"src/game/inferential_systems/name_system.cpp"
	"src/game/detail/spells/electric_shield.cpp"
	"src/game/detail/spells/electric_triad.cpp"
	"src/game/detail/spells/fury_of_the_aeons.cpp"
	"src/game/detail/spells/haste.cpp"
	"src/game/detail/spells/ultimate_wrath_of_the_aeons.cpp"
	"src/game/detail/spells/exaltation.cpp"
	"src/game/detail/spells/echoes_of_the_higher_realms.cpp"
	"src/augs/misc/lua_utils.cpp"
	"src/test_scenes/test_scenes_content.cpp"
	"src/view/necessary_resources.cpp"
	"src/augs/misc/imgui_utils.cpp"
	"src/application/gui/settings_gui.cpp"
	"src/game/assets/recoil_player.cpp"
	"src/augs/gui/bbcode.cpp"
	"src/augs/unit_tests.cpp"
	"src/game/transcendental/cosmic_profiler.cpp"
	"src/view/audiovisual_state/audiovisual_state.cpp"
	"src/view/viewables/loaded_sounds.cpp"
	"src/view/viewables/atlas_distributions.cpp"
	"src/application/main/draw_debug_details.cpp"
	"src/application/main/imgui_pass.cpp"
	"src/application/main/release_flags.cpp"
	"src/game/components/shape_polygon_component.cpp"
	"src/game/components/shape_circle_component.cpp"
	"src/game/debug_drawing_settings.cpp"
	"src/application/session_profiler.cpp"
	"src/view/viewables/all_viewables_defs.cpp"
	"src/view/audiovisual_state/audiovisual_profiler.cpp"
	"src/application/workspace.cpp"
)

# natvis files for easier debugging in Visual Studio.

if(MSVC)
	list(APPEND HYPERSOMNIA_CODEBASE_SRCS
		"src/game/transcendental/entity_handle.natvis"
		"src/game/detail/inventory/inventory_slot_handle.natvis"
		"src/augs/misc/pooled_object_id.natvis"
	)
endif()

if(BUILD_OPENGL)
	list(APPEND HYPERSOMNIA_CODEBASE_SRCS
		"src/augs/graphics/OpenGL_error.cpp"
	)
endif()

if(BUILD_OPENAL)
	list(APPEND HYPERSOMNIA_CODEBASE_SRCS
		"src/augs/audio/OpenAL_error.cpp"
	)
endif()

if(BUILD_TEST_SCENES)
	list(APPEND HYPERSOMNIA_CODEBASE_SRCS
		${HYPERSOMNIA_TEST_SCENES_SRCS}
	)

	message("Building of test scenes is enabled.")
endif()

# The rest of 3rdparty libraries with minimal amount of source files.

set(HYPERSOMNIA_TRIVIAL_3RDPARTY_SRCS
	"src/3rdparty/lodepng/lodepng.cpp"
	"src/3rdparty/polypartition/src/polypartition.cpp"
	"src/3rdparty/rectpack2D/src/pack.cpp"
	"src/3rdparty/imgui/imgui.cpp"
	"src/3rdparty/imgui/imgui_draw.cpp"
	"src/3rdparty/imgui/imgui_demo.cpp"
)

# Project resource files, e.g. an icon.

if(ADD_APPLICATION_ICON)
	if(MSVC)
		set(HYPESROMNIA_PROJECT_RC_FILES
			"${CMAKE_MODULE_PATH}/Hypersomnia.rc"
		)
	endif()
endif()

set(HYPERSOMNIA_SRCS
	${HYPERSOMNIA_TRIVIAL_3RDPARTY_SRCS}
	${HYPERSOMNIA_BOX2D_SRCS}
	${HYPERSOMNIA_CODEBASE_SRCS}
	${HYPESROMNIA_PROJECT_RC_FILES}

	"src/main.cpp"
)

# We add headers to aid IntelliSense.

file(GLOB_RECURSE HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES
	"src/augs/*.h"
	"src/game/*.h"
	"src/view/*.h"
	"src/application/*.h"
	"src/3rdparty/Box2D/Common/*.h"
	"src/3rdparty/Box2D/Dynamics/*.h"
)

file(GLOB_RECURSE HYPERSOMNIA_HEADERS
    "src/hypersomnia_version.h"
    ${HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES}
)

# We configure include directories for Hypersomnia codebase.

set(HYPERSOMNIA_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/3rdparty"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/lua"
	"${PROJECT_SOURCE_DIR}/src/3rdparty/sol2/single/sol"
)

if(BUILD_UNIT_TESTS)
	list(APPEND HYPERSOMNIA_INCLUDE_DIRS
		"${PROJECT_SOURCE_DIR}/src/3rdparty/Catch/single_include"
	)
endif()

# We configure additional flags for the compiler.

set(HYPERSOMNIA_CXX_FLAGS "/MP /Gm- /fp:strict /bigobj")
set(HYPERSOMNIA_CXX_DEBUG_FLAGS " ")
set(HYPERSOMNIA_CXX_RELEASE_FLAGS "/Ob2 /GL /Oi")
set(HYPERSOMNIA_CXX_RELWITHDEBINFO_FLAGS "/Ob2 /GL /Oi")

# We configure additional flags for linker.

if(MSVC)
	# Enable latest features of C++17
	set(HYPERSOMNIA_CXX_FLAGS "${HYPERSOMNIA_CXX_FLAGS} /std:c++latest")

	set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "/LTCG /INCREMENTAL:NO /OPT:REF")

	# We enable incremental LTCG for at least one release configuration,
	# because LTCG takes very long even with just a single source file modified.

	set(HYPERSOMNIA_LINKER_RELWITHDEBINFO_FLAGS "/LTCG:incremental /INCREMENTAL:NO")
else()

endif()

# We configure some preprocessor defines.

# Lua is configured to compile as C++, of which sol2 library must necessarily know.

add_definitions(-DSOL_USING_CXX_LUA)

# To avoid linker errors, we also need to tell OpenAL Soft we will be using it statically.

if(BUILD_OPENAL)
	add_definitions(-DAL_LIBTYPE_STATIC)
endif()

if(MSVC)
	if (BUILD_PLATFORM_CODE)
		add_definitions(-DPLATFORM_WINDOWS=1)
	endif()

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

# From now on, we will be using Git to obtain some information about commits.
find_package(Git)

message("Git path: ${GIT_EXECUTABLE}")

if(NOT (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git"))
	set(BUILD_VERSION_FILE_GENERATOR FALSE)
	set(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE FALSE)
	message("Git not found. Some functionality will be disabled.")
endif()

# We configure MSVC to use static runtime libraries ("/MT" switch),
# instead of DLL ones.
# The definition of this function is found in cmake/configure_runtime_library.cmake

include(configure_runtime_library)
configure_msvc_runtime()

# We build Lua manually as there is no up-to-date CMakeLists for Lua.

add_library(lua ${LUA_SRCS})

if(BUILD_OPENAL)
	# We build OpenAL Soft. We need to set some variables beforehand.
	
	set(LIBTYPE "STATIC")
	set(FORCE_STATIC_VCRT TRUE CACHE BOOL "/MT for static VC runtimes" FORCE)
	set(ALSOFT_UTILS FALSE CACHE BOOL "Build and install utility programs" FORCE)
	set(ALSOFT_EXAMPLES FALSE CACHE BOOL "Build and install example programs" FORCE)
	set(ALSOFT_TESTS FALSE CACHE BOOL "Build and install test programs" FORCE)
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/openal-soft")
endif()

if(BUILD_SOUND_FORMAT_DECODERS)
	# We build OGG.
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/ogg")
	
	# We build Vorbis. It must know the paths for OGG that we have just built.
	
	set(OGG_ROOT "${PROJECT_SOURCE_DIR}/src/3rdparty/ogg")
	set(OGG_LIBRARIES ogg)
	
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/vorbis")
endif()

# We build the remaining libraries.

if(BUILD_ENET)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/enet")
endif()

if(BUILD_FREETYPE)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/freetype")
endif()

if(BUILD_OPENGL)
	add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/glad")
endif()

# As the following libraries do not use the preferred target_include_directories,
# we must manually acquire the directories that they add via include_directories.

include(add_include_dirs_of)

if(BUILD_ENET)
	add_include_dirs_of("enet")
endif()

if(BUILD_FREETYPE)
	add_include_dirs_of("freetype")
endif()

if(BUILD_SOUND_FORMAT_DECODERS)
	add_include_dirs_of("ogg")
	add_include_dirs_of("vorbis")
endif()

if(BUILD_OPENAL)
	add_include_dirs_of("OpenAL")
endif()

include_directories(${HYPERSOMNIA_INCLUDE_DIRS})

# We build the source code generators.

# We arrange the output directory for the generated source files.

set(GENERATED_SOURCES_PATH
	"${PROJECT_BINARY_DIR}/generators_output/generated"
)

file(MAKE_DIRECTORY ${GENERATED_SOURCES_PATH})

include_directories("${PROJECT_BINARY_DIR}/generators_output")

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

if(BUILD_HTTP_REQUESTS) 
	add_definitions(-DBUILD_HTTP_REQUESTS=1)
endif()

if(BUILD_VERSION_FILE_GENERATOR) 
	add_definitions(-DWAS_VERSION_GENERATOR_BUILT=1)
endif()

# We build the generator of type information required for introspection.

add_subdirectory("${CMAKE_MODULE_PATH}/Introspector-generator")

# We build the generator of commit information.

if(BUILD_VERSION_FILE_GENERATOR)
	add_executable(version_file_generator 
		"${CMAKE_MODULE_PATH}/version_file_generator.cpp"
		"src/augs/misc/time_utils.cpp"
	)
endif()

set(HYPERSOMNIA_VERSION_FILE_PATH
	"${GENERATED_SOURCES_PATH}/hypersomnia_version.cpp"
)

set_source_files_properties(${HYPERSOMNIA_VERSION_FILE_PATH} PROPERTIES GENERATED TRUE)

# We build the main Hypersomnia executable.

set(HYPERSOMNIA_EXECUTABLE_ARGUMENT
	${HYPERSOMNIA_VERSION_FILE_PATH}
	${HYPERSOMNIA_SRCS}
	${HYPERSOMNIA_HEADERS}
)

if(MSVC)
	add_executable(Hypersomnia WIN32 ${HYPERSOMNIA_EXECUTABLE_ARGUMENT})
else()
	add_executable(Hypersomnia ${HYPERSOMNIA_EXECUTABLE_ARGUMENT})
endif()

# Here, we define pre-build events calling the source code generators.

# Before Hypersomnia builds,
# version_file_generator must first query git for information about the most recent commit.

set(HYPERSOMNIA_VERSION_FILE_IN_PATH
	"${PROJECT_SOURCE_DIR}/src/hypersomnia_version.cpp.in"
)

if(BUILD_VERSION_FILE_GENERATOR)
	add_custom_command(
		PRE_BUILD
		TARGET Hypersomnia
		COMMAND version_file_generator ${GIT_EXECUTABLE} ${HYPERSOMNIA_VERSION_FILE_IN_PATH} ${HYPERSOMNIA_VERSION_FILE_PATH}
		COMMENT "Generating version files..."
	)
	
	add_dependencies(Hypersomnia version_file_generator)
else()
	message("Version file generator will not be built. Copying ${HYPERSOMNIA_VERSION_FILE_IN_PATH} into ${HYPERSOMNIA_VERSION_FILE_PATH}")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${HYPERSOMNIA_VERSION_FILE_IN_PATH} ${HYPERSOMNIA_VERSION_FILE_PATH})
endif()

# Before Hypersomnia builds,
# Introspector-generator must first generate the type information.

# We prepare a correct input configuration file for Introspector-generator.

# CMake separates elements of lists with a semicolon, but the Introspector-generator accepts lists separated by newlines.
# We must correct that.

string (REPLACE ";" "\n" 
	HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES_NEWLINE_SEPARATED 
	"${HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES}"
)

set(INTROSPECTOR_GENERATOR_ACTUAL_INPUT_PATH
	"${PROJECT_BINARY_DIR}/introspector_generator_input.cfg"
)

# The correct input configuration file shall contain paths to all headers for whom the generator should create introspectors,
# and should also contain the correct output file path.
# Thus, that call should substitute:
# HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES_NEWLINE_SEPARATED 
# and GENERATED_SOURCES_PATH for the output file path.

configure_file(
	"src/introspector_generator_input.cfg.in"
	${INTROSPECTOR_GENERATOR_ACTUAL_INPUT_PATH}
	@ONLY
)

# Now to define the actual pre-build event.

add_custom_command(
	PRE_BUILD
	TARGET Hypersomnia
	COMMAND Introspector-generator ${INTROSPECTOR_GENERATOR_ACTUAL_INPUT_PATH}
	COMMENT "Generating type information for introspection..."
)

add_dependencies(Hypersomnia Introspector-generator)

if(BUILD_OPENGL)
	# Before Hypersomnia builds,
	# GLAD must first download and generate the source files.

	add_dependencies(Hypersomnia glad)
endif()

# We define libraries upon which Hypersomnia depends.

set(HYPERSOMNIA_LIBS lua)

if(BUILD_OPENGL)
	list(APPEND HYPERSOMNIA_LIBS
		"OpenGL32.lib"
		glad
	)
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

target_link_libraries(Hypersomnia ${HYPERSOMNIA_LIBS})

set_target_properties(Hypersomnia PROPERTIES DEBUG_POSTFIX "-Debug")

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