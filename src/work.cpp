#if PLATFORM_UNIX
#include <csignal>
#endif

#include <functional>

#include "fp_consistency_tests.h"

#include "augs/log_path_getters.h"
#include "augs/unit_tests.h"
#include "augs/global_libraries.h"

#include "augs/templates/identity_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/history.hpp"
#include "augs/templates/traits/in_place.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/misc/time_utils.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/lua/lua_utils.h"

#include "augs/graphics/renderer.h"

#include "augs/window_framework/shell.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/audio/audio_context.h"

#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"

#include "view/game_gui/game_gui_system.h"

#include "view/audiovisual_state/world_camera.h"
#include "view/audiovisual_state/audiovisual_state.h"
#include "view/rendering_scripts/illuminated_rendering.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/frame_profiler.h"

#include "application/session_profiler.h"
#include "application/config_lua_table.h"

#include "application/gui/settings_gui.h"
#include "application/gui/start_client_gui.h"
#include "application/gui/start_server_gui.h"
#include "application/gui/ingame_menu_gui.h"

#include "application/network/network_common.h"
#include "application/setups/all_setups.h"

#include "application/main/imgui_pass.h"
#include "application/main/draw_debug_details.h"
#include "application/main/draw_debug_lines.h"
#include "application/main/release_flags.h"
#include "application/setups/editor/editor_player.hpp"

#include "application/input/input_pass_result.h"

#include "application/setups/draw_setup_gui_input.h"

#include "cmd_line_params.h"
#include "build_info.h"

#include "augs/readwrite/byte_readwrite.h"
#include "view/game_gui/special_indicator_logic.h"
#include "augs/window_framework/create_process.h"
#include "application/setups/editor/editor_popup.h"

std::function<void()> ensure_handler;
bool log_to_live_file = false;
bool is_dedicated_server = false;

/*
	static is used for all variables because some take massive amounts of space.
	They would otherwise cause a stack overflow.
	For example, Windows provides us with mere 1MB of stack space by default.
	
	To preserve the destruction in the order of definition,
	we must also make all other variables static to avoid bugs.

	This function will only be entered ONCE during the lifetime of the program.
*/

int work(const int argc, const char* const * const argv) try {
	setup_float_flags();

	augs::create_directories(LOG_FILES_DIR);
	augs::create_directories(SERVER_LOG_FILES_DIR);

	static augs::timer until_first_swap;
	bool until_first_swap_measured = false;

#if PLATFORM_UNIX	
	static volatile std::sig_atomic_t signal_status = 0;
 
	static auto signal_handler = [](const int signal_type) {
   		signal_status = signal_type;
	};

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);
	std::signal(SIGSTOP, signal_handler);
