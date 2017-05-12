cmake_minimum_required(VERSION 3.8)

project(Hypersomnia)

# Various feature switches.
# If it is your first time building Hypersomnia and you get some errors,
# try turning all of these off to build the minimal possible working Hypersomnia runtime. Then, try to work from there.
# The game should still work, even not having these features.
# In the future, the switches will also differ depending on whether the target application is a client or a server.

option(BUILD_SOUND_FORMAT_DECODERS "Build ogg and vorbis. If this is off, there is also no point in enabling OpenAL." ON)
option(BUILD_OPENAL "Build OpenAL Soft." ON)
option(BUILD_UNIT_TESTS "Build unit tests that are run on game startup." ON)
option(BUILD_VERSION_FILE_GENERATOR "Build version file generator to generate commit information." ON)
option(BUILD_ENET "Build ENet networking library." ON)
option(ADD_COMMAND_TO_ARCHIVE_BUILDS_IN_RELEASE "Add command to archive builds in Release." ON)

if (NOT HYPERSOMNIA_RUNTIME_ARCHIVE_DIR)
	# By default, we set output directory for archives to completely outside of the source tree,
	# because it might be useful to preserve some older builds even if the repository is deleted.

	set(HYPERSOMNIA_RUNTIME_ARCHIVE_DIR
		"${PROJECT_SOURCE_DIR}/../build_database"
	)

	file(MAKE_DIRECTORY ${HYPERSOMNIA_RUNTIME_ARCHIVE_DIR})
endif()

