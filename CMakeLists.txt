cmake_minimum_required(VERSION 3.8)

project(Hypersomnia)

set(HYPERSOMNIA_SRCS
	"3rdparty/Box2D/Collision/b2BroadPhase.cpp"
	"3rdparty/Box2D/Collision/b2CollideCircle.cpp"
	"3rdparty/Box2D/Collision/b2CollideEdge.cpp"
	"3rdparty/Box2D/Collision/b2CollidePolygon.cpp"
	"3rdparty/Box2D/Collision/b2Collision.cpp"
	"3rdparty/Box2D/Collision/b2Distance.cpp"
	"3rdparty/Box2D/Collision/b2DynamicTree.cpp"
	"3rdparty/Box2D/Collision/b2TimeOfImpact.cpp"
	"3rdparty/Box2D/Collision/Shapes/b2ChainShape.cpp"
	"3rdparty/Box2D/Collision/Shapes/b2CircleShape.cpp"
	"3rdparty/Box2D/Collision/Shapes/b2EdgeShape.cpp"
	"3rdparty/Box2D/Collision/Shapes/b2PolygonShape.cpp"
	"3rdparty/Box2D/Common/b2BlockAllocator.cpp"
	"3rdparty/Box2D/Common/b2Draw.cpp"
	"3rdparty/Box2D/Common/b2Math.cpp"
	"3rdparty/Box2D/Common/b2Settings.cpp"
	"3rdparty/Box2D/Common/b2StackAllocator.cpp"
	"3rdparty/Box2D/Common/b2Timer.cpp"
	"3rdparty/Box2D/Dynamics/b2Body.cpp"
	"3rdparty/Box2D/Dynamics/b2ContactManager.cpp"
	"3rdparty/Box2D/Dynamics/b2Fixture.cpp"
	"3rdparty/Box2D/Dynamics/b2Island.cpp"
	"3rdparty/Box2D/Dynamics/b2World.cpp"
	"3rdparty/Box2D/Dynamics/b2WorldCallbacks.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2CircleContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2Contact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2ContactSolver.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
	"3rdparty/Box2D/Dynamics/Contacts/b2PolygonContact.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2DistanceJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2FrictionJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2GearJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2Joint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2MotorJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2MouseJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2PulleyJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2RopeJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2WeldJoint.cpp"
	"3rdparty/Box2D/Dynamics/Joints/b2WheelJoint.cpp"
	"3rdparty/Box2D/Rope/b2Rope.cpp"
	"3rdparty/GL/glew.c"
	"3rdparty/GL/OpenGL.cpp"
	"3rdparty/lodepng/lodepng.cpp"
	"3rdparty/polypartition/src/polypartition.cpp"
	"3rdparty/rectpack2D/src/pack.cpp"
	"application/call_config_script.cpp"
	"application/config_lua_table.cpp"
	"application/content_generation/desaturations.cpp"
	"application/content_generation/buttons_with_corners.cpp"
	"application/content_generation/polygonizations_of_images.cpp"
	"application/content_generation/scripted_images.cpp"
	"application/content_generation/neon_maps.cpp"
	"application/content_generation/texture_atlases.cpp"
	"application/setups/choreographic_setup.cpp"
	"application/setups/director_setup.cpp"
	"application/setups/menu_setup.cpp"
	"application/ui/app_ui_root.cpp"
	"application/ui/option_button.cpp"
	"augs/graphics/texture.cpp"
	"augs/gui/button_corners.cpp"
	"augs/al_log.cpp"
	"augs/audio/audio_manager.cpp"
	"augs/audio/sound_buffer.cpp"
	"augs/audio/sound_source.cpp"
	"augs/ensure.cpp"
	"augs/graphics/drawers.cpp"
	"augs/gui/clipboard.cpp"
	"augs/gui/gui_traversal_structs.cpp"
	"augs/gui/formatted_text.cpp"
	"augs/gui/rect.cpp"
	"augs/gui/text_drawer.cpp"
	"augs/image/font.cpp"
	"augs/image/image.cpp"
	"augs/image/polygonize_image.cpp"
	"augs/window_framework/log_color.cpp"
	"augs/misc/action_list.cpp"
	"augs/misc/enum_bitset.cpp"
	"augs/misc/http_requests.cpp"
	"augs/misc/input_context.cpp"
	"augs/misc/machine_entropy.cpp"
	"augs/misc/pooled_object_id.cpp"
	"augs/misc/readable_bytesize.cpp"
	"augs/misc/standard_actions.cpp"
	"augs/misc/streams.cpp"
	"augs/misc/time_utils.cpp"
	"augs/misc/typesafe_sprintf.cpp"
	"augs/misc/typesafe_sscanf.cpp"
	"augs/network/network_client.cpp"
	"augs/network/network_types.cpp"
	"augs/network/network_server.cpp"
	"augs/network/enet_raii.cpp"
	"augs/network/reliable_channel.cpp"
	"application/setups/client_setup.cpp"
	"application/setups/determinism_test_setup.cpp"
	"augs/texture_atlas/texture_atlas_entry.cpp"
	"augs/tweaker.cpp"
	"augs/window_framework/translate_windows_enums.cpp"
	"game/assets/animation.cpp"
	"game/assets/assets_manager.cpp"
	"game/assets/behaviour_tree.cpp"
	"game/assets/game_image.cpp"
	"game/assets/particle_effect.cpp"
	"game/assets/physical_material.cpp"
	"game/assets/tile_layer.cpp"
	"game/components/flags_component.cpp"
	"game/components/light_component.cpp"
	"game/components/rigid_body_component.cpp"
	"game/components/sentience_component.cpp"
	"game/components/sound_existence_component.cpp"
	"game/components/tree_of_npo_node_component.cpp"
	"game/detail/camera_cone.cpp"
	"game/detail/entity_handle_mixins/inventory_mixin.cpp"
	"game/detail/entity_handle_mixins/physics_mixin.cpp"
	"game/detail/entity_handle_mixins/relations_mixin.cpp"
	"game/detail/entity_handle_mixins/spatial_properties_mixin.cpp"
	"game/detail/explosions.cpp"
	"game/detail/grenade_logic.cpp"
	"game/detail/gui/aabb_highlighter.cpp"
	"game/detail/gui/action_button.cpp"
	"game/detail/gui/character_gui.cpp"
	"game/detail/gui/drag_and_drop.cpp"
	"game/detail/gui/grid.cpp"
	"game/detail/gui/gui_positioning.cpp"
	"game/detail/gui/hotbar_button.cpp"
	"game/detail/gui/item_button.cpp"
	"game/detail/gui/pixel_line_connector.cpp"
	"game/detail/gui/drag_and_drop_target_drop_item.cpp"
	"game/detail/gui/root_of_inventory_gui.cpp"
	"game/detail/gui/sentience_meter_bar.cpp"
	"game/detail/gui/slot_button.cpp"
	"game/detail/inventory/inventory_slot.cpp"
	"game/detail/inventory/inventory_slot_handle.cpp"
	"game/detail/inventory/inventory_slot_id.cpp"
	"game/detail/inventory/inventory_utils.cpp"
	"game/detail/particle_types.cpp"
	"game/detail/physics/physics_events.cpp"
	"game/detail/physics/physics_friction_fields.cpp"
	"game/detail/physics/physics_queries.cpp"
	"game/detail/physics/physics_scripts.cpp"
	"game/detail/sentience_meter.cpp"
	"game/detail/spell_logic.cpp"
	"game/detail/visible_entities.cpp"
	"game/detail/wielding_result.cpp"
	"game/enums/attitude_type.cpp"
	"game/enums/item_category.cpp"
	"game/enums/item_transfer_result_type.cpp"
	"game/enums/slot_physical_behaviour.cpp"
	"game/ingredients/grenades.cpp"
	"game/resource_setups/standard_animations.cpp"
	"game/resource_setups/standard_atlas.cpp"
	"game/resource_setups/standard_behaviour_trees.cpp"
	"game/resource_setups/standard_everything.cpp"
	"game/resource_setups/standard_particle_effects.cpp"
	"game/resource_setups/standard_physical_materials.cpp"
	"game/resource_setups/standard_sound_buffers.cpp"
	"game/resource_setups/standard_spell_datas.cpp"
	"game/resource_setups/standard_tile_layers.cpp"
	"game/scene_builders/networked_testbed.cpp"
	"game/scene_builders/one_entity.cpp"
	"game/scene_builders/testbed.cpp"
	"game/systems_audiovisual/exploding_ring_system.cpp"
	"game/systems_audiovisual/gui_element_system.cpp"
	"game/systems_audiovisual/interpolation_system.cpp"
	"game/systems_audiovisual/particles_simulation_system.cpp"
	"game/systems_audiovisual/past_infection_system.cpp"
	"game/systems_audiovisual/pure_color_highlight_system.cpp"
	"game/systems_audiovisual/sound_system.cpp"
	"game/systems_audiovisual/thunder_system.cpp"
	"game/systems_audiovisual/vertically_flying_number_system.cpp"
	"game/systems_audiovisual/wandering_pixels_system.cpp"
	"game/systems_inferred/physics_system.cpp"
	"game/systems_inferred/processing_lists_system.cpp"
	"game/systems_inferred/tree_of_npo_system.cpp"
	"game/systems_stateless/destruction_system.cpp"
	"game/systems_stateless/grenade_system.cpp"
	"game/systems_stateless/gui_system.cpp"
	"game/systems_audiovisual/light_system.cpp"
	"game/systems_stateless/melee_system.cpp"
	"game/systems_stateless/particles_existence_system.cpp"
	"game/systems_stateless/sound_existence_system.cpp"
	"game/tests_of_traits.cpp"
	"game/transcendental/cosmic_delta.cpp"
	"game/transcendental/cosmic_entropy.cpp"
	"game/components/processing_component.cpp"
	"game/transcendental/cosmic_movie_director.cpp"
	"game/transcendental/cosmic_profiler.cpp"
	"game/detail/convex_partitioned_shape.cpp"
	"application/game_window.cpp"
	"game/transcendental/cosmic_step.cpp"
	"game/transcendental/data_living_one_step.cpp"
	"game/transcendental/entity_handle.cpp"
	"augs/filesystem/directory.cpp"
	"augs/gui/appearance_detector.cpp"
	"augs/misc/delta.cpp"
	"augs/misc/stepped_timing.cpp"
	"augs/misc/measurements.cpp"
	"augs/misc/variable_delta_timer.cpp"
	"augs/templates/templates.cpp"
	"augs/window_framework/colored_print.cpp"
	"augs/window_framework/platform_utils.cpp"
	"game/bindings/bind_game_and_augs.cpp"
	"game/components/animation_component.cpp"
	"game/components/car_component.cpp"
	"game/components/container_component.cpp"
	"game/components/crosshair_component.cpp"
	"game/components/damage_component.cpp"
	"game/components/fixtures_component.cpp"
	"game/components/item_component.cpp"
	"game/components/item_slot_transfers_component.cpp"
	"game/components/movement_component.cpp"
	"game/components/name_component.cpp"
	"game/components/particles_existence_component.cpp"
	"game/components/pathfinding_component.cpp"
	"game/components/polygon_component.cpp"
	"game/components/sprite_component.cpp"
	"game/components/tile_layer_instance_component.cpp"
	"game/components/transform_component.cpp"
	"game/transcendental/cosmos.cpp"
	"game/detail/ai/behaviours.cpp"
	"game/detail/ai/behaviours/explore_in_search_for_last_seen_target.cpp"
	"game/detail/ai/behaviours/immediate_evasion.cpp"
	"game/detail/ai/behaviours/minimize_recoil_through_movement.cpp"
	"game/detail/ai/behaviours/navigate_to_last_seen_position_of_target.cpp"
	"game/detail/ai/behaviours/pull_trigger.cpp"
	"game/detail/ai/behaviours/target_closest_enemy.cpp"
	"game/detail/entity_scripts.cpp"
	"game/enums/filters.cpp"
	"game/ingredients/artificial_life.cpp"
	"game/detail/entity_description.cpp"
	"game/detail/names_and_details.cpp"
	"game/ingredients/backpack.cpp"
	"game/ingredients/box_physics.cpp"
	"game/ingredients/car.cpp"
	"game/ingredients/character_inventory.cpp"
	"game/ingredients/guns.cpp"
	"game/ingredients/ingredients.cpp"
	"game/ingredients/melee_weapons.cpp"
	"game/ingredients/motorcycle.cpp"
	"game/ingredients/obstacles.cpp"
	"game/ingredients/sprite.cpp"
	"game/ingredients/wsad_player.cpp"
	"game/transcendental/logic_step.cpp"
	"game/transcendental/serialization.cpp"
	"game/transcendental/simulation_broadcast.cpp"
	"game/transcendental/simulation_receiver.cpp"
	"game/systems_stateless/animation_system.cpp"
	"game/systems_stateless/behaviour_tree_system.cpp"
	"game/systems_stateless/car_system.cpp"
	"game/systems_stateless/crosshair_system.cpp"
	"game/systems_stateless/damage_system.cpp"
	"game/systems_stateless/destroy_system.cpp"
	"game/systems_stateless/driver_system.cpp"
	"game/systems_stateless/force_joint_system.cpp"
	"game/systems_stateless/gun_system.cpp"
	"game/systems_stateless/input_system.cpp"
	"game/systems_stateless/intent_contextualization_system.cpp"
	"game/systems_stateless/item_system.cpp"
	"game/systems_stateless/movement_system.cpp"
	"game/systems_stateless/pathfinding_system.cpp"
	"game/systems_stateless/position_copying_system.cpp"
	"game/systems_stateless/render_system.cpp"
	"game/systems_stateless/rotation_copying_system.cpp"
	"game/systems_stateless/sentience_system.cpp"
	"game/systems_stateless/trace_system.cpp"
	"game/systems_stateless/trigger_detector_system.cpp"
	"game/systems_stateless/visibility_system.cpp"
	"application/setups/local_setup.cpp"
	"game/view/rendering_scripts/draw_circular_bars.cpp"
	"game/view/rendering_scripts/draw_cast_spells_highlights.cpp"
	"game/view/rendering_scripts/draw_crosshair_lasers.cpp"
	"game/view/rendering_scripts/draw_hud_for_released_grenades.cpp"
	"game/view/rendering_scripts/illuminated_rendering.cpp"
	"game/view/rendering_scripts/standard_rendering.cpp"
	"game/view/standard_audiovisual_post_solve.cpp"
	"game/view/viewing_session.cpp"
	"game/view/viewing_step.cpp"
	"game/view/world_camera.cpp"
	"main.cpp"
	"application/setups/server_setup.cpp"
	"application/setups/setup_base.cpp"
	"application/web_daemon/session_report.cpp"
	"augs/global_libraries.cpp"
	"augs/filesystem/file.cpp"
	"augs/graphics/fbo.cpp"
	"augs/graphics/pixel.cpp"
	"augs/graphics/renderer.cpp"
	"augs/graphics/shader.cpp"
	"augs/graphics/vertex.cpp"
	"augs/gui/dragger.cpp"
	"augs/gui/rect_world.cpp"
	"augs/gui/material.cpp"
	"augs/gui/stroke.cpp"
	"augs/gui/stylesheet.cpp"
	"augs/gui/text/caret.cpp"
	"augs/gui/text/drafter.cpp"
	"augs/gui/text/draft_interface.cpp"
	"augs/gui/text/printer.cpp"
	"augs/gui/text/ui.cpp"
	"augs/gui/text/word_separator.cpp"
	"augs/math/rects.cpp"
	"augs/math/vec2.cpp"
	"augs/misc/fixed_delta_timer.cpp"
	"augs/misc/randomization.cpp"
	"augs/misc/smooth_value_field.cpp"
	"augs/misc/timer.cpp"
	"augs/misc/value_animator.cpp"
	"augs/log.cpp"
	"augs/window_framework/event.cpp"
	"augs/window_framework/window.cpp"
	"application/setups/two_clients_and_server_setup.cpp"
)