#endif

	static session_profiler performance;
	static network_profiler network_performance;
	static network_info network_stats;
	static server_network_info server_stats;

	LOG("If the game crashes repeatedly, consider deleting the \"cache\" folder.\n");
	LOG("Started at %x", augs::date_time().get_readable());
	LOG("Working directory: %x", augs::get_current_working_directory());

	LOG("Creating directories: %x %x %x", GENERATED_FILES_DIR, LOCAL_FILES_DIR);

	augs::create_directories(GENERATED_FILES_DIR);
	augs::create_directories(LOCAL_FILES_DIR);

	dump_detailed_sizeof_information(get_path_in_log_files("detailed_sizeofs.txt"));

	static const auto canon_config_path = augs::path_type("config.lua");
	static const auto local_config_path = augs::path_type(LOCAL_FILES_DIR "/config.local.lua");

	LOG("Creating lua state.");
	static auto lua = augs::create_lua_state();

	LOG("Loading the config.");
	static config_lua_table config { 
		lua, 
		augs::switch_path(
			canon_config_path,
			local_config_path
		)
	};

	static const auto float_tests_succeeded = 
		config.perform_float_consistency_test 
		? perform_float_consistency_tests() 
		: true
	;

	if (config.log_to_live_file) {
		augs::remove_file(get_path_in_log_files("live_debug.txt"));

		log_to_live_file = true;

		LOG("Live log was enabled due to a flag in config.");
		LOG("Live log file created at %x", augs::date_time().get_readable());
	}

	LOG("Initializing ImGui.");

	static const auto imgui_ini_path = 
		is_dedicated_server ? 
		LOCAL_FILES_DIR "/server_imgui.ini"
		: LOCAL_FILES_DIR "/imgui.ini"
	;

	static const auto imgui_log_path = get_path_in_log_files("imgui_log.txt");

	augs::imgui::init(
		imgui_ini_path,
		imgui_log_path.c_str(),
		config.gui_style
	);

	static auto last_saved_config = config;

	static auto change_with_save = [&](auto setter) {
		setter(config);
		setter(last_saved_config);

		last_saved_config.save(lua, local_config_path);
	};

	static auto last_exit_incorrect_popup = std::optional<editor_popup>();

	if (const auto last_failure_log = find_last_incorrect_exit(); last_failure_log.size() > 0) {
		change_with_save([](config_lua_table& cfg) {
			cfg.launch_mode = launch_type::MAIN_MENU;
		});

		last_exit_incorrect_popup = editor_popup {
			"Warning",
			typesafe_sprintf(R"(Looks like the game crashed since last time.
Consider sending developers the log file located at:

%x

If you experience repeated crashes, 
you might try to reset all your settings,
which can be done by pressing "Reset to factory default" in Settings->General,
and then hitting Save settings.
)", last_failure_log),
			""
		};
	}

	LOG("Parsing command-line parameters.");
	static const auto params = cmd_line_params(argc, argv);

	static augs::timer global_libraries_timer;

	LOG("Initializing global libraries");

	static const auto libraries = 
		params.start_dedicated_server 
		? augs::global_libraries({}) 
		: augs::global_libraries(augs::global_libraries::library::FREETYPE) 
	;

	LOG("Initializing global libraries took: %x ms", global_libraries_timer.template extract<std::chrono::milliseconds>());

	static std::optional<setup_variant> current_setup;

	static auto has_current_setup = []() {
		return current_setup != std::nullopt;
	};

	static auto emplace_current_setup = [&p = current_setup] (auto tag, auto&&... args) {
		using Tag = decltype(tag);
		using T = type_of_in_place_type_t<Tag>; 

		if (p == std::nullopt) {
			p.emplace(
				tag,
				std::forward<decltype(args)>(args)...
			);
		}
		else {
			p.value().emplace<T>(std::forward<decltype(args)>(args)...);
		}
	};

	if (config.unit_tests.run) {
		/* Needed by some unit tests */
		augs::network_raii raii;

		LOG("Running unit tests.");
		augs::run_unit_tests(config.unit_tests);

		LOG("All unit tests have passed.");

		if (params.unit_tests_only) {
			return EXIT_SUCCESS;
		}
	}
	else {
		LOG("Unit tests were disabled.");
	}

	if (params.start_dedicated_server) {
		LOG("Starting the dedicated server at port: %x", config.default_server_start.port);

		emplace_current_setup(
			std::in_place_type_t<server_setup>(),
			lua,
			config.default_server_start,
			config.server,
			config.client,
			config.private_server,
			config.dedicated_server
		);

		auto& server = std::get<server_setup>(*current_setup);

		while (server.is_running()) {
			auto scope = measure_scope(performance.fps);

#if PLATFORM_UNIX
			if (signal_status != 0) {
				const auto sig = signal_status;

				LOG("%x received.", strsignal(sig));

				if(
					sig == SIGINT
					|| sig == SIGSTOP
					|| sig == SIGTERM
				) {
					LOG("Gracefully shutting down.");
					break;
				}
			}
#endif

			const auto zoom = 1.f;

			server.advance(
				{
					vec2i(),
					config.input,
					zoom,
					network_performance,
					server_stats
				},
				solver_callbacks()
			);

			server.sleep_until_next_tick();
		}

		return EXIT_SUCCESS;
	}

	LOG("Initializing the audio context.");

	static augs::audio_context audio(config.audio);

	LOG("Logging all audio devices.");
	augs::log_all_audio_devices(get_path_in_log_files("audio_devices.txt"));

	LOG("Initializing the window.");
	static augs::window window(config.window);

	LOG("Initializing the renderer.");
	static augs::renderer renderer(config.renderer);
	LOG_NVPS(renderer.get_max_texture_size());

	LOG("Initializing the necessary fbos.");
	static all_necessary_fbos necessary_fbos(
		window.get_screen_size(),
		config.drawing
	);

	LOG("Initializing the necessary shaders.");
	static all_necessary_shaders necessary_shaders(
		"content/necessary/shaders",
		"content/necessary/shaders",
		config.drawing
	);

	LOG("Initializing the necessary sounds.");
	static all_necessary_sounds necessary_sounds(
		"content/necessary/sfx"
	);

	LOG("Initializing the necessary image definitions.");
	static const necessary_image_definitions_map necessary_image_definitions(
		lua,
		"content/necessary/gfx",
		config.content_regeneration.regenerate_every_time
	);
	
	LOG("Creating the ImGui atlas.");
	static const auto imgui_atlas = augs::imgui::create_atlas(config.gui_fonts.gui);

	static const auto configurables = configuration_subscribers {
		window,
		necessary_fbos,
		audio,
		renderer
	};

	static atlas_profiler atlas_performance;
	static frame_profiler frame_performance;

	/* 
		unique_ptr is used to avoid stack overflow.

		Main menu setup state may be preserved, 
		therefore it resides in a separate unique_ptr.
	*/

	static std::optional<main_menu_setup> main_menu;

	static auto has_main_menu = []() {
		return main_menu != std::nullopt;
	};

	static auto emplace_main_menu = [&p = main_menu] (auto&&... args) {
		if (p == std::nullopt) {
			p.emplace(std::forward<decltype(args)>(args)...);
		}
	};

	static settings_gui_state settings_gui = std::string("Settings");
	static start_client_gui_state start_client_gui = std::string("Connect to server");
	static start_server_gui_state start_server_gui = std::string("Host a server");

	static ingame_menu_gui ingame_menu;

	/*
		Runtime representations of viewables,
		loaded from the definitions provided by the current setup.
		The setup's chosen viewables_loading_type decides if they are 
		loaded just once or if they are for example continuously streamed.
	*/

	LOG("Initializing the streaming of viewables.");
	static viewables_streaming streaming(renderer);

	auto streaming_finalize = augs::scope_guard([&]() {
		streaming.finalize_pending_tasks();
	});

	static world_camera gameplay_camera;
	LOG("Initializing the audiovisual state.");
	static audiovisual_state audiovisuals;
	
	static auto get_audiovisuals = []() -> audiovisual_state& {
		return audiovisuals;
	};


	/*
		The lambdas that aid to make the main loop code more concise.
	*/	

	static auto visit_current_setup = [&](auto callback) -> decltype(auto) {
		if (has_current_setup()) {
			return std::visit(
				[&](auto& setup) -> decltype(auto) {
					return callback(setup);
				}, 
				*current_setup
			);
		}
		else {
			return callback(*main_menu);
		}
	};

	static auto setup_requires_cursor = []() {
		return visit_current_setup([&](const auto& s) {
			return s.requires_cursor();
		});
	};

	static auto on_specific_setup = [&](auto callback) -> decltype(auto) {
		using T = remove_cref<argument_t<decltype(callback), 0>>;

		if constexpr(std::is_same_v<T, main_menu_setup>) {
			if (has_main_menu()) {
				callback(*main_menu);
			}
		}
		else {
			if (has_current_setup()) {
				if (auto* setup = std::get_if<T>(&*current_setup)) {
					callback(*setup);
				}
			}
		}
	};

	static auto get_unofficial_content_dir = [&]() {
		return visit_current_setup([](const auto& s) { return s.get_unofficial_content_dir(); });
	};

	static auto get_render_layer_filter = [&]() {
		return visit_current_setup([](const auto& s) { return s.get_render_layer_filter(); });
	};

	/* TODO: We need to have one game gui per cosmos. */
	static game_gui_system game_gui;
	static bool game_gui_mode_flag = false;

	static auto load_all = [&](const all_viewables_defs& new_defs) {
		std::optional<arena_player_metas> new_player_metas;

		if (streaming.finished_loading_player_metas()) {
			visit_current_setup([&](auto& setup) {
				new_player_metas = setup.get_new_player_metas();
			});
		}

		streaming.load_all({
			new_defs,
			necessary_image_definitions,
			config.gui_fonts,
			config.content_regeneration,
			get_unofficial_content_dir(),
			renderer,
			renderer.get_max_texture_size(),

			new_player_metas
		});
	};

	static auto setup_launcher = [&](auto&& setup_init_callback) {
		game_gui_mode_flag = false;
		get_audiovisuals().get<sound_system>().clear();

		network_stats = {};
		server_stats = {};

		main_menu.reset();
		current_setup.reset();
		ingame_menu.show = false;

		setup_init_callback();
		
		visit_current_setup([&](const auto& setup) {
			using T = remove_cref<decltype(setup)>;
			
			if constexpr(T::loading_strategy == viewables_loading_type::LOAD_ALL_ONLY_ONCE) {
				load_all(setup.get_viewable_defs());
			}
		});
	};

	static auto launch_editor = [&](auto&&... args) {
		setup_launcher([&]() {
			emplace_current_setup(std::in_place_type_t<editor_setup>(),
				std::forward<decltype(args)>(args)...
			);
		});
	};

	static auto launch_setup = [&](const launch_type mode) {
		LOG("Launched mode: %x", augs::enum_to_string(mode));
		
		change_with_save([mode](config_lua_table& cfg) {
			cfg.launch_mode = mode;
		});

		switch (mode) {
			case launch_type::MAIN_MENU:
				setup_launcher([&]() {
					if (!has_main_menu()) {
						emplace_main_menu(lua, config.main_menu);
					}
				});

				break;

			case launch_type::CLIENT:
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<client_setup>(),
						lua,
						config.default_client_start
					);
				});

				break;

			case launch_type::SERVER:
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<server_setup>(),
						lua,
						config.default_server_start,
						config.server,
						config.client,
						config.private_server,
						std::nullopt
					);
				});

				break;

			case launch_type::EDITOR:
				launch_editor(lua);

				break;

			case launch_type::TEST_SCENE:
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<test_scene_setup>(),
						lua,
						config.test_scene,
						config.get_input_recording_mode()
					);
				});

				break;

			default:
				ensure(false && "The launch_setup mode you have chosen is currently out of service.");
				break;
		}
	};

	static auto get_viewable_defs = [&]() -> const all_viewables_defs& {
		return visit_current_setup([](auto& setup) -> const all_viewables_defs& {
			return setup.get_viewable_defs();
		});
	};

	static auto create_game_gui_deps = [&](const auto& viewing_config) {
		return game_gui_context_dependencies {
			get_viewable_defs().image_definitions,
			streaming.images_in_atlas,
			streaming.necessary_images_in_atlas,
			streaming.get_loaded_gui_fonts().gui,
			get_audiovisuals().randomizing,
			viewing_config.game_gui
		};
	};

	static auto create_menu_context_deps = [&](const auto& viewing_config) {
		return menu_context_dependencies{
			streaming.necessary_images_in_atlas,
			streaming.get_loaded_gui_fonts().gui,
			necessary_sounds,
			viewing_config.audio_volume
		};
	};

	static auto get_game_gui_subject = [&]() -> const_entity_handle {
		const auto& viewed_cosmos = visit_current_setup([](auto& setup) -> const cosmos& {
			return setup.get_viewed_cosmos();
		});

		const auto gui_character_id = visit_current_setup([](auto& setup) {
			return setup.get_game_gui_subject_id();
		});

		return viewed_cosmos[gui_character_id];
	};

	static auto get_viewed_character = [&]() -> const_entity_handle {
		const auto& viewed_cosmos = visit_current_setup([](auto& setup) -> const cosmos& {
			return setup.get_viewed_cosmos();
		});

		const auto viewed_character_id = visit_current_setup([](auto& setup) {
			return setup.get_viewed_character_id();
		});

		return viewed_cosmos[viewed_character_id];
	};
		
	static auto should_draw_game_gui = [&]() {
		{
			bool should = true;

			on_specific_setup([&](editor_setup& setup) {
				if (!setup.anything_opened() || setup.is_editing_mode()) {
					should = false;
				}
			});

			if (has_main_menu() && !has_current_setup()) {
				should = false;
			}

			if (!should) {
				return false;
			}
		}

		const auto viewed = get_game_gui_subject();

		if (!viewed.alive()) {
			return false;
		}

		if (!viewed.has<components::item_slot_transfers>()) {
			return false;
		}

		return true;
	};

	static auto get_camera_eye = [&]() {		
		if(const auto custom = visit_current_setup(
			[](const auto& setup) { 
				return setup.find_current_camera_eye(); 
			}
		)) {
			return *custom;
		}
		
		return gameplay_camera.get_current_eye();
	};

	static auto handle_app_intent = [&](const app_intent_type intent) {
		using T = decltype(intent);

		switch (intent) {
			case T::SWITCH_DEVELOPER_CONSOLE: {
				change_with_save([](config_lua_table& cfg) {
					bool& f = cfg.session.show_developer_console;
					f = !f;
				});

				break;
			}

			default: break;
		}
	};
	
	static auto handle_app_ingame_intent = [&](const app_ingame_intent_type intent) {
		using T = decltype(intent);

		switch (intent) {
			case T::CLEAR_DEBUG_LINES:
				DEBUG_PERSISTENT_LINES.clear();
				return true;

			case T::SWITCH_WEAPON_LASER: {
				bool& f = config.drawing.draw_weapon_laser;
				f = !f;
				return true;
			}

			case T::SWITCH_GAME_GUI_MODE: {
				bool& f = game_gui_mode_flag;
				f = !f;
				return true;
			}
			
			default: return false;
		}
	};
 
	static auto main_ensure_handler = []() {
		visit_current_setup(
			[&](auto& setup) {
				setup.ensure_handler();
			}
		);
	};

	::ensure_handler = main_ensure_handler;

	static bool should_quit = false;

	static augs::event::state common_input_state;

	static bool client_start_requested = false;
	static bool server_start_requested = false;

	static auto do_main_menu_option = [&](const main_menu_button_type t) {
		using T = decltype(t);

		switch (t) {
			case T::CONNECT_TO_UNIVERSE:
				start_client_gui.open();

				if (common_input_state[augs::event::keys::key::LSHIFT]) {
					client_start_requested = true;
				}

				break;
				
			case T::HOST_UNIVERSE:
				start_server_gui.open();

				if (common_input_state[augs::event::keys::key::LSHIFT]) {
					server_start_requested = true;
				}

				break;

			case T::LOCAL_UNIVERSE:
				launch_setup(launch_type::TEST_SCENE);
				break;

			case T::EDITOR:
				launch_setup(launch_type::EDITOR);
				break;

			case T::SETTINGS:
				settings_gui.open();
				break;

			case T::CREATORS:
				main_menu->launch_creators_screen();
				break;

			case T::QUIT:
				should_quit = true;
				break;

			default: break;
		}
	};

	static auto do_ingame_menu_option = [&](const ingame_menu_button_type t) {
		using T = decltype(t);

		switch (t) {
			case T::RESUME:
				ingame_menu.show = false;
				break;

			case T::QUIT_TO_MENU:
				launch_setup(launch_type::MAIN_MENU);
				break;

			case T::SETTINGS:
				settings_gui.open();
				break;

			case T::QUIT:
				should_quit = true;
				break;

			default: break;
		}
	};

	static auto setup_pre_solve = [&](auto...) {
		renderer.save_debug_logic_step_lines_for_interpolation(DEBUG_LOGIC_STEP_LINES);
		DEBUG_LOGIC_STEP_LINES.clear();
	};

	/* 
		The audiovisual_step, advance_setup and advance_current_setup lambdas
		are separated only because MSVC outputs ICEs if they become nested.
	*/

	static visible_entities all_visible;

	static auto get_character_camera = [&]() -> character_camera {
		return { get_viewed_character(), { get_camera_eye(), window.get_screen_size() } };
	};

	static auto reacquire_visible_entities = [](
		const vec2i& screen_size,
		const const_entity_handle& viewed_character,
		const config_lua_table& viewing_config
	) {
		auto scope = measure_scope(frame_performance.camera_visibility_query);

		auto queried_eye = get_camera_eye();
		queried_eye.zoom /= viewing_config.session.camera_query_aabb_mult;

		const auto queried_cone = camera_cone(queried_eye, screen_size);

		all_visible.reacquire_all_and_sort({ 
			viewed_character.get_cosmos(), 
			queried_cone, 
			visible_entities_query::accuracy_type::PROXIMATE,
			get_render_layer_filter(),
			tree_of_npo_filter::all()
		});

		frame_performance.num_visible_entities.measure(all_visible.count_all());
	};

	static auto calc_pre_step_crosshair_displacement = [&](const auto& viewing_config) {
		return visit_current_setup([&](const auto& setup) {
			using T = remove_cref<decltype(setup)>;

			if constexpr(!std::is_same_v<T, main_menu_setup>) {
				const auto& total_collected = setup.get_entropy_accumulator();

				if (const auto motion = total_collected.calc_motion(
					get_viewed_character(), 
					game_motion_type::MOVE_CROSSHAIR,
					entropy_accumulator::input {
						viewing_config.input, 
						window.get_screen_size(), 
						get_camera_eye().zoom 
					}
				)) {
					return motion->offset;
				}
			}

			return vec2::zero;
		});
	};

	static auto audiovisual_step = [&](
		const augs::delta frame_delta,
		const double speed_multiplier,
		const config_lua_table& viewing_config
	) {
		const auto screen_size = window.get_screen_size();
		const auto viewed_character = get_viewed_character();
		const auto& cosm = viewed_character.get_cosmos();
		
		//get_audiovisuals().reserve_caches_for_entities(viewed_character.get_cosmos().get_solvable().get_entity_pool().capacity());
		
		auto& interp = get_audiovisuals().get<interpolation_system>();

		{
			auto scope = measure_scope(get_audiovisuals().performance.interpolation);

			interp.integrate_interpolated_transforms(
				viewing_config.interpolation, 
				cosm, 
				augs::delta(frame_delta) *= speed_multiplier, 
				cosm.get_fixed_delta()
			);
		}

		gameplay_camera.tick(
			screen_size,
			viewing_config.drawing.fog_of_war,
			interp,
			frame_delta,
			viewing_config.camera,
			viewed_character,
			calc_pre_step_crosshair_displacement(viewing_config)
		);

		reacquire_visible_entities(screen_size, viewed_character, viewing_config);

		const auto inv_tickrate = visit_current_setup([](const auto& setup) {
			return setup.get_inv_tickrate();
		});

		get_audiovisuals().advance(audiovisual_advance_input {
			frame_delta,
			speed_multiplier,
			inv_tickrate,

			get_character_camera(),
			all_visible,

			get_viewable_defs().particle_effects,
			cosm.get_logical_assets().plain_animations,

			streaming.loaded_sounds,

			viewing_config.audio_volume,
			viewing_config.sound
		});
	};

	static auto setup_post_solve = [&](
		const const_logic_step step, 
		const config_lua_table& viewing_config,
		const audiovisual_post_solve_settings settings
	) {
		{
			const auto& defs = get_viewable_defs();

			get_audiovisuals().standard_post_solve(step, { 
				defs.particle_effects, 
				streaming.loaded_sounds,
				viewing_config.audio_volume,
				viewing_config.sound,
				get_character_camera(),
				settings
			});
		}

		game_gui.standard_post_solve(
			step, 
			{ settings.prediction }
		);
	};

	static auto setup_post_cleanup = [&](const const_logic_step step) {
		(void)step;
	};

	static auto advance_setup = [&](
		const augs::delta frame_delta,
		auto& setup,
		const input_pass_result& result
	) {
		const auto& viewing_config = result.viewing_config;

		setup.control(result.motions);
		setup.control(result.intents);

		setup.accept_game_gui_events(game_gui.get_and_clear_pending_events());
		
		auto setup_audiovisual_post_solve = [&](const const_logic_step step, const audiovisual_post_solve_settings settings = {}) {
			setup_post_solve(step, viewing_config, settings);
		};

		{
			using S = remove_cref<decltype(setup)>;

			auto callbacks = solver_callbacks(
				setup_pre_solve,
				setup_audiovisual_post_solve,
				setup_post_cleanup
			);

			const auto zoom = get_camera_eye().zoom;

			if constexpr(std::is_same_v<S, client_setup>) {
				/* The client needs more goodies */

				setup.advance(
					{ 
						window.get_screen_size(), 
						viewing_config.input, 
						zoom,
						viewing_config.simulation_receiver, 
						viewing_config.lag_compensation, 
						network_performance,
						network_stats,
						get_audiovisuals().get<interpolation_system>(),
						get_audiovisuals().get<past_infection_system>()
					},
					callbacks
				);
			}
			else if constexpr(std::is_same_v<S, server_setup>) {
				setup.advance(
					{ 
						window.get_screen_size(), 
						viewing_config.input, 
						zoom,
						network_performance,
						server_stats
					},
					callbacks
				);
			}
			else {
				setup.advance(
					{ 
						frame_delta, 
						window.get_screen_size(), 
						viewing_config.input, 
						zoom 
					},
					callbacks
				);
			}
		}

		get_audiovisuals().randomizing.last_frame_delta = frame_delta;
		audiovisual_step(frame_delta, setup.get_audiovisual_speed(), viewing_config);
	};

	static auto advance_current_setup = [&](
		const augs::delta frame_delta,
		const input_pass_result& result
	) { 
		visit_current_setup(
			[&](auto& setup) {
				advance_setup(frame_delta, setup, result);
			}
		);
	};

	if (!params.editor_target.empty()) {
		launch_editor(lua, params.editor_target);
	}
	else if (params.start_server) {
		launch_setup(launch_type::SERVER);
	}
	else if (params.should_connect) {
		{
			const auto& target = params.connect_to_address;

			if (!target.empty()) {
				change_with_save([&](config_lua_table& cfg) {
					cfg.default_client_start.ip_port = params.connect_to_address;
				});
			}
		}

		launch_setup(launch_type::CLIENT);
	}
	else {
		launch_setup(config.get_launch_mode());
	}

	/* 
		The main loop variables.
	*/

	static augs::timer frame_timer;
	
	static release_flags releases;

	static auto make_create_game_gui_context = [&](const config_lua_table& viewing_config) {
		return [&]() {
			return game_gui.create_context(
				window.get_screen_size(),
				common_input_state,
				get_game_gui_subject(),
				create_game_gui_deps(viewing_config)
			);
		};
	};

	static auto make_create_menu_context = [&](const config_lua_table& cfg) {
		return [&](auto& gui) {
			return gui.create_context(
				window.get_screen_size(),
				common_input_state,
				create_menu_context_deps(cfg)
			);
		};
	};

	/* 
		MousePos is initially set to negative infinity.
	*/

	ImGui::GetIO().MousePos = { 0, 0 };

	LOG("Entered the main loop.");

	while (!should_quit) {
		ensure_float_flags_hold();

		auto scope = measure_scope(performance.fps);
		
#if PLATFORM_UNIX
		if (signal_status != 0) {
			const auto sig = signal_status;

			LOG("%x received.", strsignal(sig));

			if(
				sig == SIGINT
				|| sig == SIGSTOP
				|| sig == SIGTERM
			) {
				LOG("Gracefully shutting down.");
				should_quit = true;
				
				break;
			}
		}
#endif

		const auto frame_delta = frame_timer.extract_delta();

		/* 
			The centralized transformation of all window inputs.
			No window inputs will be acquired and/or used beyond the scope of this lambda,
			to the exception of remote packets, received by the client/server setups.
			
			This is necessary because we need some complicated interactions between multiple GUI contexts,
			primarily in deciding what events should be propagated further, down to the gameplay itself.
			It is the easiest if every possibility is considered in one place. 
			We have decided that some stronger decoupling here would benefit nobody.

			The lambda is called right away, like so: 
				result = [...](){...}().
			The result of the call, which is the collection of new game commands, will be passed further down the loop. 
		*/

		auto game_gui_mode = game_gui_mode_flag;

		if (setup_requires_cursor()) {
			game_gui_mode = true;
		}

		const auto result = [&]() -> input_pass_result {
			input_pass_result out;

			augs::local_entropy new_window_entropy;

			/* Generate release events if the previous frame so requested. */

			releases.append_releases(new_window_entropy, common_input_state);
			releases = {};

			if (get_viewed_character().dead()) {
				game_gui_mode = true;
			}

			const bool in_direct_gameplay =
				!game_gui_mode
				&& has_current_setup()
				&& !ingame_menu.show
			;

			{
				auto scope = measure_scope(performance.local_entropy);
				window.collect_entropy(new_window_entropy);
			}

			/*
				Top-level events, higher than IMGUI.
			*/
			
			{
				auto simulated_input_state = common_input_state;

				erase_if(new_window_entropy, [&](const augs::event::change e) {
					using namespace augs::event;
					using namespace augs::event::keys;

					simulated_input_state.apply(e);

					if (e.msg == message::deactivate) {
						releases.set_all();
					}

					if (e.is_exit_message()) {
						should_quit = true;
						return true;
					}
					
					if (e.was_pressed(key::F11)) {
						bool& f = config.window.fullscreen;
						f = !f;
						return true;
					}

					if (!ingame_menu.show) {
						if (visit_current_setup([&](auto& setup) {
							using T = remove_cref<decltype(setup)>;

							if constexpr(T::handles_window_input) {
								/* 
									Lets a setup fetch an input before IMGUI does,
									if for example IMGUI wants to capture keyboard input.	
								*/

								return setup.handle_input_before_imgui({
									simulated_input_state, e, window
								});
							}

							return false;
						})) {
							return true;
						}
					}

					return false;
				});
			}

			/* 
				IMGUI is our top GUI whose priority precedes everything else. 
				It will eat from the window input vector that is later passed to the game and other GUIs.	
			*/

			configurables.sync_back_into(config);

			/*
				We "pause" the mouse cursor's position when we are in direct gameplay,
				so that when switching to GUI, the cursor appears exactly where it had disappeared.
				(it does not physically freeze the cursor, it just remembers the position)
			*/

			window.set_mouse_pos_paused(in_direct_gameplay);

			perform_imgui_pass(
				new_window_entropy,
				window.get_screen_size(),
				frame_delta,
				config,
				last_saved_config,
				local_config_path,
				settings_gui,
				lua,
				[&]() {
					if (!has_current_setup()) {
						if (start_client_gui.perform(window, config.default_client_start, config.client) || client_start_requested) {
							client_start_requested = false;

							change_with_save(
								[&](auto& cfg) {
									cfg.default_client_start = config.default_client_start;
									cfg.client = config.client;
								}
							);

							launch_setup(launch_type::CLIENT);
						}

						if (start_server_gui.perform(config.default_server_start) || server_start_requested) {
							server_start_requested = false;

							change_with_save(
								[&](auto& cfg) {
									cfg.default_server_start = config.default_server_start;
									cfg.client = config.client;
								}
							);

							if (start_server_gui.instance_type == server_instance_type::INTEGRATED) {
								launch_setup(launch_type::SERVER);
							}
							else {
								augs::spawn_detached_process(params.exe_path.string(), "--dedicated-server");
								config.default_client_start.ip_port = typesafe_sprintf("%x:%x", config.default_server_start.ip, config.default_server_start.port);

								launch_setup(launch_type::CLIENT);
							}
						}
					}

					/*
						The editor setup might want to use IMGUI to create views of entities or resources,
						thus we ask the current setup for its custom ImGui logic.

						Similarly, client and server setups might want to perform ImGui for things like team selection.
					*/

					visit_current_setup([&](auto& setup) {
						const auto result = setup.perform_custom_imgui({ 
							lua, window, streaming.images_in_atlas, config 
						});

						if (result == custom_imgui_result::GO_TO_MAIN_MENU) {
							launch_setup(launch_type::MAIN_MENU);
						}
					});

					if (last_exit_incorrect_popup != std::nullopt) {
						if (last_exit_incorrect_popup->perform()) {
							last_exit_incorrect_popup = std::nullopt;
						}
					}
				},

				/* Flags controlling IMGUI behaviour */

				ingame_menu.show,
				has_current_setup(),

				in_direct_gameplay,
				float_tests_succeeded
			);
			
			const auto viewing_config = visit_current_setup([&](auto& setup) {
				auto config_copy = config;

				/*
					For example, the main menu might want to disable HUD or tune down the sound effects.
					Editor might want to change the window name to the current file.
				*/

				setup.customize_for_viewing(config_copy);
				setup.apply(config_copy);

				if (get_camera_eye().zoom < 1.f) {
					/* Force linear filtering when zooming out */
					config_copy.renderer.default_filtering = augs::filtering_type::LINEAR;
				}

				return config_copy;
			});

			out.viewing_config = viewing_config;

			configurables.apply(viewing_config);

			if (window.is_active()
				&& (
					in_direct_gameplay
					|| (
						viewing_config.window.raw_mouse_input
#if TODO
						&& !viewing_config.session.use_system_cursor_for_gui
#endif
					)
				)
			) {
				window.clip_system_cursor();
				window.set_cursor_visible(false);
			}
			else {
				window.disable_cursor_clipping();
				window.set_cursor_visible(true);
			}

			releases.set_due_to_imgui(ImGui::GetIO());

			auto create_menu_context = make_create_menu_context(viewing_config);
			auto create_game_gui_context = make_create_game_gui_context(viewing_config);

			/*
				Since ImGUI has quite a different philosophy about input,
				we will need some ugly inter-op with our GUIs.
			*/

			if (ImGui::GetIO().WantCaptureMouse) {
				/* 
					If mouse enters any IMGUI element, rewrite ImGui's mouse position to common_input_state.

					This allows us to keep common_input_state up to date, 
					because mousemotions are eaten from the vector already due to ImGui wanting mouse.
				*/

				common_input_state.mouse.pos = ImGui::GetIO().MousePos;

				/* Neutralize hovers on all GUIs whose focus may have just been stolen. */

				game_gui.world.unhover_and_undrag(create_game_gui_context());
				
				if (has_main_menu()) {
					main_menu->gui.world.unhover_and_undrag(create_menu_context(main_menu->gui));
				}

				ingame_menu.world.unhover_and_undrag(create_menu_context(ingame_menu));

				on_specific_setup([](editor_setup& setup) {
					setup.unhover();
				});
			}

			/*
				We also need inter-op between our own GUIs, 
				since we have more than just one.
			*/

			if (game_gui_mode && should_draw_game_gui() && game_gui.world.wants_to_capture_mouse(create_game_gui_context())) {
				if (current_setup) {
					if (auto* editor = std::get_if<editor_setup>(&*current_setup)) {
						editor->unhover();
					}
				}
			}

			/* Maybe the game GUI was deactivated while the button was still hovered. */

			else if (!game_gui_mode && has_current_setup()) {
				game_gui.world.unhover_and_undrag(create_game_gui_context());
			}

			/* Distribution of all the remaining input happens here. */

			for (const auto e : new_window_entropy) {
				using namespace augs::event;
				using namespace keys;
				
				/* Now is the time to actually track the input state. */
				common_input_state.apply(e);

				if (e.was_pressed(key::ESC)) {
					if (has_current_setup()) {
						if (ingame_menu.show) {
							ingame_menu.show = false;
						}
						else if (!visit_current_setup([&](auto& setup) {
							switch (setup.escape()) {
								case setup_escape_result::LAUNCH_INGAME_MENU: ingame_menu.show = true; return true;
								case setup_escape_result::JUST_FETCH: return true;
								case setup_escape_result::GO_TO_MAIN_MENU: launch_setup(launch_type::MAIN_MENU); return true;
								default: return false;
							}
						})) {
							/* Setup ignored the ESC button */
							ingame_menu.show = true;
						}

						releases.set_all();
					}

					continue;
				}

				const auto key_change = ::to_intent_change(e.get_key_change());

				const bool was_pressed = key_change == intent_change::PRESSED;
				const bool was_released = key_change == intent_change::RELEASED;
				
				if (was_pressed || was_released) {
					const auto key = e.get_key();

					if (const auto it = mapped_or_nullptr(viewing_config.app_controls, key)) {
						if (was_pressed) {
							handle_app_intent(*it);
							continue;
						}
					}
				}

				{
					auto control_main_menu = [&]() {
						if (has_main_menu() && !has_current_setup()) {
							if (main_menu->gui.show) {
								main_menu->gui.control(create_menu_context(main_menu->gui), e, do_main_menu_option);
							}

							return true;
						}

						return false;
					};

					auto control_ingame_menu = [&]() {
						if (ingame_menu.show || was_released) {
							return ingame_menu.control(create_menu_context(ingame_menu), e, do_ingame_menu_option);
						}

						return false;
					};
					
					if (was_released) {
						control_main_menu();
						control_ingame_menu();
					}
					else {
						if (control_main_menu()) {
							continue;
						}

						if (control_ingame_menu()) {
							continue;
						}

						/* Prevent e.g. panning in editor when the ingame menu is on */
						if (ingame_menu.show) {
							continue;
						}
					}
				}

				{
					if (visit_current_setup([&](auto& setup) {
						using T = remove_cref<decltype(setup)>;

						if constexpr(T::handles_window_input) {
							if (!streaming.necessary_images_in_atlas.empty()) {
								/* Viewables reloading happens later so it might not be ready yet */

								const auto& app_ingame_controls = viewing_config.app_ingame_controls;

								return setup.handle_input_before_game({
									app_ingame_controls, streaming.necessary_images_in_atlas, common_input_state, e, window
								});
							}
						}

						return false;
					})) {
						continue;
					}
				}

				const auto viewed_character = get_viewed_character();

				if (was_released || (has_current_setup() && !ingame_menu.show)) {
					const bool direct_gameplay = viewed_character.alive() && !game_gui_mode;
					const bool game_gui_effective = viewed_character.alive() && game_gui_mode;

					if (was_released || was_pressed) {
						const auto key = e.get_key();

						if (was_released || direct_gameplay || game_gui_effective) {
							if (const auto it = mapped_or_nullptr(viewing_config.app_ingame_controls, key)) {
								if (was_pressed) {
									if (handle_app_ingame_intent(*it)) {
										continue;
									}
								}
							}
							if (const auto it = mapped_or_nullptr(viewing_config.game_gui_controls, key)) {
								if (should_draw_game_gui()) {
									game_gui.control_hotbar_and_action_button(get_game_gui_subject(), { *it, *key_change });

									if (was_pressed) {
										continue;
									}
								}
							}
						}

						if (const auto it = mapped_or_nullptr(viewing_config.game_controls, key)) {
							if (e.uses_mouse() && game_gui_effective) {
								/* Leave it for the game gui */
							}
							else {
								out.intents.push_back({ *it, *key_change });

								if (was_pressed) {
									continue;
								}
							}
						}
					}

					if (direct_gameplay && e.msg == message::mousemotion) {
						raw_game_motion m;
						m.motion = game_motion_type::MOVE_CROSSHAIR;
						m.offset = e.data.mouse.rel;

						out.motions.emplace_back(m);
						continue;
					}

					if (was_released || should_draw_game_gui()) {
						if (game_gui.control_gui_world(create_game_gui_context(), e)) {
							continue;
						}
					}
				}
			}

			/* 
				Notice that window inputs do not propagate
				beyond the closing of this scope.
			*/

			return out;
		}();

		const auto& new_viewing_config = result.viewing_config;

		/* 
			Viewables reloading pass.
		*/

		visit_current_setup(
			[&](const auto& setup) {
				using T = remove_cref<decltype(setup)>;
				using S = viewables_loading_type;

				constexpr auto s = T::loading_strategy;

				if constexpr(s == S::LOAD_ALL) {
					load_all(setup.get_viewable_defs());
				}
				else if constexpr(s == S::LOAD_ONLY_NEAR_CAMERA) {
					static_assert(always_false_v<T>, "Unimplemented");
				}
				else if constexpr(T::loading_strategy == S::LOAD_ALL_ONLY_ONCE) {
					/* Do nothing */
				}
				else {
					static_assert(always_false_v<T>, "Unknown viewables loading strategy.");
				}
			}
		);

		streaming.finalize_load({
			new_viewing_config.debug.measure_atlas_uploading,
			renderer,
			get_audiovisuals().get<sound_system>()
		});

		const auto screen_size = window.get_screen_size();

		auto create_menu_context = make_create_menu_context(new_viewing_config);
		auto create_game_gui_context = make_create_game_gui_context(new_viewing_config);

		/* 
			Advance the current setup's logic,
			and let the audiovisual_state sample the game world 
			that it chooses via get_viewed_cosmos.

			This also advances the audiovisual state, based on the cosmos returned by the setup.
		*/

		{
			auto scope = measure_scope(frame_performance.advance_setup);
			advance_current_setup(frame_delta, result);
		}
		
		/*
			Game GUI might have been altered by the step's post-solve,
			therefore we need to rebuild its layouts (and from them, the tree data)
			for immediate visual response.
		*/

		if (should_draw_game_gui()) {
			const auto context = create_game_gui_context();

			game_gui.advance(context, frame_delta);
			game_gui.rebuild_layouts(context);
			game_gui.build_tree_data(context);
		}

		/* 
			What follows is strictly view part,
			without advancement of any kind.
			
			No state is altered beyond this point,
			except for usage of graphical resources and profilers.
		*/

		if (/* minimized */ screen_size.is_zero()) {
			continue;
		}

		auto frame = measure_scope(frame_performance.total);
		
		auto get_drawer = [&]() { 
			return augs::drawer_with_default {
				renderer.get_triangle_buffer(),
				streaming.necessary_images_in_atlas[assets::necessary_image_id::BLANK]
			};
		};

		auto get_line_drawer = [&]() { 
			return augs::line_drawer_with_default {
				renderer.get_line_buffer(),
				streaming.necessary_images_in_atlas[assets::necessary_image_id::BLANK]
			};
		};

		const auto interpolation_ratio = visit_current_setup([](auto& setup) {
			return setup.get_interpolation_ratio();
		});

		const auto context = viewing_game_gui_context {
			create_game_gui_context(),

			{
				get_audiovisuals().get<interpolation_system>(),
				get_audiovisuals().world_hover_highlighter,
				new_viewing_config.hotbar,
				new_viewing_config.drawing,
				new_viewing_config.game_gui_controls,
				get_camera_eye(),
				get_drawer()
			}
		};

		/*
			Canonical rendering order of the Hypersomnia Universe:
			
			1.  Draw the cosmos in the vicinity of the viewed character.
				Both the cosmos and the character are specified by the current setup (main menu is a setup, too).
			
			2.	Draw the debug lines over the game world, if so is appropriate.
			
			3.	Draw the game GUI, if so is appropriate.
				Game GUI involves things like inventory buttons, hotbar and health bars.

			4.  Draw the mode GUI.
				Mode GUI involves things like team selection, weapon shop, round time remaining etc.

			5.	Draw either the main menu buttons, or the in-game menu overlay accessed by ESC.
				These two are almost identical, except the layouts of the first (e.g. tweened buttons) 
				may also be influenced by a playing intro.

			6.	Draw IMGUI, which is the highest priority GUI. 
				This involves settings window, developer console and the like.

			7.	Draw the GUI cursor. It may be:
					- The cursor of the IMGUI, if it wants to capture the mouse.
					- Or, the cursor of the main menu or the in-game menu overlay, if either is currently active.
					- Or, the cursor of the game gui, with maybe tooltip, with maybe dragged item's ghost, if we're in-game in GUI mode.
		*/

		if (streaming.general_atlas == std::nullopt) {
			/* Set imgui atlas as a fallback texture as it has some white pixels */
			imgui_atlas.set_as_current();
		}

		renderer.set_viewport({ vec2i{0, 0}, screen_size });

		const bool rendering_flash_afterimage = gameplay_camera.is_flash_afterimage_requested();

		if (rendering_flash_afterimage) {
			necessary_fbos.flash_afterimage->set_as_current();
		}
		else if (augs::graphics::fbo::current_exists()) {
			augs::graphics::fbo::set_current_to_none();
		}

		renderer.clear_current_fbo();

		const auto viewed_character = get_viewed_character();

		if (const auto& viewed_cosmos = viewed_character.get_cosmos();
			std::addressof(viewed_cosmos) != std::addressof(cosmos::zero)
		) {
			const auto cone = camera_cone(get_camera_eye(), screen_size);

			{
				/* #1 */
				auto scope = measure_scope(frame_performance.rendering_script);

				thread_local std::vector<additional_highlight> highlights;
				highlights.clear();

				visit_current_setup([&](const auto& setup) {
					using T = remove_cref<decltype(setup)>;

					if constexpr(T::has_additional_highlights) {
						setup.for_each_highlight([&](auto&&... args) {
							highlights.push_back({ std::forward<decltype(args)>(args)... });
						});
					}
				});

				thread_local std::vector<special_indicator> special_indicators;
				special_indicators.clear();
				special_indicator_meta indicator_meta;

				if (viewed_character) {
					visit_current_setup([&](const auto& setup) {
						setup.on_mode_with_input(
							[&](const auto&... args) {
								::gather_special_indicators(
									args..., 
									viewed_character.get_official_faction(), 
									streaming.necessary_images_in_atlas, 
									special_indicators,
									indicator_meta,
									viewed_character
								);
							}
						);
					});
				}

				illuminated_rendering({
					{ viewed_character, cone },
					calc_pre_step_crosshair_displacement(new_viewing_config),
					new_viewing_config.session.camera_query_aabb_mult,
					get_audiovisuals(),
					new_viewing_config.drawing,
					streaming.necessary_images_in_atlas,
					streaming.get_loaded_gui_fonts().gui,
					streaming.images_in_atlas,
					interpolation_ratio,
					renderer,
					frame_performance,
					streaming.general_atlas,
					necessary_fbos,
					necessary_shaders,
					all_visible,
					new_viewing_config.performance,
					highlights,
					special_indicators,
					indicator_meta
				});
			}

			if (DEBUG_DRAWING.enabled) {
				/* #2 */
				auto scope = measure_scope(frame_performance.debug_lines);

				draw_debug_lines(
					viewed_cosmos,
					renderer,
					interpolation_ratio,
					get_drawer().default_texture,
					new_viewing_config,
					cone
				);
			}

			auto scope = measure_scope(frame_performance.game_gui);

			necessary_shaders.standard->set_projection(augs::orthographic_projection(vec2(screen_size)));

			/*
				Illuminated rendering leaves the renderer in a state
				where the default shader is being used and the game world atlas is still bound.

				It is the configuration required for further viewing of GUI.
			*/

			if (should_draw_game_gui()) {
				/* #3 */
				game_gui.world.draw(context);
			}

			const auto player_metas = visit_current_setup([&](auto& setup) {
				return setup.find_player_metas();
			});

			/* #4 */
			visit_current_setup([&](auto& setup) {
				setup.draw_custom_gui({
					all_visible,
					cone,
					get_drawer(),
					get_line_drawer(),
					new_viewing_config,
					streaming.necessary_images_in_atlas,
					streaming.general_atlas,
					streaming.avatar_atlas,
					streaming.images_in_atlas,
					streaming.avatars_in_atlas,
					renderer,
					common_input_state.mouse.pos,
					screen_size,
					streaming.get_loaded_gui_fonts(),
					necessary_sounds,
					player_metas
				});

				renderer.call_and_clear_lines();
			});
		}
		else {
			if (streaming.general_atlas != std::nullopt) {
				streaming.general_atlas->set_as_current();
			}

			necessary_shaders.standard->set_as_current();
			necessary_shaders.standard->set_projection(augs::orthographic_projection(vec2(screen_size)));

			get_drawer().color_overlay(screen_size, darkgray);
		}

		const auto menu_chosen_cursor = [&](){
			auto scope = measure_scope(frame_performance.menu_gui);

			if (has_current_setup()) {
				if (ingame_menu.show) {
					const auto context = create_menu_context(ingame_menu);
					ingame_menu.advance(context, frame_delta);

					/* #5 */
					return ingame_menu.draw({ context, get_drawer() });
				}

				return assets::necessary_image_id::INVALID;
			}
			else {
				const auto context = create_menu_context(main_menu->gui);

				main_menu->gui.advance(context, frame_delta);

#if MENU_ART
				get_drawer().aabb(streaming.necessary_images_in_atlas[assets::necessary_image_id::ART_1], ltrb(0, 0, screen_size.x, screen_size.y), white);
#endif

				/* #5 */
				const auto cursor = main_menu->gui.draw({ context, get_drawer() });

				main_menu->draw_overlays(
					get_drawer(),
					streaming.necessary_images_in_atlas,
					streaming.get_loaded_gui_fonts().gui,
					screen_size
				);

				return cursor;
			}
		}();
		
		renderer.call_and_clear_triangles();

		{
			/* #6 */
			auto scope = measure_scope(frame_performance.imgui);

			const augs::graphics::texture* game_world_atlas = nullptr;
			const augs::graphics::texture* avatar_atlas = nullptr;
			const augs::graphics::texture* avatar_preview_atlas = nullptr;

			if (streaming.general_atlas.has_value()) {
				game_world_atlas = std::addressof(streaming.general_atlas.value());
			}
			
			if (streaming.avatar_atlas.has_value()) {
				avatar_atlas = std::addressof(streaming.avatar_atlas.value());
			}

			if (start_client_gui.avatar_preview_tex.has_value()) {
				avatar_preview_atlas = std::addressof(start_client_gui.avatar_preview_tex.value());
			}

			renderer.draw_call_imgui(imgui_atlas, game_world_atlas, avatar_atlas, avatar_preview_atlas);
		}

		{
			/* #7 */

			const bool should_draw_our_cursor = new_viewing_config.window.raw_mouse_input && !window.is_mouse_pos_paused();
			const auto cursor_drawing_pos = common_input_state.mouse.pos;

			if (ImGui::GetIO().WantCaptureMouse) {
				if (should_draw_our_cursor) {
					get_drawer().cursor(streaming.necessary_images_in_atlas, augs::imgui::get_cursor<assets::necessary_image_id>(), cursor_drawing_pos, white);
				}
			}
			else if (menu_chosen_cursor != assets::necessary_image_id::INVALID) {
				/* We must have drawn some menu */

				if (should_draw_our_cursor) {
					get_drawer().cursor(streaming.necessary_images_in_atlas, menu_chosen_cursor, cursor_drawing_pos, white);
				}
			}
			else if (game_gui_mode && should_draw_game_gui()) {
				if (viewed_character) {
					const auto& character_gui = game_gui.get_character_gui(get_game_gui_subject());

					character_gui.draw_cursor_with_tooltip(context, should_draw_our_cursor);
				}
			}
			else {
				if (should_draw_our_cursor) {
					on_specific_setup([&](editor_setup& setup) {
						if (setup.is_editing_mode()) {
							get_drawer().cursor(streaming.necessary_images_in_atlas, assets::necessary_image_id::GUI_CURSOR, cursor_drawing_pos, white);
						}
					});
				}
			}
		}

		if (streaming.general_atlas != std::nullopt) {
			const auto flash_mult = gameplay_camera.get_effective_flash_mult();

			if (flash_mult > 0 || rendering_flash_afterimage) {
				/* 
					While capturing, overlay the afterimage onto the final fbo and go back to the default fbo. 
					This is to avoid a single black frame before the afterimage actually kicks in.
				*/

				if (rendering_flash_afterimage) {
					augs::graphics::fbo::set_current_to_none();
				}

				necessary_fbos.flash_afterimage->get_texture().set_as_current();
				necessary_shaders.flash_afterimage->set_as_current();
				necessary_shaders.flash_afterimage->set_projection(augs::orthographic_projection(vec2(screen_size)));

				{
					auto flash_afterimage_col = white;
					flash_afterimage_col.mult_alpha(flash_mult);

					auto whole_texture = augs::atlas_entry();
					whole_texture.atlas_space.x = 0.f;
					whole_texture.atlas_space.y = 0.f;
					whole_texture.atlas_space.w = 1.f;
					whole_texture.atlas_space.h = 1.f;

					get_drawer().color_overlay(whole_texture, screen_size, flash_afterimage_col, flip_flags::make_vertically());
				}

				renderer.call_and_clear_triangles();

				necessary_shaders.standard->set_as_current();
				streaming.general_atlas->set_as_current();
			}

			if (!rendering_flash_afterimage) {
				/* Don't draw white blinding overlay while we are capturing the flash afterimage */

				if (flash_mult > 0) {
					renderer.call_and_clear_triangles();

					auto flash_overlay_col = white;

					if (viewed_character) {
						if (const auto sentience = viewed_character.find<components::sentience>()) {
							if (!sentience->is_conscious()) {
								flash_overlay_col = rgba(255, 40, 40, 255);
							}
						}
					}

					flash_overlay_col.mult_alpha(flash_mult);

					get_drawer().color_overlay(screen_size, flash_overlay_col);
				}
			}
		}

		if (new_viewing_config.session.show_developer_console) {
			auto scope = measure_scope(frame_performance.debug_details);

			draw_debug_details(
				get_drawer(),
				streaming.get_loaded_gui_fonts().gui,
				screen_size,
				viewed_character,
				frame_performance,
				network_performance,
				network_stats,
				server_stats,
				streaming.performance,
				streaming.general_atlas_performance,
				performance,
				get_audiovisuals().performance
			);
		}

		renderer.call_and_clear_triangles();

		frame_performance.num_triangles.measure(renderer.num_total_triangles_drawn);
		renderer.num_total_triangles_drawn = 0u;

		if (!until_first_swap_measured) {
			LOG("Time until first swap: %x ms", until_first_swap.extract<std::chrono::milliseconds>());
			until_first_swap_measured = true;
		}

		{
			auto scope = measure_scope(performance.swap_buffers);
			window.swap_buffers();
		}
	}

	return EXIT_SUCCESS;
}
catch (const config_read_error& err) {
	LOG("Failed to read the initial config for the game!\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::imgui_init_error& err) {
	LOG("Failed init imgui:\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::audio_error& err) {
	LOG("Failed to establish the audio context:\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::window_error& err) {
	LOG("Failed to create an OpenGL window:\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::renderer_error& err) {
	LOG("Failed to initialize the renderer: %x", err.what());
	return EXIT_FAILURE;
}
catch (const necessary_resource_loading_error& err) {
	LOG("Failed to load a resource necessary for the game to function!\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::lua_state_creation_error& err) {
	LOG("Failed to create a lua state for the game!\n%x", err.what());
	return EXIT_FAILURE;
}
catch (const augs::unit_test_session_error& err) {
	LOG("Unit test session failure:\n%x\ncout:%x\ncerr:%x\nclog:%x\n", 
		err.what(), err.cout_content, err.cerr_content, err.clog_content
	);

	return EXIT_FAILURE;
}
catch (const augs::too_many_sound_sources_error& err) {
	LOG("std::runtime_error thrown: %x", err.what());

	return EXIT_FAILURE;
}
catch (const augs::filesystem_error& err) {
	LOG("std::filesystem_error thrown: %x\npath1: %x\npath2: %x", err.what(), err.path1(), err.path2());

	return EXIT_FAILURE;
}
catch (const entity_creation_error& err) {
	LOG("Unhandled entity creation error: %x", format_enum(err.type));

	return EXIT_FAILURE;
}