set(RESOURCE_FOLDER_NAME "hypersomnia")
set(HYPERSOMNIA_EXE_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/${RESOURCE_FOLDER_NAME}")

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

# We don't care about alphabetical order. Put new files to the very bottom.

set(HYPERSOMNIA_CODEBASE_SRCS
	"src/application/call_config_script.cpp"
	"src/application/config_lua_table.cpp"
	"src/application/content_generation/desaturations.cpp"
	"src/application/content_generation/buttons_with_corners.cpp"
	"src/application/content_generation/polygonizations_of_images.cpp"
	"src/application/content_generation/scripted_images.cpp"
	"src/application/content_generation/neon_maps.cpp"
	"src/application/content_generation/texture_atlases.cpp"
	"src/application/setups/choreographic_setup.cpp"
	"src/application/setups/director_setup.cpp"
	"src/application/setups/menu_setup.cpp"
	"src/application/ui/app_ui_root.cpp"
	"src/application/ui/option_button.cpp"
	"src/augs/graphics/texture.cpp"
	"src/augs/gui/button_corners.cpp"
	"src/augs/al_log.cpp"
	"src/augs/audio/audio_manager.cpp"
	"src/augs/audio/sound_buffer.cpp"
	"src/augs/audio/sound_source.cpp"
	"src/augs/ensure.cpp"
	"src/augs/graphics/drawers.cpp"
	"src/augs/gui/clipboard.cpp"
	"src/augs/gui/gui_traversal_structs.cpp"
	"src/augs/gui/formatted_text.cpp"
	"src/augs/gui/rect.cpp"
	"src/augs/gui/text_drawer.cpp"
	"src/augs/image/font.cpp"
	"src/augs/image/image.cpp"
	"src/augs/image/polygonize_image.cpp"
	"src/augs/window_framework/log_color.cpp"
	"src/augs/misc/action_list.cpp"
	"src/augs/misc/enum_bitset.cpp"
	"src/augs/misc/http_requests.cpp"
	"src/augs/misc/input_context.cpp"
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
	"src/application/setups/client_setup.cpp"
	"src/application/setups/determinism_test_setup.cpp"
	"src/augs/texture_atlas/texture_atlas_entry.cpp"
	"src/augs/tweaker.cpp"
	"src/augs/window_framework/translate_windows_enums.cpp"
	"src/game/assets/animation.cpp"
	"src/game/assets/assets_manager.cpp"
	"src/game/assets/behaviour_tree.cpp"
	"src/game/assets/game_image.cpp"
	"src/game/assets/particle_effect.cpp"
	"src/game/assets/physical_material.cpp"
	"src/game/assets/tile_layer.cpp"
	"src/game/components/flags_component.cpp"
	"src/game/components/light_component.cpp"
	"src/game/components/rigid_body_component.cpp"
	"src/game/components/sentience_component.cpp"
	"src/game/components/sound_existence_component.cpp"
	"src/game/components/tree_of_npo_node_component.cpp"
	"src/game/detail/camera_cone.cpp"
	"src/game/detail/entity_handle_mixins/inventory_mixin.cpp"
	"src/game/detail/entity_handle_mixins/physics_mixin.cpp"
	"src/game/detail/entity_handle_mixins/relations_mixin.cpp"
	"src/game/detail/entity_handle_mixins/spatial_properties_mixin.cpp"
	"src/game/detail/explosions.cpp"
	"src/game/detail/grenade_logic.cpp"
	"src/game/detail/gui/aabb_highlighter.cpp"
	"src/game/detail/gui/action_button.cpp"
	"src/game/detail/gui/character_gui.cpp"
	"src/game/detail/gui/drag_and_drop.cpp"
	"src/game/detail/gui/grid.cpp"
	"src/game/detail/gui/gui_positioning.cpp"
	"src/game/detail/gui/hotbar_button.cpp"
	"src/game/detail/gui/item_button.cpp"
	"src/game/detail/gui/pixel_line_connector.cpp"
	"src/game/detail/gui/drag_and_drop_target_drop_item.cpp"
	"src/game/detail/gui/root_of_inventory_gui.cpp"
	"src/game/detail/gui/sentience_meter_bar.cpp"
	"src/game/detail/gui/slot_button.cpp"
	"src/game/detail/inventory/inventory_slot.cpp"
	"src/game/detail/inventory/inventory_slot_handle.cpp"
	"src/game/detail/inventory/inventory_slot_id.cpp"
	"src/game/detail/inventory/inventory_utils.cpp"
	"src/game/detail/particle_types.cpp"
	"src/game/detail/physics/physics_events.cpp"
	"src/game/detail/physics/physics_friction_fields.cpp"
	"src/game/detail/physics/physics_queries.cpp"
	"src/game/detail/physics/physics_scripts.cpp"
	"src/game/detail/sentience_meter.cpp"
	"src/game/detail/spell_logic.cpp"
	"src/game/detail/visible_entities.cpp"
	"src/game/detail/wielding_result.cpp"
	"src/game/enums/attitude_type.cpp"
	"src/game/enums/item_category.cpp"
	"src/game/enums/item_transfer_result_type.cpp"
	"src/game/enums/slot_physical_behaviour.cpp"
	"src/game/ingredients/grenades.cpp"
	"src/game/test_scenes/resource_setups/standard_animations.cpp"
	"src/game/test_scenes/resource_setups/standard_atlas.cpp"
	"src/game/test_scenes/resource_setups/standard_behaviour_trees.cpp"
	"src/game/test_scenes/resource_setups/standard_everything.cpp"
	"src/game/test_scenes/resource_setups/standard_particle_effects.cpp"
	"src/game/test_scenes/resource_setups/standard_physical_materials.cpp"
	"src/game/test_scenes/resource_setups/standard_sound_buffers.cpp"
	"src/game/test_scenes/resource_setups/standard_spell_datas.cpp"
	"src/game/test_scenes/resource_setups/standard_tile_layers.cpp"
	"src/game/test_scenes/networked_testbed.cpp"
	"src/game/test_scenes/one_entity.cpp"
	"src/game/test_scenes/testbed.cpp"
	"src/game/systems_audiovisual/exploding_ring_system.cpp"
	"src/game/systems_audiovisual/gui_element_system.cpp"
	"src/game/systems_audiovisual/interpolation_system.cpp"
	"src/game/systems_audiovisual/particles_simulation_system.cpp"
	"src/game/systems_audiovisual/past_infection_system.cpp"
	"src/game/systems_audiovisual/pure_color_highlight_system.cpp"
	"src/game/systems_audiovisual/sound_system.cpp"
	"src/game/systems_audiovisual/thunder_system.cpp"
	"src/game/systems_audiovisual/vertically_flying_number_system.cpp"
	"src/game/systems_audiovisual/wandering_pixels_system.cpp"
	"src/game/systems_inferred/physics_system.cpp"
	"src/game/systems_inferred/processing_lists_system.cpp"
	"src/game/systems_inferred/tree_of_npo_system.cpp"
	"src/game/systems_stateless/destruction_system.cpp"
	"src/game/systems_stateless/grenade_system.cpp"
	"src/game/systems_stateless/gui_system.cpp"
	"src/game/systems_audiovisual/light_system.cpp"
	"src/game/systems_stateless/melee_system.cpp"
	"src/game/systems_stateless/particles_existence_system.cpp"
	"src/game/systems_stateless/sound_existence_system.cpp"
	"src/game/tests_of_traits.cpp"
	"src/game/transcendental/cosmic_delta.cpp"
	"src/game/transcendental/cosmic_entropy.cpp"
	"src/game/components/processing_component.cpp"
	"src/game/transcendental/cosmic_movie_director.cpp"
	"src/game/transcendental/cosmic_profiler.cpp"
	"src/game/detail/convex_partitioned_shape.cpp"
	"src/application/game_window.cpp"
	"src/game/transcendental/cosmic_step.cpp"
	"src/game/transcendental/data_living_one_step.cpp"
	"src/game/transcendental/entity_handle.cpp"
	"src/augs/filesystem/directory.cpp"
	"src/augs/gui/appearance_detector.cpp"
	"src/augs/misc/delta.cpp"
	"src/augs/misc/stepped_timing.cpp"
	"src/augs/misc/measurements.cpp"
	"src/augs/misc/variable_delta_timer.cpp"
	"src/augs/templates/templates.cpp"
	"src/augs/window_framework/colored_print.cpp"
	"src/augs/window_framework/platform_utils.cpp"
	"src/game/bindings/bind_game_and_augs.cpp"
	"src/game/components/animation_component.cpp"
	"src/game/components/car_component.cpp"
	"src/game/components/container_component.cpp"
	"src/game/components/crosshair_component.cpp"
	"src/game/components/damage_component.cpp"
	"src/game/components/fixtures_component.cpp"
	"src/game/components/item_component.cpp"
	"src/game/components/item_slot_transfers_component.cpp"
	"src/game/components/movement_component.cpp"
	"src/game/components/name_component.cpp"
	"src/game/components/particles_existence_component.cpp"
	"src/game/components/pathfinding_component.cpp"
	"src/game/components/polygon_component.cpp"
	"src/game/components/sprite_component.cpp"
	"src/game/components/tile_layer_instance_component.cpp"
	"src/game/components/transform_component.cpp"
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
	"src/game/ingredients/artificial_life.cpp"
	"src/game/detail/entity_description.cpp"
	"src/game/detail/names_and_details.cpp"
	"src/game/ingredients/backpack.cpp"
	"src/game/ingredients/box_physics.cpp"
	"src/game/ingredients/car.cpp"
	"src/game/ingredients/character_inventory.cpp"
	"src/game/ingredients/guns.cpp"
	"src/game/ingredients/ingredients.cpp"
	"src/game/ingredients/melee_weapons.cpp"
	"src/game/ingredients/motorcycle.cpp"
	"src/game/ingredients/obstacles.cpp"
	"src/game/ingredients/sprite.cpp"
	"src/game/ingredients/wsad_player.cpp"
	"src/game/transcendental/logic_step.cpp"
	"src/game/transcendental/serialization.cpp"
	"src/game/transcendental/simulation_broadcast.cpp"
	"src/game/transcendental/simulation_receiver.cpp"
	"src/game/systems_stateless/animation_system.cpp"
	"src/game/systems_stateless/behaviour_tree_system.cpp"
	"src/game/systems_stateless/car_system.cpp"
	"src/game/systems_stateless/crosshair_system.cpp"
	"src/game/systems_stateless/damage_system.cpp"
	"src/game/systems_stateless/destroy_system.cpp"
	"src/game/systems_stateless/driver_system.cpp"
	"src/game/systems_stateless/force_joint_system.cpp"
	"src/game/systems_stateless/gun_system.cpp"
	"src/game/systems_stateless/input_system.cpp"
	"src/game/systems_stateless/intent_contextualization_system.cpp"
	"src/game/systems_stateless/item_system.cpp"
	"src/game/systems_stateless/movement_system.cpp"
	"src/game/systems_stateless/pathfinding_system.cpp"
	"src/game/systems_stateless/position_copying_system.cpp"
	"src/game/systems_stateless/render_system.cpp"
	"src/game/systems_stateless/rotation_copying_system.cpp"
	"src/game/systems_stateless/sentience_system.cpp"
	"src/game/systems_stateless/trace_system.cpp"
	"src/game/systems_stateless/trigger_detector_system.cpp"
	"src/game/systems_stateless/visibility_system.cpp"
	"src/application/setups/local_setup.cpp"
	"src/game/view/rendering_scripts/draw_circular_bars.cpp"
	"src/game/view/rendering_scripts/draw_cast_spells_highlights.cpp"
	"src/game/view/rendering_scripts/draw_crosshair_lasers.cpp"
	"src/game/view/rendering_scripts/draw_hud_for_released_grenades.cpp"
	"src/game/view/rendering_scripts/illuminated_rendering.cpp"
	"src/game/view/rendering_scripts/standard_rendering.cpp"
	"src/game/view/standard_audiovisual_post_solve.cpp"
	"src/game/view/viewing_session.cpp"
	"src/game/view/viewing_step.cpp"
	"src/game/view/world_camera.cpp"
	"src/application/setups/server_setup.cpp"
	"src/application/setups/setup_base.cpp"
	"src/application/web_daemon/session_report.cpp"
	"src/augs/global_libraries.cpp"
	"src/augs/graphics/fbo.cpp"
	"src/augs/graphics/pixel.cpp"
	"src/augs/graphics/renderer.cpp"
	"src/augs/graphics/shader.cpp"
	"src/augs/graphics/vertex.cpp"
	"src/augs/gui/dragger.cpp"
	"src/augs/gui/rect_world.cpp"
	"src/augs/gui/material.cpp"
	"src/augs/gui/stroke.cpp"
	"src/augs/gui/stylesheet.cpp"
	"src/augs/gui/text/caret.cpp"
	"src/augs/gui/text/drafter.cpp"
	"src/augs/gui/text/draft_interface.cpp"
	"src/augs/gui/text/printer.cpp"
	"src/augs/gui/text/ui.cpp"
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
	"src/application/setups/two_clients_and_server_setup.cpp"
	"src/augs/graphics/OpenGL_error.cpp"
	"src/augs/audio/sound_data.cpp"
	"src/augs/audio/sound_samples_from_file.cpp"
	"src/game/systems_inferred/relational_system.cpp"
	"src/game/components/motor_joint_component.cpp"
)

set(HYPERSOMNIA_TRIVIAL_3RDPARTY_SRCS
	"src/3rdparty/lodepng/lodepng.cpp"
	"src/3rdparty/polypartition/src/polypartition.cpp"
	"src/3rdparty/rectpack2D/src/pack.cpp"
)

set(HYPERSOMNIA_SRCS
	${HYPERSOMNIA_TRIVIAL_3RDPARTY_SRCS}
	${HYPERSOMNIA_BOX2D_SRCS}
	${HYPERSOMNIA_CODEBASE_SRCS}

	"src/main.cpp"
)

# We add headers to aid IntelliSense.

file(GLOB_RECURSE HYPERSOMNIA_HEADERS_WITH_INTROSPECTED_CLASSES
	"src/augs/*.h"
	"src/game/*.h"
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
set(HYPERSOMNIA_CXX_RELEASE_FLAGS "/Ob2 /GL /Oi")

# We configure additional flags for linker.

if(MSVC)
	set(HYPERSOMNIA_LINKER_RELEASE_FLAGS "/LTCG /INCREMENTAL:NO /OPT:REF")

	# We enable incremental LTCG for at least one release configuration,
	# because LTCG takes vastly long even with just a single source file modified.

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
	add_definitions(
		-DPLATFORM_WINDOWS
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

set(CMAKE_CXX_FLAGS_RELEASE 
 "${CMAKE_CXX_FLAGS_RELEASE} ${HYPERSOMNIA_CXX_RELEASE_FLAGS}"
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 
 "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${HYPERSOMNIA_CXX_RELEASE_FLAGS}"
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

add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/freetype")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/3rdparty/glad")

# As the following libraries do not use the preferred target_include_directories,
# we must manually acquire the directories that they add via include_directories.

include(add_include_dirs_of)

if(BUILD_ENET)
	add_include_dirs_of("enet")
endif()

add_include_dirs_of("freetype")

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

# First we configure the define headers according to the currently set options.
# We could just add the preprocessor definitions as flags,
# but this has the advantage that testing a change to a flag does not incur recompilation of the whole project.

configure_file("src/augs/build_settings/setting_build_openal.h.in" "${GENERATED_SOURCES_PATH}/setting_build_openal.h") 
configure_file("src/augs/build_settings/setting_build_sound_format_decoders.h.in" "${GENERATED_SOURCES_PATH}/setting_build_sound_format_decoders.h") 
configure_file("src/augs/build_settings/setting_build_enet.h.in" "${GENERATED_SOURCES_PATH}/setting_build_enet.h") 
configure_file("src/augs/build_settings/setting_build_unit_tests.h.in" "${GENERATED_SOURCES_PATH}/setting_build_unit_tests.h")
# Just a different name for the variable
set(WAS_VERSION_GENERATOR_BUILT ${BUILD_VERSION_FILE_GENERATOR})
configure_file("src/augs/build_settings/setting_was_version_generator_built.h.in" "${GENERATED_SOURCES_PATH}/setting_was_version_generator_built.h") 

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

add_executable(Hypersomnia 
	${HYPERSOMNIA_VERSION_FILE_PATH} 
	${HYPERSOMNIA_SRCS} 
	${HYPERSOMNIA_HEADERS}
)

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

# Before Hypersomnia builds,
# GLAD must first download and generate the source files.

add_dependencies(Hypersomnia glad)

# We define libraries upon which Hypersomnia depends.

set(HYPERSOMNIA_LIBS
	"OpenGL32.lib"
	freetype
	lua
	glad
)

if(BUILD_OPENAL)
	list(APPEND HYPERSOMNIA_LIBS OpenAL)
endif()

if(BUILD_ENET)
	list(APPEND HYPERSOMNIA_LIBS enet)
endif()

if(BUILD_SOUND_FORMAT_DECODERS)
	list(APPEND HYPERSOMNIA_LIBS vorbisfile)
endif()

if(MSVC)
	if(BUILD_ENET)
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
			-DRESOURCE_FOLDER_NAME=${RESOURCE_FOLDER_NAME}
			-P ${CMAKE_MODULE_PATH}/create_build_archive.cmake
		DEPENDS ${CMAKE_MODULE_PATH}/create_build_archive.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Archiving the executable and resources..."
		VERBATIM
	)
endif()