file(GLOB_RECURSE HYPERSOMNIA_HEADERS
    "augs/*.h"
    "game/*.h"
    "application/*.h"
    "3rdparty/Box2D/*.h"
)

# We will be building Lua manually.

set(LUA_SRCS
	"3rdparty/lua/lapi.c"
	"3rdparty/lua/lauxlib.c"
	"3rdparty/lua/lbaselib.c"
	"3rdparty/lua/lbitlib.c"
	"3rdparty/lua/lcode.c"
	"3rdparty/lua/lcorolib.c"
	"3rdparty/lua/lctype.c"
	"3rdparty/lua/ldblib.c"
	"3rdparty/lua/ldebug.c"
	"3rdparty/lua/ldo.c"
	"3rdparty/lua/ldump.c"
	"3rdparty/lua/lfunc.c"
	"3rdparty/lua/lgc.c"
	"3rdparty/lua/linit.c"
	"3rdparty/lua/liolib.c"
	"3rdparty/lua/llex.c"
	"3rdparty/lua/lmathlib.c"
	"3rdparty/lua/lmem.c"
	"3rdparty/lua/loadlib.c"
	"3rdparty/lua/lobject.c"
	"3rdparty/lua/lopcodes.c"
	"3rdparty/lua/loslib.c"
	"3rdparty/lua/lparser.c"
	"3rdparty/lua/lstate.c"
	"3rdparty/lua/lstring.c"
	"3rdparty/lua/lstrlib.c"
	"3rdparty/lua/ltable.c"
	"3rdparty/lua/ltablib.c"
	"3rdparty/lua/ltm.c"
	"3rdparty/lua/lundump.c"
	"3rdparty/lua/lutf8lib.c"
	"3rdparty/lua/lvm.c"
	"3rdparty/lua/lzio.c"
)

set_source_files_properties(${LUA_SRCS} PROPERTIES LANGUAGE CXX)

set(HYPERSOMNIA_LIBS
	"OpenGL32.lib"
	"${PROJECT_SOURCE_DIR}/3rdparty/libsndfile/lib/libsndfile-1.lib"
	enet
	freetype
	lua
	OpenAL
)

set(HYPERSOMNIA_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/3rdparty"
	"${PROJECT_SOURCE_DIR}/3rdparty/libsndfile/include"
	"${PROJECT_SOURCE_DIR}/3rdparty/lua"
	"${PROJECT_SOURCE_DIR}/3rdparty/sol2/single/sol"
	"${PROJECT_SOURCE_DIR}/3rdparty/Catch/single_include"
)

set(HYPERSOMNIA_ADDITIONAL_COMPILER_FLAGS "/MP /Gm- /fp:strict /bigobj")

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

configure_msvc_runtime()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HYPERSOMNIA_ADDITIONAL_COMPILER_FLAGS}")

add_definitions(
	-DGLEW_STATIC
	-DSOL_USING_CXX_LUA
)

if(MSVC)
	list(APPEND HYPERSOMNIA_LIBS
		"Ws2_32.lib" 		# for ENet
		"winmm.lib"  		# for ENet
		"${PROJECT_SOURCE_DIR}/3rdparty/http/libmicrohttpd.lib"
	)

	add_definitions(
		-DPLATFORM_WINDOWS
		-D_WINSOCK_DEPRECATED_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS
		-D_SCL_SECURE_NO_WARNINGS
		-DUNICODE
	)
endif()

# We build lua sources manually as there is no up-to-date CMakeLists for lua.

add_library(lua ${LUA_SRCS})

# We build OpenAL. We need to set some variables beforehand.

set(LIBTYPE "STATIC")
set(FORCE_STATIC_VCRT TRUE CACHE BOOL "/MT for static VC runtimes" FORCE)
set(ALSOFT_UTILS FALSE CACHE BOOL          "Build and install utility programs"         FORCE)
set(ALSOFT_EXAMPLES FALSE CACHE BOOL "Build and install example programs"  FORCE)
set(ALSOFT_TESTS FALSE CACHE BOOL     "Build and install test programs"     FORCE)

add_subdirectory("${PROJECT_SOURCE_DIR}/3rdparty/openal-soft")

# We build the remaining libraries.

add_subdirectory("${PROJECT_SOURCE_DIR}/3rdparty/enet")
add_subdirectory("${PROJECT_SOURCE_DIR}/3rdparty/freetype")

macro(add_include_dirs_of target_name)
	get_property(dirs TARGET ${target_name} PROPERTY INCLUDE_DIRECTORIES)
	
	foreach(dir ${dirs})
		message(STATUS "adding external include dir='${dir}'")
		include_directories(${dir})
	endforeach()
endmacro(add_include_dirs_of target_name)

add_include_dirs_of("OpenAL")
add_include_dirs_of("enet")
add_include_dirs_of("freetype")

include_directories(${HYPERSOMNIA_INCLUDE_DIRS})

add_executable(Hypersomnia ${HYPERSOMNIA_SRCS} ${HYPERSOMNIA_HEADERS})
target_link_libraries(Hypersomnia ${HYPERSOMNIA_LIBS})

set_target_properties(Hypersomnia PROPERTIES DEBUG_POSTFIX "-Debug")

if(MSVC)
	set_target_properties(Hypersomnia PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/output")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Hypersomnia)
endif()