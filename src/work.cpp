#include <cstdint>
/*
	BEFORE YOU CONTINUE:

	If it's your first time reading the codebase...
	work.cpp is arguably the most complex source file in Hypersomnia, boasting over 4000 lines of code.

	Why?

	It's where the top-most level flow control happens.
	Examples of what you'll find here:

	- Initializing window, audio, renderers and passing them by reference to whatever subsystems might require them.
	- Client connecting from the command line.
	- Interactions between all the top-level app modules like e.g. editor hosting a server, tutorial exiting to the main menu etc.
	- Keeping the separately running game thread that continuously produces frames to be rendered on the main thread.
	- Inputs being propagated to either gui or gameplay.
	- Running the main loop:
		- waiting on a game thread to produce a frame,
		- calling GL commands on the main thread,
		- finally swapping window buffers.

	It would be extremely counterproductive to separate it into many files because the above tasks are inherently interconnected.
	I believe it's the simplest to just have everything that relates to the main game loop in a single function: "work".
*/

#if PLATFORM_UNIX
#include <csignal>
#endif

#include <functional>

#include "augs/templates/enum_introspect.h"
#include "augs/templates/thread_templates.h"

#include "fp_consistency_tests.h"

#include "augs/window_framework/shell.h"
#include "augs/log_path_getters.h"
#include "augs/unit_tests.h"
#include "augs/global_libraries.h"

#include "augs/templates/identity_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/history.hpp"
#include "augs/templates/traits/in_place.h"
#include "augs/templates/thread_pool.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "augs/misc/date_time.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/mutex.h"
#include "augs/misc/future.h"

#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"

#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"

#include "application/session_profiler.h"
#include "application/config_json_table.h"

#include "application/gui/headless_map_catalogue.h"

#if HEADLESS

#else
#include "view/rendering_scripts/illuminated_rendering.h"
#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/streaming/viewables_streaming.h"
#include "view/frame_profiler.h"
#include "view/shader_paths.h"

#include "view/game_gui/game_gui_system.h"

#include "augs/graphics/renderer.h"
#include "augs/graphics/renderer_backend.h"

#include "augs/window_framework/shell.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/audio/audio_context.h"
#include "augs/audio/audio_command_buffers.h"
#include "augs/drawing/drawing.hpp"

#include "view/audiovisual_state/world_camera.h"
#include "view/audiovisual_state/audiovisual_state.h"

#include "application/gui/settings_gui.h"
#include "application/gui/start_client_gui.h"
#include "application/gui/start_server_gui.h"
#if BUILD_NETWORKING
#include "application/gui/browse_servers_gui.h"
#endif
#include "application/gui/map_catalogue_gui.h"
#include "application/gui/leaderboards_gui.h"
#include "application/gui/social_sign_in_gui.h"
#include "application/gui/ingame_menu_gui.h"
#include "application/setups/editor/editor_paths.h"

#include "application/main/imgui_pass.h"
#include "application/main/draw_debug_details.h"
#include "application/main/draw_debug_lines.h"
#include "application/main/release_flags.h"
#include "application/main/flash_afterimage.h"
#include "application/main/abortable_popup.h"
#include "application/setups/draw_setup_gui_input.h"
#include "view/game_gui/special_indicator_logic.h"
#include "augs/misc/imgui/simple_popup.h"
#include "application/main/game_frame_buffer.h"
#include "application/main/cached_visibility_data.h"
#include "augs/graphics/frame_num_type.h"
#include "view/rendering_scripts/launch_visibility_jobs.h"
#include "view/rendering_scripts/for_each_vis_request.h"
#include "view/hud_messages/hud_messages_gui.h"
#include "application/input/input_pass_result.h"

#include "application/setups/client/demo_paths.h"

#include "steam_integration_callbacks.h"
#endif

#include "steam_integration.h"
#include "steam_integration_helpers.hpp"

#if HEADLESS
#include "application/setups/server/server_setup.h"
#else
#include "application/setups/all_setups.h"
#include "application/setups/editor/editor_setup_for_each_highlight.hpp"
#endif

#include "application/masterserver/masterserver.h"
#include "application/network/network_common.h"

#if BUILD_DEBUGGER_SETUP
#include "application/setups/debugger/debugger_player.hpp"
#endif

#include "application/nat/nat_detection_session.h"

#include "application/network/address_utils.h"
#include "augs/network/netcode_socket_raii.h"

#include "cmd_line_params.h"
#include "build_info.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/window_framework/create_process.h"
#include "game/cosmos/for_each_entity.h"
#include "application/nat/stun_server_provider.h"
#include "application/arena/arena_paths.h"

#include "application/main/self_updater.h"
#include "application/setups/editor/packaged_official_content.h"
#include "augs/string/parse_url.h"
#if PLATFORM_WEB
#include "augs/templates/main_thread_queue.h"
#include "augs/misc/httplib_utils.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/string/parse_url.h"
#endif

#include "application/gui/make_random_nickname.hpp"
#include "make_canon_config.hpp"

#if BUILD_WEBRTC
#include "rtc/rtc.hpp"
#endif

#include "augs/readwrite/file_to_bytes.h"
#if !PLATFORM_WEB
#include "application/main/dedicated_server_worker.hpp"
#endif
#include "work_result.h"

namespace augs {
	void sleep(double secs);
}

std::function<void()> ensure_handler;

extern augs::mutex log_mutex;
extern std::string log_timestamp_format;

#if PLATFORM_UNIX
std::atomic<int> signal_status = 0;
static_assert(std::atomic<int>::is_always_lock_free);
#endif

float max_zoom_out_at_edges_v = 0.7f;

#if HEADLESS

#else
constexpr bool no_edge_zoomout_v = false;
#endif

#if PLATFORM_WEB
#include <emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, call_hideProgress, (), {
	hideProgress();
});

EM_JS(bool, call_try_fetch_initial_user, (), {
  return try_fetch_initial_user();
});

EM_JS(void, call_setBrowserLocation, (const char* newPath), {
	Module.setBrowserLocation(newPath);
});

EM_JS(void, call_get_user_geolocation, (), {
	Module.getUserGeolocation();
});

double web_get_secs_until_next_weekend_evening(const char* locationId) {
	return EM_ASM_DOUBLE({
		const locationIdStr = UTF8ToString($0);
		return get_secs_until_next_weekend_evening(locationIdStr);
	}, locationId);
}

extern augs::mutex open_url_on_main_lk;
extern std::string open_url_on_main;

EM_JS(void, call_openUrl, (const char* newPath), {
	openUrl(newPath);
});

#include "application/main/auth_providers.h"

extern augs::mutex lat_lon_mutex;
extern std::optional<double> player_latitude;
extern std::optional<double> player_longitude;

extern "C" {
	EMSCRIPTEN_KEEPALIVE
	void on_geolocation_received(double lat, double lon) {
		LOG("on_geolocation_received: lat: %x, lon: %x", lat, lon);

		{
			auto lock = augs::scoped_lock(lat_lon_mutex);

			player_latitude = lat;
			player_longitude = lon;
		}

		LOG("on_geolocation_received end.");
	}
}

#endif

enum class ad_state_type {
	NONE,
	REQUESTING,
	PLAYING
} ad_state = ad_state_type::NONE;

#if PLATFORM_WEB
EM_JS(char*, crazygames_get_invite_link, (), {
    var gameLink = Module.cg_invite_link || "";
    var length = lengthBytesUTF8(gameLink) + 1;
    var buffer = _malloc(length);
    stringToUTF8(gameLink, buffer, length);
    return buffer;
});

EM_JS(void, call_sdk_invite_link, (const char* connect_string), {
	Module.sdk_invite_link(connect_string);
});

void web_sdk_invite_link(const std::string& connect_string) {
	call_sdk_invite_link(connect_string.c_str());
}

void web_sdk_happy_time() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.sdk_happy_time();
		});
	});
}

void web_sdk_gameplay_start() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.sdk_gameplay_start();
		});
	});
}

void web_sdk_gameplay_stop() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.sdk_gameplay_stop();
		});
	});
}

void web_sdk_loading_start() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.sdk_loading_start();
		});
	});
}

void web_sdk_loading_stop() {
	main_thread_queue::execute([&]() {
		EM_ASM({
			Module.sdk_loading_stop();
		});
	});
}

extern "C" {
	EMSCRIPTEN_KEEPALIVE
	void on_ad_started() {
		ad_state = ad_state_type::PLAYING;
	}

	EMSCRIPTEN_KEEPALIVE
	void on_ad_ended() {
		ad_state = ad_state_type::NONE;
	}
}

EM_JS(bool, call_sdk_request_ad, (), {
	return Module.sdk_request_ad();
});

void web_sdk_request_ad() {
	if (ad_state != ad_state_type::NONE) {
		return;
	}

	ad_state = ad_state_type::REQUESTING;

	main_thread_queue::execute([&]() {
		if (!call_sdk_request_ad()) {
			ad_state = ad_state_type::NONE;
		}
	});
}

#else
const char* crazygames_get_invite_link() { return nullptr; }
void web_sdk_gameplay_start() {}
void web_sdk_happy_time() { LOG("web_sdk_happy_time"); }
void web_sdk_gameplay_stop() {}
void web_sdk_loading_start() {}
void web_sdk_loading_stop() {}
void web_sdk_invite_link(const std::string&) {}
void web_sdk_request_ad() {}
#endif

#include "augs/persistent_filesystem.hpp"

#if PLATFORM_WEB
#define WEBSTATIC static
#else
#define WEBSTATIC 
#endif

#if PLATFORM_WEB
randomization netcode_rng;
augs::mutex rng_lk;

extern "C" {
	uint32_t randombytes_external(void) {
		auto lock = augs::scoped_lock(rng_lk);
		return netcode_rng.random<uint32_t>();
	}
}
#endif

#if PLATFORM_WEB
#include "application/main/check_token_still_valid.hpp"
augs::future<bool> token_still_valid_check;
#endif

work_result work(
	const cmd_line_params& parsed_params,
	const bool log_directory_existed,
	const int argc,
	const char* const * const argv
) try {
#if PLATFORM_WEB
	main_thread_queue::save_main_thread_id();
#endif

	web_sdk_loading_start();

	WEBSTATIC const auto params = [&parsed_params]() {
		auto p = parsed_params;

#if PLATFORM_WEB
		if (p.is_crazygames) {
			const auto invite_link = crazygames_get_invite_link();

			if (invite_link) {
				LOG_NVPS(invite_link);

				const auto link = std::string(invite_link);

				if (!link.empty()) {
					p.set_connect(link);
				}
			}

			free(invite_link);
		}
#endif

		return p;
	}();

	(void)argc;
	(void)argv;
	(void)log_directory_existed;

#if PLATFORM_WEB
	call_get_user_geolocation();

	WEBSTATIC const bool is_steam_client = false;

#if WEB_ITCH
	WEBSTATIC const bool ranked_servers_enabled = false;
#else
	WEBSTATIC const bool ranked_servers_enabled = true;
#endif

#else
	WEBSTATIC const bool is_cli_tool = params.is_cli_tool();

	if (is_cli_tool) {
		LOG("Launching a CLI tool. Skipping Steam API initialization.");
		LOG("WARNING: CLI tools will always read \"user\",\n instead of the folder named after the SteamID");
	}
	else {
		const bool running_from_appimage = !params.appimage_path.empty();

		if (!running_from_appimage) {
			/* 
				Creating steam_appid.txt will fail on a read-only system.
			*/

#if CREATE_STEAM_APPID
			LOG("Creating steam_appid.txt");

			augs::save_as_text("steam_appid.txt", std::to_string(::steam_get_appid()));
#elif !IS_PRODUCTION_BUILD
			LOG("Removing steam_appid.txt");

			augs::remove_file("steam_appid.txt");
#endif
		}

		if (::steam_restart()) {
			return work_result::STEAM_RESTART;
		}
	}

	WEBSTATIC const auto steam_status = 
		is_cli_tool ?
		steam_init_result::DISABLED : 
		steam_init_result(::steam_init())
	;

	if (steam_status == steam_init_result::FAILURE) {
		LOG("Failed to init Steam API!");
		return work_result::FAILURE;
	}

	WEBSTATIC const bool steam_initialized = steam_status == steam_init_result::SUCCESS;

	WEBSTATIC auto deinit = augs::scope_guard([&]() {
		if (steam_initialized) {
			::steam_deinit();
		}
	});

	WEBSTATIC const bool is_steam_client = steam_initialized;
	WEBSTATIC const bool ranked_servers_enabled = is_steam_client;
	(void)ranked_servers_enabled;

	LOG_NVPS(is_steam_client);

	WEBSTATIC uint32_t steam_auth_request_id = 0;
	(void)steam_auth_request_id;

	WEBSTATIC const auto steam_id = is_steam_client ? std::to_string(::steam_get_id()) : std::string("0");
#endif

#if PLATFORM_WEB
	::USER_DIR = APPDATA_DIR / NONSTEAM_USER_FOLDER_NAME;
#else
	if (is_steam_client) {
		::USER_DIR = APPDATA_DIR / steam_id;
	}
	else {
		::USER_DIR = APPDATA_DIR / NONSTEAM_USER_FOLDER_NAME;
	}
#endif

	/*
		On Web, ::USER_DIR will be CWD/user,
		just like during normal development on Linux.
	*/

	/* 
		Just use the "user" folder when developing.
		CREATE_STEAM_APPID means we're still not really in "true" production.
		Otherwise the numbered user folder will show up in our git as we can't easily ignore it.
	*/

#if !IS_PRODUCTION_BUILD || CREATE_STEAM_APPID
	::USER_DIR = APPDATA_DIR / NONSTEAM_USER_FOLDER_NAME;
#endif

#if PLATFORM_UNIX && !PLATFORM_WEB
	WEBSTATIC auto signal_handler = [](const int signal_type) {
   		signal_status = signal_type;
	};

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);
	std::signal(SIGSTOP, signal_handler);
#endif

	setup_float_flags();

	{
		const auto all_created_directories = std::vector<augs::path_type> {
			CACHE_DIR,
			USER_DIR,
			DEMOS_DIR,
			CONFD_DIR,

			DOWNLOADED_ARENAS_DIR,
			EDITOR_PROJECTS_DIR
		};

		LOG("Creating directories:");

		for (const auto& a : all_created_directories) {
			LOG("%x", a);
			augs::create_directories(a);
		}
	}

	WEBSTATIC const auto canon_config_path = augs::path_type("default_config.json");
	WEBSTATIC const auto legacy_local_config_path = USER_DIR / "config.json";
	WEBSTATIC const auto runtime_prefs_path       = USER_DIR / "runtime_prefs.json";

	LOG("Loading %x.", canon_config_path);

	WEBSTATIC const auto canon_config_ptr = [&]() {
		auto result_ptr = std::make_unique<config_json_table>(canon_config_path);

		::make_canon_config(*result_ptr, params.type == app_type::DEDICATED_SERVER);

		return result_ptr;
	}();

	WEBSTATIC const auto canon_config_with_confd_ptr = std::make_unique<config_json_table>();

	WEBSTATIC auto& canon_config = *canon_config_ptr;
	WEBSTATIC auto& canon_config_with_confd = *canon_config_with_confd_ptr;

	WEBSTATIC auto config_ptr = [&]() {
		auto result = std::make_unique<config_json_table>(canon_config);

		if (augs::exists(legacy_local_config_path)) {
			if (!augs::exists(runtime_prefs_path)) {
				LOG("RENAMING LEGACY CONFIG:\n%x\n->\n%x", legacy_local_config_path, runtime_prefs_path);

				std::filesystem::rename(
					legacy_local_config_path,
					runtime_prefs_path
				);
			}
		}

		augs::for_each_in_directory_sorted(
			CONFD_DIR,
			[&](auto...) { return callback_result::CONTINUE; },
			[&](const augs::path_type& config_file) {
				if (config_file.extension() == ".json") {
					LOG("Applying config: %x", config_file);
					result->load_patch(config_file);
				}
				else {
					LOG("Skipping non-json file: %x", config_file);
				}

				return callback_result::CONTINUE;
			}
		);

		canon_config_with_confd = *result;

		if (augs::exists(runtime_prefs_path)) {
			LOG("Applying config: %x", runtime_prefs_path);
			result->load_patch(runtime_prefs_path);
		}

		if (!params.apply_config.empty()) {
			const auto cli_config_path = CALLING_CWD / params.apply_config;
			LOG("Applying config: %x", cli_config_path);
			result->load_patch(cli_config_path);
		}

		if (params.daily_autoupdate) {
			result->server.daily_autoupdate = true;
		}

		if (params.autoupdate_delay.has_value()) {
			result->server.autoupdate_delay = *params.autoupdate_delay;
		}

#if !PLATFORM_WEB
		if (result->client.nickname.empty()) {
			result->client.nickname = augs::get_user_name();
		}
#endif

		if (is_steam_client) {
			if (result->client.use_account_nickname) {
				if (const auto steam_username = ::steam_get_username()) {
					result->client.nickname = std::string(steam_username);
				}
			}

			if (result->client.use_account_avatar) {
				if (const auto avatar = ::steam_get_avatar_image(); avatar.get_size().is_nonzero()) {
					avatar.save_as_png(CACHED_AVATAR);

					result->client.avatar_image_path = CACHED_AVATAR;
				}
			}
		}

		return result;
	}();

	LOG("Loaded all user configs.");

	if (!params.assign_teams.empty()) {
		LOG("Reading server assigned teams from: %x", CALLING_CWD / params.assign_teams); 
	}

#if BUILD_NETWORKING
	WEBSTATIC const auto assigned_teams = 
		params.assign_teams.empty() ? 
		server_assigned_teams() :
		server_assigned_teams(CALLING_CWD / params.assign_teams)
	;

	if (params.type == app_type::DEDICATED_SERVER && params.assign_teams.empty()) {
		LOG("No players were assigned to teams for this dedicated server session.");
	}
	else {
		LOG("Assigned teams: %x", assigned_teams.id_to_faction);
	}
#endif

	WEBSTATIC auto& config = *config_ptr;

	{
		augs::unique_lock<augs::mutex> lock(log_mutex);
		::log_timestamp_format = config.log_timestamp_format;
	}

	WEBSTATIC const auto fp_test_settings = [&]() {
		auto result = config.float_consistency_test;

		if (params.test_fp_consistency != -1) {
			result.passes = params.test_fp_consistency;
			LOG("Forcing %x fp consistency passes.", params.test_fp_consistency);
		}

		if (!params.consistency_report.empty()) {
			result.report_filename = params.consistency_report;
		}

		return result;
	}();	

	WEBSTATIC const auto float_tests_succeeded = 
		perform_float_consistency_tests(fp_test_settings)
	;

	if (!float_tests_succeeded) {
		LOG("WARNING! FLOAT CONSISTENCY TESTS HAVE FAILED!");
	}

#if BUILD_NETWORKING
	LOG("Initializing network RAII.");

	const bool report_rtc_errors = 
		params.type == app_type::MASTERSERVER
		&& config.masterserver.report_rtc_errors_to_webhook
	;

	const bool verbose_rtc_log = 
		params.type == app_type::MASTERSERVER
	;

	WEBSTATIC auto network_raii = augs::network_raii(
		report_rtc_errors,
		verbose_rtc_log
	);

#if PLATFORM_WEB
	const auto random_seed = EM_ASM_INT_V({
		return Module.getRandomValue();
	});

	LOG("Random seed for this run: %x", random_seed);

	netcode_rng = randomization(random_seed);
#endif
#endif

	if (config.unit_tests.run || params.unit_tests_only) {
		/* Needed by some unit tests */

		LOG("Running unit tests.");
		augs::run_unit_tests(config.unit_tests);

		LOG("All unit tests have passed.");

		if (params.unit_tests_only) {
			return work_result::SUCCESS;
		}
	}
	else {
		LOG("Unit tests were disabled.");
	}

	LOG("Initializing ImGui.");

#if !HEADLESS
	WEBSTATIC const auto imgui_ini_path = (USER_DIR / (get_preffix_for(current_app_type) + "imgui.ini")).string();
	WEBSTATIC const auto imgui_log_path = get_path_in_log_files("imgui_log.txt");

	WEBSTATIC const auto imgui_raii = augs::imgui::context_raii(
		imgui_ini_path.c_str(),
		imgui_log_path.c_str(),
		config.gui_style
	);
#endif

	WEBSTATIC auto last_update_result = self_update_result();

#if PLATFORM_WEB
	LOG("Nothing to self-update on the Web.");
#else
	WEBSTATIC const bool should_run_self_updater = 
		params.update_once_now ||
		params.only_check_update_availability_and_quit ||
		(config.self_update.update_on_launch && !params.no_update_on_launch)
	;

	if (should_run_self_updater) {
		using update_result = self_update_result_type;

		const bool should_update_headless = is_cli_tool;

		LOG_NVPS(should_update_headless);

		const bool only_check = is_steam_client || params.only_check_update_availability_and_quit;

		if (should_update_headless) {
			last_update_result = check_and_apply_updates(
				params.appimage_path,
				only_check,
				config.self_update
			);
		}
		else {
			last_update_result = check_and_apply_updates(
				params.appimage_path,
				only_check,
				config.self_update,
				&config.gui_fonts.gui,
				config.window
			);
		}

		LOG_NVPS(last_update_result.type);

		if (is_steam_client) {
			LOG("No need to run updater as this is a Steam client.");
		}
		else {
			if (params.only_check_update_availability_and_quit) {
				if (last_update_result.type == update_result::UPDATE_AVAILABLE) {
					return work_result::REPORT_UPDATE_AVAILABLE;
				}
				else {
					return work_result::REPORT_UPDATE_UNAVAILABLE;
				}
			}

			if (last_update_result.type == update_result::UPGRADED) {
				LOG("work: Upgraded successfully. Requesting relaunch.");
				return work_result::RELAUNCH_UPGRADED;
			}

			if (last_update_result.type == update_result::EXIT_APPLICATION) {
				return work_result::SUCCESS;
			}

			if (last_update_result.exit_with_failure_if_not_upgraded) {
				return work_result::FAILURE;
			}

			if (last_update_result.type == update_result::UP_TO_DATE) {
				if (params.upgraded_successfully) {
					last_update_result.type = update_result::FIRST_LAUNCH_AFTER_UPGRADE;
				}
			}
		}
	}
	else {
		if (params.no_update_on_launch) {
			LOG("Skipping update check due to --no-update-on-launch flag.");
		}
		else {
			LOG("Skipping update check due to update_on_launch = false.");
		}
	}
#endif

	WEBSTATIC augs::timer until_first_swap;
	WEBSTATIC bool until_first_swap_measured = false;
	(void)until_first_swap;
	(void)until_first_swap_measured;

	WEBSTATIC session_profiler render_thread_performance;
	WEBSTATIC network_profiler network_performance;
	WEBSTATIC network_info network_stats;
	(void)network_stats;
	WEBSTATIC server_network_info server_stats;
	(void)server_stats;

	dump_detailed_sizeof_information(get_path_in_log_files("detailed_sizeofs.txt"));

	WEBSTATIC auto last_saved_config_ptr = std::make_unique<config_json_table>(config);
	WEBSTATIC auto& last_saved_config = *last_saved_config_ptr;

	WEBSTATIC auto change_with_save = [&](auto setter) {
		setter(config);
		setter(last_saved_config);

		last_saved_config.save_patch(canon_config_with_confd, runtime_prefs_path, true);
	};
	(void)change_with_save;

#if PLATFORM_WEB
	LOG_NVPS(config.client.nickname);

	WEBSTATIC auto gen_random_nickname = [&]() {
		const auto rng_name = ::make_random_nickname(netcode_rng);
		const auto final_nickname = 
			!params.guest.empty() ?
			typesafe_sprintf("[%x] %x", params.guest, rng_name) :
			rng_name
		;

		LOG("Generated guest nickname: %x", final_nickname);

		return final_nickname;
	};

	if (config.client.nickname.empty() || config.client.nickname == "web_user" || config.client.nickname == "Guest") {
		const auto final_nickname = gen_random_nickname();

		change_with_save(
			[&](auto& cfg) {
				cfg.client.nickname = final_nickname;
			}
		);
	}
#endif

#if HEADLESS
#else
	WEBSTATIC auto last_exit_incorrect_popup = std::optional<simple_popup>();
#endif

	WEBSTATIC auto freetype_library = std::optional<augs::freetype_raii>();

	if (params.type == app_type::GAME_CLIENT) {
		LOG("Initializing freetype");

		freetype_library.emplace();
	}

#if BUILD_MASTERSERVER
	if (params.type == app_type::MASTERSERVER) {
		auto adjusted_config = config;
		auto& masterserver = adjusted_config.masterserver;

		if (params.first_udp_command_port.has_value()) {
			masterserver.first_udp_command_port = *params.first_udp_command_port;
		}

		if (params.server_list_port.has_value()) {
			masterserver.server_list_port = *params.server_list_port;
		}

		LOG(
			"Starting the masterserver at ports: %x (Server list), %x-%x (UDP commands ports)",
			masterserver.server_list_port,
			masterserver.first_udp_command_port,
			masterserver.get_last_udp_command_port()
		);

		return perform_masterserver(adjusted_config);
	}
#endif

#if !HEADLESS
#if BUILD_NATIVE_SOCKETS
	WEBSTATIC auto last_requested_local_port = port_type(0);

	WEBSTATIC auto chosen_server_port = [&](){
		if (params.server_port.has_value()) {
			return *params.server_port;
		}

		return config.server_start.port;
	};

	WEBSTATIC auto auxiliary_socket = std::optional<netcode_socket_raii>();

	auto delete_auxiliary_socket = [&]() {
		auxiliary_socket.reset();
	};

	WEBSTATIC auto get_bound_local_port = [&]() {
		return auxiliary_socket ? auxiliary_socket->socket.address.port : 0;
	};

	WEBSTATIC auto recreate_auxiliary_socket = [&](std::optional<port_type> temporary_port = std::nullopt) {
		if (params.is_cli_server()) {
			return;
		}

		const auto preferred_port = temporary_port.has_value() ? *temporary_port : chosen_server_port();
		last_requested_local_port = preferred_port;

		try {
			auxiliary_socket.emplace(preferred_port);

			LOG("Successfully bound the nat detection socket to the preferred server port: %x.", preferred_port);
		}
		catch (const netcode_socket_raii_error&) {
			LOG("WARNING! Could not bind the nat detection socket to the preferred server port: %x.", preferred_port);

			delete_auxiliary_socket();
			auxiliary_socket.emplace();
		}

		LOG_NVPS(get_bound_local_port());
		ensure(get_bound_local_port() != 0);
	};

	recreate_auxiliary_socket();

	WEBSTATIC auto stun_provider = stun_server_provider(config.nat_detection.stun_server_list);

	WEBSTATIC auto nat_detection = std::optional<nat_detection_session>();

	WEBSTATIC auto nat_detection_complete = [&]() {
		if (nat_detection == std::nullopt) {
			return false;
		}

		return nat_detection->query_result().has_value();
	};

	WEBSTATIC auto restart_nat_detection = [&]() {
		if (params.is_cli_server()) {
			return;
		}

		nat_detection.reset();
		nat_detection.emplace(config.nat_detection, stun_provider);
	};

	WEBSTATIC auto get_detected_nat = [&]() {
		if (nat_detection == std::nullopt) {
			return nat_detection_result();
		}

		if (const auto detected_nat = nat_detection->query_result()) {
			return *detected_nat;
		}

		return nat_detection_result();
	};

	restart_nat_detection();
#else
	WEBSTATIC auto get_detected_nat = [&]() {
		return nat_detection_result();
	};

	WEBSTATIC auto chosen_server_port = [&](){
		return 0;
	};

	auto delete_auxiliary_socket = [&]() {

	};

	WEBSTATIC auto get_bound_local_port = [&]() {
		return 0;
	};
#endif
#endif

	WEBSTATIC const auto official = std::make_unique<packaged_official_content>();

#if !PLATFORM_WEB

	WEBSTATIC auto handle_sigint = [&]() {
#if PLATFORM_UNIX
		if (signal_status != 0) {
			const auto sig = signal_status.load();

			LOG("%x received.", strsignal(sig));

			if(
				sig == SIGINT
				|| sig == SIGSTOP
				|| sig == SIGTERM
			) {
				LOG("Gracefully shutting down.");
				return true;
			}
		}
#endif

		return false;

	};

	{
		bool sync = false;
		bool quit_after_sync = false;

		if (params.type == app_type::DEDICATED_SERVER && config.server.sync_all_external_arenas_on_startup) {
			LOG("sync_all_external_arenas_on_startup specified.");
			sync = true;
		}
		else if (params.sync_external_arenas) {
			LOG("--sync-external-arenas specified.");
			sync = true;
		}
		else if (params.sync_external_arenas_and_quit) {
			LOG("--sync-external-arenas-and-quit specified.");
			sync = true;
			quit_after_sync = true;
		}

		if (sync) {
			const auto& provider = config.server.external_arena_files_provider;

			if (parsed_url(provider).valid()) {
				LOG("External arena provider: %x", provider);

				headless_map_catalogue headless;

				augs::timer last_report;

				auto report_progress = [&]() {
					if (const auto& dl = headless.get_downloading()) {
						if (const auto current = dl->get_current_map_name()) {
							const auto i = dl->get_current_map_index();
							const auto total = dl->get_total_maps();

							LOG(
								"Syncing %x (%x/%x): %2f% complete. (Total: %2f%)", 
								*current, i, total,
								100 * dl->get_current_map_progress(),
								100 * dl->get_total_progress()
							);
						}
					}
				};

				while (true) {
					if (headless.advance({ provider }) == headless_catalogue_result::LIST_REFRESH_COMPLETE) {
						const auto last_error = headless.get_list_catalogue_error();

						if (last_error.size() > 0) {
							LOG("Failed to download the catalogue. Reason:\n%x", last_error);
							break;
						}
						else {
							const auto all = headless.launch_download_all({ provider });

							if (all.empty()) {
								LOG("All arenas are up to date. There's nothing to sync.");
								break;
							}

							std::string all_report;

							for (const auto& a : all) {
								all_report += a.name + ", ";
							}

							if (all_report.size() >= 2) {
								all_report.pop_back();
								all_report.pop_back();
							}

							LOG("Syncing %x arena(s): %x", all.size(), all_report);
						}
					}

					if (last_report.get<std::chrono::seconds>() > 0.5) {
						last_report.reset();
						report_progress();
					}

					if (headless.finalize_download()) {
						LOG("Finished syncing all arenas.");
						break;
					}

					if (handle_sigint()) {
						return work_result::SUCCESS;
					}

					augs::sleep(1.0 / 1000);
				}
			}
			else {
				LOG("Couldn't parse arena provider URL: \"%x\". Aborting sync.", provider);
			}

			if (quit_after_sync) {
				LOG("Quitting after map sync finished.");
				return work_result::SUCCESS;
			}
			else {
				LOG("Map sync finished.");
			}
		}
	}

	if (params.type == app_type::DEDICATED_SERVER) {
#if BUILD_NETWORKING
		LOG("Starting the dedicated server.");

		enum instance_type {
			RANKED,
			CASUAL,
			SINGLE
		};

		const auto config_pattern = config;
		const auto original_port = config_pattern.server_start.port;

		auto port_counter = original_port;
		auto webrtc_port_counter = config_pattern.server.webrtc_port_range_begin;

		auto write_vars_to_disk = [&](const server_vars& new_vars) {
			LOG("Writing server_vars to disk.");

			change_with_save([&](auto& cfg) {
				cfg.server = new_vars;
			});
		};

		auto make_next_worker_input = [&](const uint16_t index, const instance_type type) {
			const bool first_server = port_counter == original_port;
			const bool later_server = !first_server;

			const auto instance_log_label = typesafe_sprintf("#%x%x", index, type == RANKED ? "R" : "c");
			const auto name_suffix = typesafe_sprintf("#%x%x", index, type == RANKED ? " R" : "");

			const auto server_name_suffix = type == SINGLE ? std::string("") : std::string(" ") + name_suffix;

			const auto label = [type]() {
				if (type == RANKED) {
					return "ranked";
				}

				if (type == CASUAL) {
					return "casual";
				}

				return "single";
			}();

			const auto instance_label = std::string(label) + server_name_suffix;

			auto this_config = config_pattern;

			this_config.server.ranked.autostart_when = 
				type == RANKED ?
				ranked_autostart_type::ALWAYS :
				ranked_autostart_type::NEVER
			;

			LOG_NVPS(this_config.server.server_name, server_name_suffix);

			this_config.server_start.port = port_counter++;

			if (type != SINGLE) {
				/* Force muxing on consecutive ports */
				this_config.server.webrtc_udp_mux = true;
				this_config.server.webrtc_port_range_begin = webrtc_port_counter++;
			}

			LOG(
				"Starting %x server instance. Binding to a port: %x",
				instance_label,
				this_config.server_start.port
			);

			auto write_or_not = std::function<void(const server_vars&)>(write_vars_to_disk);
			bool should_suppress_webhook = params.suppress_server_webhook;
			
			const bool pick_random_map = 
				later_server && 
				this_config.server.cycle_randomize_order &&
				this_config.server.cycle != arena_cycle_type::REPEAT_CURRENT
			;

			if (later_server) {
				/*
					To prevent concurrent downloads,
					only one instance should check for autoupdates.
				*/
				this_config.server.daily_autoupdate = false;

				should_suppress_webhook = true;
				write_or_not = nullptr;
			}

			if (pick_random_map) {
				this_config.server.arena = "";
			}

			auto server_ptr = std::make_unique<server_setup>(
				*official,
				this_config.server_start,
				this_config.server,
				canon_config_with_confd.server,
				this_config.server_private,
				this_config.client,
				this_config.dedicated_server,

				should_suppress_webhook,
				type == SINGLE ? assigned_teams : server_assigned_teams(),

				this_config.webrtc_signalling_server_url,
				server_name_suffix
			);

			if (pick_random_map) {
				server_ptr->choose_next_map_from_cycle();
			}

			return dedicated_server_worker_input { 
				std::move(server_ptr),
				params.appimage_path,
				this_config.self_update,
				write_or_not,
				instance_label,
				"[" + instance_log_label + "] "
			};
		};

		const auto num_ranked = std::min(uint16_t(20u), config_pattern.num_ranked_servers);
		const auto num_casual = std::min(uint16_t(20u), config_pattern.num_casual_servers);

		const auto num_total = num_ranked + num_casual;

		if (num_total == 0) {
			LOG("This is a single-instance dedicated server.");

			const auto worker_in = make_next_worker_input(1, SINGLE);
			const auto result = dedicated_server_worker(worker_in, handle_sigint);
			write_vars_to_disk(worker_in.server_ptr->get_current_vars());

			LOG("Quitting the single-dedicated server with: %x", ::describe_work_result(result));

			return result;
		}
		else {
			LOG("This is a multi-instance dedicated server.");

			LOG("Casual instances: %x", num_casual);
			LOG("Ranked instances: %x", num_ranked);
			LOG("Total  instances: %x", num_total);

			std::mutex result_lk;
			std::atomic<bool> one_shutdown_already = false;
			work_result result = work_result::SUCCESS;

			auto should_interrupt = [&one_shutdown_already, handle_sigint]() {
				if (one_shutdown_already.load()) {
					return true;
				}

				return handle_sigint();
			};

			auto on_instance_exit = [&](const work_result this_result) {
				/* All need to exit. */
				one_shutdown_already.store(true);

				if (this_result != work_result::SUCCESS) {
					std::scoped_lock lk(result_lk);

					/*
						In extreme edge cases multiple instances could set different non-SUCCESS results
						(e.g. autonomous RELAUNCH_AND_UPDATE_DEDICATED_SERVER
						and manually-induced RELAUNCH_DEDICATED_SERVER)

						Thus, we need to arbitrarily choose just one result.
						It will not really break anything if we do.
					*/

					result = this_result;
				}
			};

			auto make_worker = [&](const uint16_t i, const instance_type type) {
				return make_server_worker(make_next_worker_input(i + 1, type), should_interrupt, on_instance_exit);
			};

			std::vector<std::thread> instances;
			instances.reserve(num_total);

			for (uint16_t i = 0; i < num_casual; ++i) {
				instances.emplace_back(make_worker(i, CASUAL));
			}

			for (uint16_t i = 0; i < num_ranked; ++i) {
				instances.emplace_back(make_worker(i, RANKED));
			}

			for (auto& t : instances) {
				t.join();
			}

			LOG("Quitting the multi-dedicated server with: %x", ::describe_work_result(result));

			return result;
		}
#endif
	}

#endif // #if !PLATFORM_WEB

#if HEADLESS
	LOG("Headless build. Nothing to do.");
	return work_result::SUCCESS;
#else
	WEBSTATIC auto abandon_pending_op = std::optional<ingame_menu_button_type>();
	WEBSTATIC auto abandon_are_you_sure_popup = std::optional<simple_popup>();

	WEBSTATIC auto make_abandon_popup = [&](auto op) {
		abandon_pending_op = op;

		simple_popup sp;
		sp.title = "WARNING";
		sp.warning_notice_above = "Are you sure you want to abandon the match?";
		sp.message = "You have 3 minutes to rejoin after quitting.\n";

		sp.warning_notice = "\nIf you do not come back,\nyou will LOSE MMR - as if you lost the match THREE TIMES!\n \n";

		abandon_are_you_sure_popup = sp;
	};

	WEBSTATIC auto perform_abandon_are_you_sure_popup = [&]() {
		if (abandon_are_you_sure_popup.has_value()) {
			const auto result = abandon_are_you_sure_popup->perform({
				{ "Abandon", rgba(200, 80, 0, 255), rgba(25, 20, 0, 255) },
				{ "Cancel", rgba::zero, rgba::zero }
			});

			if (result) {
				abandon_are_you_sure_popup = std::nullopt;

				if (const bool cancelled = result == 2) {
					abandon_pending_op = std::nullopt;
				}
			}
		}

		return 0;
	};

	WEBSTATIC auto failed_to_load_arena_popup = std::optional<simple_popup>();

	WEBSTATIC auto perform_failed_to_load_arena_popup = [&]() {
		if (failed_to_load_arena_popup.has_value()) {
			if (failed_to_load_arena_popup->perform()) {
				failed_to_load_arena_popup = std::nullopt;
			}
		}
	};

	WEBSTATIC auto perform_last_exit_incorrect = [&]() {
		if (last_exit_incorrect_popup.has_value()) {
			if (last_exit_incorrect_popup->perform()) {
				last_exit_incorrect_popup = std::nullopt;
			}
		}
	};

	LOG("Initializing the audio context.");

	WEBSTATIC augs::audio_context audio(config.audio);

	LOG("Logging all audio devices.");
	augs::log_all_audio_devices(get_path_in_log_files("audio_devices.txt"));

	WEBSTATIC const auto num_pool_workers = config.performance.get_num_pool_workers();
	LOG("Creating the thread pool with %x workers.", num_pool_workers);
	WEBSTATIC auto thread_pool = augs::thread_pool(num_pool_workers);

	LOG("Initializing audio command buffers.");
	WEBSTATIC augs::audio_command_buffers audio_buffers(thread_pool);

	LOG("Initializing the window.");
	WEBSTATIC augs::window window(config.window);

	LOG("Initializing the renderer backend.");
	WEBSTATIC augs::graphics::renderer_backend renderer_backend;

	WEBSTATIC game_frame_buffer_swapper buffer_swapper;

	WEBSTATIC auto get_read_buffer = [&]() -> game_frame_buffer& {
		return buffer_swapper.get_read_buffer();
	};

	WEBSTATIC auto get_write_buffer = [&]() -> game_frame_buffer& {
		return buffer_swapper.get_write_buffer();
	};

	get_write_buffer().screen_size = window.get_screen_size();
	get_read_buffer().new_settings = config.window;
	get_read_buffer().swap_when = config.performance.swap_window_buffers_when;

	WEBSTATIC auto logic_get_screen_size = [&]() {
		return get_write_buffer().screen_size;
	};

	WEBSTATIC augs::timer when_last_gui_fonts_ratio_changed;

	WEBSTATIC auto get_gui_fonts_ratio = [&]() {
		auto scale = std::clamp(config.ui_scale, 1.0f, 4.0f);

#if WEB_LOWEND
		/*
			Up to 1080, increase only a little bit,
			because we've already adjusted everything in the game to look good with this res.

			Then increase proportionally.
		*/

		const auto real_size = logic_get_screen_size();

		if (real_size.y <= 1080) {
			const auto orig_size = vec2(922, 487);
			const auto ratio = real_size.y / orig_size.y;

			return scale * std::min(1.333333333f, ratio);
		}
		else {
			const auto orig_size = vec2(1920, 1080);
			const auto ratio = real_size.y / orig_size.y;

			return scale * 1.333333333f * ratio;
		}
#else
		return scale;
#endif
	};

	WEBSTATIC float last_gui_fonts_ratio = get_gui_fonts_ratio();

	WEBSTATIC auto get_general_renderer = [&]() -> augs::renderer& {
		return get_write_buffer().renderers.all[renderer_type::GENERAL];
	};

	LOG_NVPS(renderer_backend.get_max_texture_size());

	LOG("Initializing the necessary fbos.");
	WEBSTATIC all_necessary_fbos necessary_fbos(
		logic_get_screen_size(),
		config.drawing
	);

	LOG("Initializing the necessary shaders.");
	WEBSTATIC all_necessary_shaders necessary_shaders(
		get_general_renderer(),
		CANON_SHADER_FOLDER,
		LOCAL_SHADER_FOLDER,
		config.drawing
	);

	LOG("Initializing the necessary sounds.");
	WEBSTATIC all_necessary_sounds necessary_sounds(
		"content/sfx/necessary"
	);

	LOG("Initializing the necessary image definitions.");
	WEBSTATIC const necessary_image_definitions_map necessary_image_definitions(
		"content/gfx/necessary",
		config.content_regeneration.regenerate_every_time
	);

	WEBSTATIC auto loaded_gui_fonts_ratio = get_gui_fonts_ratio();

	LOG("Creating the ImGui atlas.");
	WEBSTATIC auto imgui_atlas = augs::imgui::create_atlas(config.gui_fonts.gui, loaded_gui_fonts_ratio);

	WEBSTATIC const auto configurables = configuration_subscribers {
		window,
		necessary_fbos,
		audio,
		get_general_renderer()
	};

	WEBSTATIC auto remember_window_settings = augs::scope_guard([&]() {
		/*
			Remember window size, position and fullscreen status.
			People don't expect to have to save it.
		*/

		configurables.sync_back_into(config);

		bool update = false;

		if (config.window.position != last_saved_config.window.position) {
			last_saved_config.window.position = config.window.position;
			update = true;
		}

		if (config.window.size != last_saved_config.window.size) {
			last_saved_config.window.size = config.window.size;
			update = true;
		}

		if (config.window.fullscreen != last_saved_config.window.fullscreen) {
			last_saved_config.window.fullscreen = config.window.fullscreen;
			update = true;
		}

		if (update) {
			last_saved_config.save_patch(canon_config_with_confd, runtime_prefs_path, true);
		}
	});

	WEBSTATIC atlas_profiler atlas_performance;
	WEBSTATIC frame_profiler game_thread_performance;

	/* 
		unique_ptr is used to avoid stack overflow.

		Main menu setup state may be preserved, 
		therefore it resides in a separate unique_ptr.
	*/

	WEBSTATIC std::unique_ptr<main_menu_setup> main_menu;
	WEBSTATIC main_menu_gui main_menu_gui;
	WEBSTATIC ltrb menu_ltrb;

	WEBSTATIC auto has_main_menu = [&]() {
		return main_menu != nullptr;
	};

	WEBSTATIC settings_gui_state settings_gui = std::string("Settings");
	settings_gui.display_size_for_clipping = window.get_display().get_size();

	WEBSTATIC start_client_gui_state start_client_gui = std::string("Connect to server");
	WEBSTATIC start_server_gui_state start_server_gui = std::string("Host a server");

	start_client_gui.is_steam_client = is_steam_client;
	start_server_gui.is_steam_client = is_steam_client;

#if BUILD_NETWORKING
	WEBSTATIC bool was_browser_open_in_main_menu = false;
	WEBSTATIC browse_servers_gui_state browse_servers_gui = std::string("Browse servers");

	WEBSTATIC auto find_chosen_server_info = [&]() {
		return browse_servers_gui.find_entry_by_connect_string(config.client_connect);
	};

	(void)find_chosen_server_info;
#endif

	WEBSTATIC map_catalogue_gui_state map_catalogue_gui = std::string("Download maps");

	WEBSTATIC leaderboards_gui_state leaderboards_gui = std::string("Leaderboards");

	WEBSTATIC std::string displayed_connecting_server_name;

	WEBSTATIC auto emplace_main_menu = [&] (auto&&... args) {
		if (main_menu == nullptr) {
			main_menu = std::make_unique<main_menu_setup>(std::forward<decltype(args)>(args)...);
			map_catalogue_gui.rebuild_miniatures();
		}
	};

	WEBSTATIC ingame_menu_gui ingame_menu;

	/*
		Runtime representations of viewables,
		loaded from the definitions provided by the current setup.
		The setup's chosen viewables_loading_type decides if they are 
		loaded just once or if they are for example continuously streamed.
	*/

	LOG("Initializing the streaming of viewables.");

	WEBSTATIC auto streaming_ptr = std::make_unique<viewables_streaming>();
	WEBSTATIC viewables_streaming& streaming = *streaming_ptr;

	WEBSTATIC social_sign_in_state social_sign_in = std::string("Login");

#if PLATFORM_WEB
	WEBSTATIC auto is_signed_in = [&]() {
		if (social_sign_in.cached_auth.can_self_refresh()) {
			/*
				Will always self-refresh if there is such a need.
			*/

			return true;
		}

		return social_sign_in.cached_auth.is_signed_in();
	};

	WEBSTATIC auto is_auth_expired = [&]() {
		return social_sign_in.cached_auth.expired();
	};

	WEBSTATIC auto auth_self_refresh = [&]() {
		return social_sign_in.cached_auth.self_refresh();
	};

	WEBSTATIC auto auth_can_self_refresh = [&]() {
		return social_sign_in.cached_auth.can_self_refresh();
	};

	WEBSTATIC auto social_log_out = [&]() {
		social_sign_in.cached_auth.log_out();

		streaming.requested_avatar_preview = BLANK_AVATAR;

		augs::remove_file(CACHED_AUTH_PATH);
		augs::remove_file(CACHED_AVATAR);

		config.client.signed_in.nickname = "";

		change_with_save(
			[&](auto& cfg) {
				cfg.client.avatar_image_path = augs::path_type();
			}
		);
	};

	WEBSTATIC auto perform_social_sign_in_popup = [&](const bool prompted_once) {
		const bool confirmed = social_sign_in.perform({
			streaming.necessary_images_in_atlas,
			prompted_once,
			params.is_crazygames
		});

		if (confirmed) {
			const bool play_as_guest = !prompted_once;
			social_sign_in.close();

			if (play_as_guest) {
				change_with_save(
					[&](auto& cfg) {
						cfg.client.nickname = social_sign_in.guest_nickname;
						cfg.prompted_for_sign_in_once = true;
					}
				);
			}
		}
	};

	if (params.is_crazygames) {
		LOG("call_try_fetch_initial_user");

		if (call_try_fetch_initial_user()) {
			LOG("Fetched initial user");
			config.prompted_for_sign_in_once = true;
		}
		else {
			LOG("No initial user");
			social_log_out();
		}
	}
	else {
		try {
			social_sign_in.cached_auth = augs::from_json_file<web_auth_data>(CACHED_AUTH_PATH);
			LOG("Loaded some cached auth from %x", CACHED_AUTH_PATH);
		}
		catch (...) {
			social_sign_in.cached_auth = {};
			LOG("No cached auth found in %x", CACHED_AUTH_PATH);
		}

		if (social_sign_in.cached_auth.is_set()) {
			if (is_auth_expired()) {
				LOG("Auth expired. Logging out.");
				social_log_out();
			}
			else {
				LOG("Starting token_still_valid_check");
				token_still_valid_check = launch_async([ch = social_sign_in.cached_auth]() { return ch.check_token_still_valid(); });
			}
		}
	}

	social_sign_in.guest_nickname = config.client.nickname;
#endif

	try {
		augs::image avatar;
		avatar.from_file(config.client.avatar_image_path);
		streaming.avatar_preview_tex = augs::graphics::texture(avatar); 
	}
	catch (...) {
		augs::image avatar;
		avatar.from_file(BLANK_AVATAR);
		streaming.avatar_preview_tex = augs::graphics::texture(avatar); 
	}

	WEBSTATIC auto get_my_avatar_bytes = [&]() {
		std::vector<std::byte> avatar;

		try {
			const auto& path = config.client.avatar_image_path;

			if (!path.empty()) {
				return augs::file_to_bytes(path);
			}
		}
		catch (...) {

		}

		return std::vector<std::byte>();
	};

	WEBSTATIC auto get_blank_texture = [&]() {
		return streaming.necessary_images_in_atlas[assets::necessary_image_id::BLANK];
	};

	WEBSTATIC auto get_drawer_for = [&](augs::renderer& chosen_renderer) { 
		return augs::drawer_with_default {
			chosen_renderer.get_triangle_buffer(),
			get_blank_texture()
		};
	};

	WEBSTATIC world_camera gameplay_camera;
	LOG("Initializing the audiovisual state.");
	WEBSTATIC auto audiovisuals = std::make_unique<audiovisual_state>();

	WEBSTATIC auto get_audiovisuals = [&]() -> audiovisual_state& {
		return *audiovisuals;
	};


	/*
		The lambdas that aid to make the main loop code more concise.
	*/	

	WEBSTATIC std::unique_ptr<setup_variant> current_setup = nullptr;
	WEBSTATIC std::unique_ptr<setup_variant> background_setup = nullptr;

	WEBSTATIC auto has_current_setup = [&]() {
		return current_setup != nullptr;
	};

	WEBSTATIC auto would_abandon_ranked_match = [&]() {
#if BUILD_NETWORKING
		if (has_current_setup()) {
			if (const auto setup = std::get_if<client_setup>(std::addressof(*current_setup))) {
				return setup->would_abandon_match();
			}
		}
#endif

		return false;
	};

	WEBSTATIC auto is_during_tutorial = [&]() {
		if (has_current_setup()) {
			if (const auto setup = std::get_if<test_scene_setup>(std::addressof(*current_setup))) {
				return setup->is_tutorial();
			}
		}

		return false;
	};

	WEBSTATIC auto is_shooting_range = [&]() {
		if (has_current_setup()) {
			if (const auto setup = std::get_if<test_scene_setup>(std::addressof(*current_setup))) {
				return !setup->is_tutorial();
			}
		}

		return false;
	};

	WEBSTATIC auto still_querying_server_info = [&]() {
		if (has_current_setup()) {
			if (const auto setup = std::get_if<server_setup>(std::addressof(*current_setup))) {
				return setup->get_connect_string() == "";
			}
		}

		return false;
	};

	WEBSTATIC auto should_hide_invite_to_join = [&]() {
		if (has_current_setup()) {
			if (params.is_crazygames) {
				return 
					std::holds_alternative<client_setup>(*current_setup) ||
					std::holds_alternative<server_setup>(*current_setup)
				;
			}

			return std::holds_alternative<editor_setup>(*current_setup);
		}
		
		return false;
	};

	WEBSTATIC auto restore_background_setup = [&]() {
		current_setup = std::move(background_setup);
		background_setup = std::unique_ptr<setup_variant>();

		ingame_menu.show = false;
	};

	WEBSTATIC auto visit_current_setup = [&](auto callback) -> decltype(auto) {
		if (has_current_setup()) {
			return std::visit(
				[&](auto& setup) -> decltype(auto) {
					return callback(setup);
				}, 
				*current_setup
			);
		}
		else {
			ensure(main_menu != nullptr);
			return callback(*main_menu);
		}
	};

	WEBSTATIC auto setup_requires_cursor = [&]() {
		if (social_sign_in.is_open()) {
			return true;
		}

		return visit_current_setup([&](const auto& s) {
			return s.requires_cursor();
		});
	};

	WEBSTATIC auto get_interpolation_ratio = [&]() {
		return visit_current_setup([&](auto& setup) {
			return setup.get_interpolation_ratio();
		});
	};

	WEBSTATIC auto continuous_sounds_clock = [&]() {
		return visit_current_setup([&]<typename S>(S& setup) {
			if constexpr(S::has_game_mode) {
				return setup.on_mode_with_input(
					[&](const auto& typed_mode, const auto& mode_input) {
						return typed_mode.continuous_sounds_clock(mode_input);
					}
				);
			}
			else {
				return uint32_t(0);
			}
		});
	};

	WEBSTATIC auto on_specific_setup = [&](auto callback) -> decltype(auto) {
		using T = remove_cref<argument_t<decltype(callback), 0>>;

		if constexpr(std::is_same_v<T, main_menu_setup>) {
			if (has_main_menu()) {
				callback(*main_menu);
			}
		}
		else {
			if (has_current_setup()) {
				/* Test */
				static_assert(is_one_of_list_v<project_selector_setup, setup_variant>);

				if constexpr(is_one_of_list_v<T, setup_variant>) {
					if (auto* setup = std::get_if<T>(&*current_setup)) {
						callback(*setup);
					}
				}
			}
		}
	};

	WEBSTATIC auto get_unofficial_content_dir = [&]() {
		return visit_current_setup([&](const auto& s) { return s.get_unofficial_content_dir(); });
	};

	WEBSTATIC auto get_render_layer_filter = [&]() {
		return visit_current_setup([&](const auto& s) { return s.get_render_layer_filter(); });
	};

	/* TODO: We need to have one game gui per cosmos. */
	WEBSTATIC game_gui_system game_gui;
	WEBSTATIC bool game_gui_mode_flag = false;

	WEBSTATIC hud_messages_gui hud_messages;

	WEBSTATIC std::atomic<augs::frame_num_type> current_frame = 0;

	WEBSTATIC auto load_all = [&](const all_viewables_defs& new_defs) {
		const auto frame_num = current_frame.load();

		std::optional<arena_player_metas> new_player_metas;
		std::optional<ad_hoc_atlas_subjects> new_ad_hoc_images;

		if (streaming.avatars.work_slot_free(frame_num)) {
			visit_current_setup([&](auto& setup) {
				new_player_metas = setup.get_new_player_metas();
				new_ad_hoc_images = setup.get_new_ad_hoc_images();
			});

			if (!has_current_setup()) {
				new_ad_hoc_images = map_catalogue_gui.get_new_ad_hoc_images();
			}
		}

#if WEB_LOWEND
		auto in_fonts = config.gui_fonts;

		if (is_during_tutorial()) {
			/*
				Bring back the default because it looked good
				for context tips.
			*/
			in_fonts.larger_gui.size_in_pixels = 32.0f;
		}
#else
		const auto& in_fonts = config.gui_fonts;
#endif

		streaming.load_all({
			frame_num,
			new_defs,
			necessary_image_definitions,
			in_fonts,
			loaded_gui_fonts_ratio,
			config.content_regeneration,
			get_unofficial_content_dir(),
			get_general_renderer(),
			renderer_backend.get_max_texture_size(),

			new_player_metas,
			new_ad_hoc_images,
			audio_buffers
		});
	};

	WEBSTATIC bool set_rich_presence_now = true;
	WEBSTATIC int setup_just_launched = 0;

	(void)set_rich_presence_now;

	WEBSTATIC auto setup_launcher = [&](auto&& setup_init_callback) {
		setup_just_launched = 1;

		::steam_clear_rich_presence();
		set_rich_presence_now = true;
		
		get_audiovisuals().get<particles_simulation_system>().clear();
		
		game_gui_mode_flag = false;

		audio_buffers.finish();
		audio_buffers.stop_all_sources();

		get_audiovisuals().get<sound_system>().clear();

		network_stats = {};
		server_stats = {};

#if BUILD_NETWORKING
		if (main_menu != nullptr) {
			was_browser_open_in_main_menu = browse_servers_gui.show;
		}

		browse_servers_gui.close();
#endif

		map_catalogue_gui.close();
		settings_gui.close();

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

#if BUILD_NETWORKING
		if (main_menu != nullptr) {
			if (was_browser_open_in_main_menu) {
				browse_servers_gui.open();
			}

#if BUILD_NATIVE_SOCKETS
			if (auxiliary_socket == std::nullopt) {
				recreate_auxiliary_socket();

				if (!nat_detection_complete()) {
					restart_nat_detection();
				}
			}
#endif
		}
#endif
	};

	WEBSTATIC auto emplace_current_setup = [&p = current_setup] (auto tag, auto&&... args) {
		using Tag = decltype(tag);
		using T = type_of_in_place_type_t<Tag>; 

		if (p == nullptr) {
			p = std::make_unique<setup_variant>(
				tag,
				std::forward<decltype(args)>(args)...
			);
		}
		else {
			p->emplace<T>(std::forward<decltype(args)>(args)...);
		}
	};

#if BUILD_DEBUGGER_SETUP
	WEBSTATIC auto launch_debugger = [&](auto&&... args) {
		setup_launcher([&]() {
			emplace_current_setup(std::in_place_type_t<debugger_setup>(),
				std::forward<decltype(args)>(args)...
			);
		});
	};
#else
	WEBSTATIC auto launch_debugger = [&](auto&&...) {

	};
#endif

	WEBSTATIC auto save_last_activity = [&](const activity_type mode) {
		if (mode != activity_type::CLIENT) {
			change_with_save([mode](config_json_table& cfg) {
				cfg.last_activity = mode;
			});
		}
	};

	WEBSTATIC auto launch_main_menu = [&]() {
		if (!has_main_menu()) {
			main_menu_gui = {};
			
			setup_launcher([&]() {
				emplace_main_menu(*official, config.main_menu);
			});

			map_catalogue_gui.request_refresh();
			leaderboards_gui.request_refresh();

			streaming.recompress_demos();
		}
	};

#if BUILD_NETWORKING
	WEBSTATIC auto get_browse_servers_input = [&]() {
		return browse_servers_input {
			config.server_list_provider,
			config.client_connect,
			displayed_connecting_server_name,
			config.faction_view,
			config.streamer_mode && config.streamer_mode_flags.community_servers
		};
	};

	WEBSTATIC auto launch_client_setup = [&]() {
		auto connect_string = config.client_connect;

#if PLATFORM_WEB
		const bool is_official_connect_string = ::is_official_webrtc_id(connect_string);
		const bool requires_sign_in = is_official_connect_string && begins_with(connect_string, "ranked");

		if (requires_sign_in) {
			if (is_auth_expired()) {
				LOG("Trying to connect with an expired auth.");

				if (auth_self_refresh()) {
					LOG("Requested self-refresh. Delaying connect. connect_string: %x", connect_string);

					social_sign_in.connect_string_post_sign_in = connect_string;
					return false;
				}

				social_log_out();
			}

			if (token_still_valid_check.valid()) {
				if (!token_still_valid_check.get()) {
					social_log_out();
				}
			}

			if (!is_signed_in()) {
				social_sign_in.open("play ranked matches", connect_string);
				return false;
			}
		}
#endif

		LOG("Launching client setup with connect string: %x", connect_string);

		streaming.wait_demos_compressed();

#if !PLATFORM_WEB
		if (is_steam_client) {
			LOG("Calling steam_request_auth_ticket.");
			steam_auth_request_id = ::steam_request_auth_ticket("hypersomnia_gameserver");
			LOG_NVPS(steam_auth_request_id);
		}
#endif

		setup_launcher([&]() {
			const auto preferred_binding_port = 0;

			emplace_current_setup(std::in_place_type_t<client_setup>(),
				*official,
				connect_string,
				displayed_connecting_server_name,
				config.client,
				preferred_binding_port,

				config.webrtc_signalling_server_url
			);
		});

#if PLATFORM_WEB
		const bool send_auth = requires_sign_in;

		if (send_auth) {
			/*
				At this point we verified we're signed in.
			*/

			on_specific_setup([&](client_setup& setup) {
				LOG("Server requires_sign_in. Calling send_auth_ticket.");

				auto& cached_auth = social_sign_in.cached_auth;
				setup.send_auth_ticket(cached_auth);
			});
		}
		else {
			LOG("Server doesn't require sign in.");
		}
#endif

		displayed_connecting_server_name.clear();

		std::string saved_for_next_launch;

#if !PLATFORM_WEB
		if (const auto found = find_chosen_server_info(); found && found->is_official_server()) {
			saved_for_next_launch = found->meta.official_url;
		}
#endif

		change_with_save(
			[&](auto& cfg) {
				cfg.client = config.client;

				if (const bool should_save = !saved_for_next_launch.empty()) {
					cfg.client_connect = saved_for_next_launch;
					cfg.last_activity = activity_type::CLIENT;
				}
				else {
					if (const bool same_as_last_launch = cfg.client_connect == connect_string) {
						cfg.last_activity = activity_type::CLIENT;
					}
					else {
						cfg.last_activity = activity_type::MAIN_MENU;
					}
				}
			}
		);

		return true;
	};
#endif

	WEBSTATIC auto launch_editor = [&](auto&&... args) {
		setup_launcher([&]() {
			emplace_current_setup(
				std::in_place_type_t<editor_setup>(),
				config.editor,
				*official,
				std::forward<decltype(args)>(args)...
			);
		});

		save_last_activity(activity_type::EDITOR);
	};

	WEBSTATIC auto launch_setup = [&](const activity_type mode) {
		if (mode != activity_type::TUTORIAL && !config.skip_tutorial) {
			change_with_save(
				[&](auto& cfg) {
					cfg.skip_tutorial = true;
				}
			);
		}

		if (map_catalogue_gui.is_downloading()) {
			LOG("Cannot launch %x. Download in progress.", augs::enum_to_string(mode));
			return;
		}

		LOG("Launched mode: %x", augs::enum_to_string(mode));

		switch (mode) {
			case activity_type::MAIN_MENU:
				launch_main_menu();
				break;

#if BUILD_NETWORKING
			case activity_type::CLIENT: {
				if (!launch_client_setup()) {
					if (!has_main_menu()) {
						launch_main_menu();
					}

					return;
				}

				break;
			}

			case activity_type::SERVER: {
#if BUILD_NATIVE_SOCKETS
				const auto bound_port = get_bound_local_port();
				delete_auxiliary_socket();

				LOG("Starting server setup. Binding to a port: %x (%x was preferred)", bound_port, last_requested_local_port);

				auto start = config.server_start;
				start.port = bound_port;
#else
				auto start = config.server_start;
#endif
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<server_setup>(),
						*official,
						start,
						config.server,
						canon_config_with_confd.server,
						config.server_private,
						config.client,
						std::nullopt,
						params.suppress_server_webhook,
						assigned_teams,
						config.webrtc_signalling_server_url,
						"",
						start_server_gui.get_initial_overrides()
					);
				});

				break;
			}
#endif

			case activity_type::DEBUGGER:
				launch_debugger();

				break;

			case activity_type::EDITOR:
				if (!params.editor_target.empty()) {
					launch_editor(params.editor_target);
				}
				else {
					try {
						const auto loaded_path = augs::path_type(augs::file_to_string(get_editor_last_project_path()));

						try {
							launch_editor(loaded_path);
							break;
						}
						catch (const std::runtime_error& err) {
							const auto full_content = typesafe_sprintf("Failed to load: %x\nReason:\n\n%x", loaded_path, err.what());
							failed_to_load_arena_popup = simple_popup { "Error", full_content, "" };
						}
						catch (...) {

						}
					}
					catch (...) {
						LOG("No %x detected. Launching the project selector.", get_editor_last_project_path());
					}
					break;
				}

			case activity_type::EDITOR_PROJECT_SELECTOR:
				setup_launcher([&]() {
					emplace_current_setup(
						std::in_place_type_t<project_selector_setup>()
					);
				});

				break;

			case activity_type::SHOOTING_RANGE:
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<test_scene_setup>(),
						config.client.get_nickname(),
						get_my_avatar_bytes(),
						*official,
						test_scene_type::SHOOTING_RANGE
					);
				});

				break;

			case activity_type::TUTORIAL:
				setup_launcher([&]() {
					emplace_current_setup(std::in_place_type_t<test_scene_setup>(),
						config.client.get_nickname(),
						get_my_avatar_bytes(),
						*official,
						test_scene_type::TUTORIAL
					);
				});

				break;

			default:
				ensure(false && "The launch_setup mode you have chosen is currently out of service.");
				break;
		}

		if (mode == activity_type::SERVER) {
			save_last_activity(activity_type::MAIN_MENU);
		}
		else {
			save_last_activity(mode);
		}
	};

	WEBSTATIC bool client_start_requested = false;
	WEBSTATIC bool server_start_requested = false;
	(void)client_start_requested;
	(void)server_start_requested;

#if BUILD_NETWORKING
	WEBSTATIC auto start_client_setup = [&]() {
		client_start_requested = false;

		launch_setup(activity_type::CLIENT);
	};

	WEBSTATIC auto perform_browse_servers = [&]() {
		browse_servers_gui.allow_ranked_servers = ranked_servers_enabled;

		const bool perform_result = browse_servers_gui.perform(get_browse_servers_input());

		if (perform_result) {
			start_client_setup();
		}
	};

	WEBSTATIC auto perform_start_client_gui = [&](const auto frame_num) {
		auto best_casual = browse_servers_gui.find_best_server(false);

		const bool perform_result = start_client_gui.perform(
			best_casual,
			browse_servers_gui.refresh_in_progress(),
			frame_num,
			get_general_renderer(), 
			streaming.avatar_preview_tex, 
			window, 
			config.client_connect, 
			displayed_connecting_server_name,
			config.client,
			config.streamer_mode
		);

		if (perform_result || client_start_requested) {
			if (client_start_requested) {
				LOG("Launch client due to client_start_requested.");
			}

			start_client_setup();
		}

		if (start_client_gui.request_server_list_open) {
			start_client_gui.request_server_list_open = false;

			browse_servers_gui.open();

			if (best_casual.has_value()) {
				browse_servers_gui.select_server(*best_casual);
			}
		}
	};

	WEBSTATIC auto perform_start_server_gui = [&]() {
		const bool launched_from_server_start_gui = start_server_gui.perform(
			config.server_start, 
			config.server, 
#if BUILD_NATIVE_SOCKETS
			nat_detection.has_value() ? std::addressof(*nat_detection) : nullptr,
#else
			nullptr,
#endif
			get_bound_local_port()
		);

		if (launched_from_server_start_gui || server_start_requested) {
			server_start_requested = false;

			change_with_save(
				[&](auto& cfg) {
					cfg.server_start = config.server_start;
					cfg.client = config.client;
					cfg.server = config.server;
				}
			);

			if (start_server_gui.type == dedicated_or_integrated::INTEGRATED) {
				launch_setup(activity_type::SERVER);
			}
			else {
				const auto chosen_port = get_bound_local_port();
				LOG_NVPS(get_bound_local_port(), chosen_server_port());

				delete_auxiliary_socket();

				const auto cmd_line = typesafe_sprintf(
					"--dedicated-server --server-port %x",
					chosen_port
				);

				augs::restart_application(argc, argv, params.exe_path.string(), { cmd_line });
				displayed_connecting_server_name = "local dedicated server";

				const auto address_str = typesafe_sprintf("%x:%x", config.server_start.ip, chosen_port);

				LOG("Connecting to the launched dedicated server at: %x", address_str);

				config.client_connect = address_str;

				launch_setup(activity_type::CLIENT);
			}
		}
	};
#endif

	WEBSTATIC auto get_map_catalogue_input = [&]() {
		return map_catalogue_input {
			config.server.external_arena_files_provider,
			streaming.ad_hoc.in_atlas,
			streaming.necessary_images_in_atlas,
			window,
			config.streamer_mode && config.streamer_mode_flags.map_catalogue
		};
	};

	WEBSTATIC auto perform_leaderboards = [&]() {
		leaderboards_gui.show = !browse_servers_gui.show;

		leaderboards_gui.perform({
			config.client.get_nickname(),
#if PLATFORM_WEB
			social_sign_in.cached_auth.profile_id,
#else
			typesafe_sprintf("steam_%x", steam_id),
#endif

			config.main_menu.leaderboards_provider_url,
			streaming.necessary_images_in_atlas,
			streaming.avatar_preview_tex,

			menu_ltrb
#if PLATFORM_WEB
			, is_signed_in(),
			params.is_crazygames
#endif
		});
	};

	WEBSTATIC auto perform_map_catalogue = [&]() {
		const bool perform_result = map_catalogue_gui.perform(get_map_catalogue_input());

		if (perform_result) {
			change_with_save(
				[&](auto& cfg) {
					cfg.server.external_arena_files_provider = config.server.external_arena_files_provider;
				}
			);
		}

		if (map_catalogue_gui.open_host_server_window.has_value()) {
			config.server.arena = *map_catalogue_gui.open_host_server_window;
			start_server_gui.open();

			map_catalogue_gui.open_host_server_window = std::nullopt;
		}
	};

	WEBSTATIC auto get_viewable_defs = [&]() -> const all_viewables_defs& {
		return visit_current_setup([&](auto& setup) -> const all_viewables_defs& {
			return setup.get_viewable_defs();
		});
	};

	WEBSTATIC auto create_game_gui_deps = [&](const config_json_table& viewing_config) {
		return game_gui_context_dependencies {
			get_viewable_defs().image_definitions,
			streaming.images_in_atlas,
			streaming.necessary_images_in_atlas,
			streaming.get_loaded_gui_fonts().gui,
			get_audiovisuals().randomizing,
			viewing_config.game_gui,
			viewing_config.hotbar
		};
	};

	WEBSTATIC auto create_menu_context_deps = [&](const config_json_table& viewing_config) {
		return menu_context_dependencies{
			streaming.necessary_images_in_atlas,
			streaming.get_loaded_gui_fonts().gui,
			necessary_sounds,
			viewing_config.audio_volume,
			background_setup != nullptr,
			has_current_setup() && std::holds_alternative<editor_setup>(*current_setup),
			is_during_tutorial(),
			is_shooting_range(),
			still_querying_server_info(),
			would_abandon_ranked_match(),
			should_hide_invite_to_join()
		};
	};

	WEBSTATIC auto get_game_gui_subject = [&]() -> const_entity_handle {
		const auto& viewed_cosmos = visit_current_setup([&](auto& setup) -> const cosmos& {
			return setup.get_viewed_cosmos();
		});

		const auto gui_character_id = visit_current_setup([&](auto& setup) {
			return setup.get_game_gui_subject_id();
		});

		return viewed_cosmos[gui_character_id];
	};

	WEBSTATIC auto get_viewed_character = [&]() -> const_entity_handle {
		const auto& viewed_cosmos = visit_current_setup([&](auto& setup) -> const cosmos& {
			return setup.get_viewed_cosmos();
		});

		const auto viewed_character_id = visit_current_setup([&](auto& setup) {
			return setup.get_viewed_character_id();
		});

		return viewed_cosmos[viewed_character_id];
	};

	WEBSTATIC auto get_controlled_character = [&]() -> const_entity_handle {
		const auto& viewed_cosmos = visit_current_setup([&](auto& setup) -> const cosmos& {
			return setup.get_viewed_cosmos();
		});

		const auto controlled_character_id = visit_current_setup([&](auto& setup) {
			return setup.get_controlled_character_id();
		});

		return viewed_cosmos[controlled_character_id];
	};
		
	WEBSTATIC auto should_draw_game_gui = [&]() {
		{
			bool should = true;

#if BUILD_DEBUGGER_SETUP
			on_specific_setup([&](debugger_setup& setup) {
				if (!setup.anything_opened() || setup.is_editing_mode()) {
					should = false;
				}
			});
#endif

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

	WEBSTATIC auto get_logic_eye = [&](const bool with_edge_zoomout) {
		if (const auto custom = visit_current_setup(
			[](const auto& setup) { 
				return setup.find_current_camera_eye(); 
			}
		)) {
			return *custom;
		}

		if (get_viewed_character().dead()) {
			return camera_eye();
		}

		return gameplay_camera.get_current_eye(with_edge_zoomout);
	};


	WEBSTATIC auto get_camera_edge_zoomout_mult = [&]() {		
		return gameplay_camera.current_edge_zoomout_mult;
	};

	WEBSTATIC auto get_camera_requested_fov_expansion = [&]() {		
		auto result = 1.0f / get_logic_eye(false).zoom;

		if (get_camera_edge_zoomout_mult() > 0.001f) {
			result /= max_zoom_out_at_edges_v;
		}

		return result;
	};

	WEBSTATIC auto get_camera_eye = [&](const config_json_table& viewing_config, bool with_edge_zoomout = true) {		
		auto logic_eye = get_logic_eye(with_edge_zoomout);

		const auto considered_fov_size = viewing_config.drawing.fog_of_war.size;
		const float target_visible_pixels_x = considered_fov_size.x;

		const bool should_zoom_to_fov = viewing_config.drawing.snap_zoom_to_fov_size;

		if (should_zoom_to_fov && target_visible_pixels_x != 0.0f) {
			const float screen_x = float(logic_get_screen_size().x);

			float closest_zoom_multiple = 1.0f;
			float closest_multiple_dist = std::numeric_limits<float>::max();

			for (int i = -10; i <= 10; ++i) {
				if (i == 0) {
					continue;
				}

				const auto mult = i < 0 ? (1.0f / (-i)) : float(i);
				const auto scaled = target_visible_pixels_x * mult;

				const auto dist = std::abs(scaled - screen_x);

				if (dist < closest_multiple_dist) {
					closest_multiple_dist = dist;
					closest_zoom_multiple = mult;
				}
			}

			const auto eps = viewing_config.drawing.snap_zoom_to_multiple_if_different_by_pixels;

			auto zoom_to_snap_to_fov = screen_x / target_visible_pixels_x;

			if (closest_multiple_dist <= eps) {
				zoom_to_snap_to_fov = closest_zoom_multiple;
			}

			logic_eye.zoom *= zoom_to_snap_to_fov;
		}

		logic_eye.zoom *= std::max(1.0f, viewing_config.drawing.custom_zoom);

		return logic_eye;
	};

	WEBSTATIC auto get_camera_cone = [&](const config_json_table& viewing_config) {		
		return camera_cone(get_camera_eye(viewing_config), logic_get_screen_size());
	};

	WEBSTATIC auto get_nonzoomedout_visible_world_area = [&](const config_json_table& viewing_config) {
		return vec2(logic_get_screen_size()) / get_camera_eye(viewing_config, no_edge_zoomout_v).zoom;
	};

	WEBSTATIC auto get_queried_cone = [&](const config_json_table& viewing_config) {		
		const auto query_mult = viewing_config.session.camera_query_aabb_mult;

		const auto queried_cone = [&]() {
			auto c = get_camera_cone(viewing_config);
			c.eye.zoom /= query_mult;
			return c;
		}();

		return queried_cone;
	};

	WEBSTATIC auto get_setup_customized_config = [&]() {
		return visit_current_setup([&]<typename S>(S& setup) {
			auto config_copy = config;

			/*
				For example, the main menu might want to disable HUD or tune down the sound effects.
				Editor might want to change the window name to the current file.
			*/

			setup.customize_for_viewing(config_copy);
			setup.apply(config_copy);

			if constexpr(is_one_of_v<S, client_setup, server_setup>) {
				setup.apply_nonzoomedout_visible_world_area(get_nonzoomedout_visible_world_area(config_copy));
			}

			if (get_camera_eye(config_copy).zoom < 1.f) {
				/* Force linear filtering when zooming out */
				config_copy.renderer.default_filtering = augs::filtering_type::LINEAR;
			}

			if (config_copy.drawing.cinematic_mode) {
				auto& d = config_copy.drawing;

				d.draw_crosshairs = false;
				//d.draw_weapon_laser = false;
				d.draw_aabb_highlighter = false;
				d.draw_inventory = false;
				d.draw_hotbar = false;
				d.draw_area_markers.is_enabled = false;
				d.draw_callout_indicators.is_enabled = false;
				d.enemy_hud_mode = character_hud_type::NONE;
				d.draw_hp_bar = false;
				d.draw_cp_bar = false;
				d.draw_pe_bar = false;
				d.draw_character_status = false;
				d.draw_remaining_ammo = false;

				d.draw_offscreen_indicators = false;
				d.draw_offscreen_callouts = false;
				d.draw_nicknames = false;
				d.draw_small_health_bars = false;
				d.draw_health_numbers = false;
				//d.draw_damage_indicators = false;

				d.draw_teammate_indicators.is_enabled = false;
				d.draw_danger_indicators.is_enabled = false;
				d.draw_tactical_indicators.is_enabled = false;
				d.print_current_character_callout = false;
				d.show_danger_indicator_for_seconds = 0;
				d.show_death_indicator_for_seconds = 0;
				d.nickname_characters_for_offscreen_indicators = 0;
			}

			if (!game_gui_mode_flag) {
				config_copy.drawing.draw_inventory = false;
			}

			if (ad_state == ad_state_type::PLAYING) {
				config_copy.audio_volume.master = 0.0f;
			}

			if (social_sign_in.is_open()) {
				config_copy.arena_mode_gui.context_tip_settings.is_enabled = false;
			}

			return config_copy;
		});
	};

	WEBSTATIC auto is_replaying_demo = [&]() {
		bool result = false;

		on_specific_setup([&](client_setup& setup) {
			result = setup.is_replaying();
		});

		return result;
	};

	WEBSTATIC auto viewer_is_spectator = [&]() {
		if (is_replaying_demo()) {
			return true;
		}

		bool result = false;

		on_specific_setup([&](client_setup& setup) {
			result = setup.get_assigned_faction() == faction_type::SPECTATOR;
		});

		on_specific_setup([&](server_setup& setup) {
			result = setup.get_assigned_faction() == faction_type::SPECTATOR;
		});

		return result;
	};

	WEBSTATIC augs::timer ad_hoc_animation_timer;

	WEBSTATIC auto perform_setup_custom_imgui = [&]() {
		/*
			The debugger setup might want to use IMGUI to create views of entities or resources,
			thus we ask the current setup for its custom ImGui logic.

			Similarly, client and server setups might want to perform ImGui for things like team selection.
		*/

		visit_current_setup([&](auto& setup) {
			streaming.ad_hoc.in_atlas.animation_time = ad_hoc_animation_timer.get<std::chrono::seconds>();

			auto imgui_in = perform_custom_imgui_input {
				window,
				streaming.images_in_atlas,
				streaming.ad_hoc.in_atlas,
				streaming.necessary_images_in_atlas,
				config,
				is_replaying_demo()
			};

			const auto result = setup.perform_custom_imgui(imgui_in);

			using S = remove_cref<decltype(setup)>;

			switch (result) {
				case custom_imgui_result::GO_TO_MAIN_MENU:
					launch_setup(activity_type::MAIN_MENU);
					break;

				case custom_imgui_result::GO_TO_PROJECT_SELECTOR:
					launch_setup(activity_type::EDITOR_PROJECT_SELECTOR);
					break;

				case custom_imgui_result::RETRY:
					if constexpr(std::is_same_v<S, client_setup>) {
						launch_setup(activity_type::CLIENT);
					}

					break;

				case custom_imgui_result::OPEN_SELECTED_PROJECT:
					if constexpr(std::is_same_v<S, project_selector_setup>) {
						auto backup = std::make_unique<setup_variant>(setup);

						const auto path = setup.get_selected_project_path();

						try {
							launch_editor(path);
						}
						catch (const std::runtime_error& err) {
							const auto full_content = typesafe_sprintf("Failed to load: %x\nReason:\n\n%x", path, err.what());
							failed_to_load_arena_popup = simple_popup { "Error", full_content, "" };

							current_setup = std::move(backup);
						}
					}

					break;

				case custom_imgui_result::PLAYTEST_ONLINE:
#if BUILD_NETWORKING
					if constexpr(std::is_same_v<S, editor_setup>) {
						setup.prepare_for_online_playtesting();

#if BUILD_NATIVE_SOCKETS
						const auto bound_port = get_bound_local_port();
						delete_auxiliary_socket();

						LOG("Starting server setup for playtesting. Binding to a port: %x (%x was preferred)", bound_port, last_requested_local_port);

						auto start = config.server_start;
						start.port = bound_port;
#else
						auto start = config.server_start;
#endif

						auto playtest_vars = config.server;
						playtest_vars.server_name = "${MY_NICKNAME} is creating " + setup.get_arena_name();
						playtest_vars.playtesting_context = setup.make_playtesting_context();
						playtest_vars.arena = setup.get_arena_name();

						background_setup = std::move(current_setup);

						setup_launcher([&]() {
							emplace_current_setup(std::in_place_type_t<server_setup>(),
								*official,
								start,
								playtest_vars,
								canon_config_with_confd.server,
								config.server_private,
								config.client,
								std::nullopt,
								params.suppress_server_webhook,
								assigned_teams,
								config.webrtc_signalling_server_url
							);
						});
					}
#endif

					break;

				case custom_imgui_result::QUIT_PLAYTESTING:
					restore_background_setup();
					break;

				default: break;
			}
		});
	};

#if !PLATFORM_WEB
	WEBSTATIC auto process_steam_callbacks = [&]() {
		auto connect_to = [&](const std::string& address_string) {
			if (address_string.length() > 0) {
				LOG("(Steam Callback) Joining server (length: %x): %x", address_string.length(), address_string);

				config.client_connect = address_string;

#if BUILD_NETWORKING
				/* We must know if it's behind NAT */

				browse_servers_gui.sync_download_server_entry(
					get_browse_servers_input(),
					config.client_connect
				);

				start_client_setup();
#endif
			}
			else {
				LOG("(Steam Callback) Server address was empty.");
			}
		};

		using steam_queue_type = std::vector<
			std::variant<
				steam_new_url_launch_parameters,
				steam_new_join_game_request,
				steam_change_server_request,
				steam_auth_ticket
			>
		>; 
		
		thread_local steam_queue_type steam_events;

		steam_events.clear();

		auto handler_c = [](void* ctx, uint32_t idx, void* object) {
			auto check_index = [&]<typename T>(T) {
				if (idx == T::index) {
					auto typed_ctx = reinterpret_cast<steam_queue_type*>(ctx);
					typed_ctx->push_back(*reinterpret_cast<T*>(object));
				}
			};

			check_index(steam_new_url_launch_parameters());
			check_index(steam_new_join_game_request());
			check_index(steam_change_server_request());
			check_index(steam_auth_ticket());
		};

		steam_run_callbacks(handler_c, &steam_events);

		auto handler_lbd = [&]<typename E>(const E& event) {
			if constexpr(std::is_same_v<E, steam_new_url_launch_parameters>) {
				LOG("(Steam Callback) steam_new_url_launch_parameters.");
				(void)event;

				connect_to(steam_get_launch_command_line_string());
			}
			else if constexpr(std::is_same_v<E, steam_new_join_game_request>) {
				LOG("(Steam Callback) steam_new_join_game_request.");

				connect_to(std::string(event.connect_cli.data()));
			}
			else if constexpr(std::is_same_v<E, steam_change_server_request>) {
				LOG("(Steam Callback) steam_change_server_request.");

				connect_to(std::string(event.server_address.data()));
			}
			else if constexpr(std::is_same_v<E, steam_auth_ticket>) {
				LOG("(Steam Callback) steam_auth_ticket.");

				if (event.request_id == steam_auth_request_id) {
					visit_current_setup([&](auto& setup) {
						using T = remove_cref<decltype(setup)>;

						if constexpr(is_one_of_v<T, client_setup>) {
							setup.send_auth_ticket(event);
						}
					});
				}
			}
			else {
				static_assert(always_false_v<E>, "Non-exhaustive");
			}
		};

		for (auto& event : steam_events) {
			std::visit(handler_lbd, event);
		}
	};
#endif

	WEBSTATIC auto do_imgui_pass = [&](const auto frame_num, auto& new_window_entropy, const auto& frame_delta, const bool in_direct_gameplay) {
		(void)frame_num;

		bool freeze_imgui_inputs = false;

		on_specific_setup([&](editor_setup& editor) {
			if (editor.is_mover_active()) {
				freeze_imgui_inputs = true;
			}
		});

		perform_imgui_pass(
			window,
			new_window_entropy,
			logic_get_screen_size(),
			frame_delta,
			canon_config,
			canon_config_with_confd,
			config,
			last_saved_config,
			runtime_prefs_path,
			settings_gui,
			audio,
			[&]() {
#if BUILD_NATIVE_SOCKETS
				auto do_nat_detection_logic = [&]() {
					bool do_nat_detection = true;

					visit_current_setup([&](const auto& setup) {
						using T = remove_cref<decltype(setup)>;

						if constexpr(is_one_of_v<T, client_setup, server_setup>) {
							/* Don't do NAT detection when it's already too late to do so */
							do_nat_detection = false;
						}
					});

					if (!do_nat_detection) {
						return;
					}

					if (last_requested_local_port != chosen_server_port()) {
						recreate_auxiliary_socket();
						restart_nat_detection();
					}

					if (nat_detection.has_value()) {
						if (config.nat_detection != nat_detection->get_settings()) {
							restart_nat_detection();
						}

						if (auxiliary_socket.has_value()) {
							nat_detection->advance(auxiliary_socket->socket);
						}
					}
				};

				do_nat_detection_logic();
#endif
				if (ingame_menu.show) {
					on_specific_setup([&](server_setup& server) {
						server.do_integrated_rcon_gui(true);

						if (!server.is_running()) {
							ingame_menu.show = false;
						}
					});

					on_specific_setup([&](client_setup& client) {
						if (client.get_rcon_level() > rcon_level_type::DENIED) {
							client.do_rcon_gui(true);

							if (!client.is_connected()) {
								ingame_menu.show = false;
							}
						}
					});
				}


#if BUILD_NETWORKING
				if (start_client_gui.show) {
					if (start_client_gui.current_tab == start_client_tab_type::BEST_SERVER) {
						if (!browse_servers_gui.refreshed_at_least_once()) {
							browse_servers_gui.refresh_server_list(get_browse_servers_input());
						}
					}
				}

				if (start_client_gui.request_refresh_best_server) {
					start_client_gui.request_refresh_best_server = false;

					browse_servers_gui.refresh_server_list(get_browse_servers_input());
				}

#if BUILD_NATIVE_SOCKETS
				browse_servers_gui.advance_ping_logic();
#endif

				if (const bool show_server_browser = !has_current_setup() || ingame_menu.show) {
					perform_browse_servers();
				}
#endif

				if (const bool show_map_catalogue = !has_current_setup()) {
					perform_map_catalogue();
				}

				if (const bool show_leaderboards = !has_current_setup()) {
					perform_leaderboards();
#if PLATFORM_WEB
					if (is_auth_expired()) {
						if (!auth_can_self_refresh()) {
							LOG("Leaderboards: auth expired. Logging out.");
							social_log_out();
						}
					}

					if (leaderboards_gui.wants_sign_in) {
						leaderboards_gui.wants_sign_in = false;

						social_sign_in.open();
					}

					if (leaderboards_gui.wants_log_out) {
						leaderboards_gui.wants_log_out = false;

						social_log_out();
					}
#endif
				}


#if PLATFORM_WEB

				if (const auto new_auth = get_new_auth_data()) { 
					LOG("new_auth. is_signed_in() = %x", is_signed_in());

					social_sign_in.cached_auth = *new_auth;
					social_sign_in.close();

					augs::save_as_json(*new_auth, CACHED_AUTH_PATH);

					bool has_avatar = false;

					try {
						augs::image avatar;

						avatar.from_bytes_stbi(new_auth->avatar_bytes, "new_auth->avatar_bytes");

						on_specific_setup([&](test_scene_setup& setup) {
							setup.set_new_avatar(new_auth->avatar_bytes);
						});

						const auto max_s = static_cast<unsigned>(max_avatar_side_v);
						avatar.scale(vec2u::square(max_s));
						avatar.save_as_png(CACHED_AVATAR);

						streaming.requested_avatar_preview = CACHED_AVATAR;
						has_avatar = true;
					}
					catch (...) {
						LOG("Failed to load the downloaded avatar.");
					}

					change_with_save(
						[&](auto& cfg) {
							cfg.prompted_for_sign_in_once = true;

							if (has_avatar) {
								cfg.client.avatar_image_path = CACHED_AVATAR;
							}
						}
					);

					auto& connect_string = social_sign_in.connect_string_post_sign_in;

					if (!connect_string.empty()) {
						LOG("Triggering client connect post-auth: %x", connect_string);

						config.client_connect = connect_string;

						connect_string.clear();
						start_client_setup();
					}
				}
				else if (valid_and_is_ready(token_still_valid_check)) {
					if (!token_still_valid_check.get()) {
						social_log_out();
					}
				}

				const bool should_prompt_for_social_sign_in = [&]() {
					if (!streaming.completed_all_loading()) {
						return false;
					}

					if (config.prompted_for_sign_in_once) {
						return false;
					}

					if (is_signed_in()) {
						return false;
					}

					return true;
				}();

				config.client.signed_in.nickname = social_sign_in.cached_auth.profile_name;

				if (should_prompt_for_social_sign_in) {
					if (!social_sign_in.is_open()) {
						config.client.nickname = gen_random_nickname();
						social_sign_in.guest_nickname = config.client.nickname;
						social_sign_in.open();
					}
				}

				perform_social_sign_in_popup(config.prompted_for_sign_in_once);
#endif
			

				if (!has_current_setup()) {
					perform_last_exit_incorrect();
#if BUILD_NETWORKING
					perform_start_client_gui(frame_num);
					perform_start_server_gui();
#endif
				}

				perform_failed_to_load_arena_popup();
				perform_abandon_are_you_sure_popup();

				streaming.display_loading_progress();
			},

			[&]() {
				perform_setup_custom_imgui();
			},

			/* Flags controlling IMGUI behaviour */

			ingame_menu.show,
			has_current_setup(),

			in_direct_gameplay,
			float_tests_succeeded
		);

		if (freeze_imgui_inputs) {
			ImGui::GetIO().WantCaptureMouse = false;
		}

		new_window_entropy = augs::imgui::filter_inputs(new_window_entropy);
	};

	WEBSTATIC auto decide_on_cursor_clipping = [&](const bool in_direct_gameplay, const auto& cfg) {
		get_write_buffer().should_clip_cursor = ad_state == ad_state_type::NONE && (
			in_direct_gameplay
			|| cfg.window.draws_own_cursor()
		);
	};

	WEBSTATIC auto get_current_input_settings = [&](const auto& cfg) {
		auto settings = cfg.input;

#if BUILD_NETWORKING
		on_specific_setup([&](client_setup& setup) {
			settings.character = setup.get_current_requested_settings().public_settings.character_input;
		});
#endif

		return settings;
	};

	WEBSTATIC auto handle_app_intent = [&](const app_intent_type intent) {
		using T = decltype(intent);

		switch (intent) {
			case T::SHOW_PERFORMANCE: {
				change_with_save([&](config_json_table& cfg) {
					bool& f = cfg.session.show_performance;
					f = !f;

					if (f) {
						cfg.session.show_logs = false;
					}
				});

				break;
			}

			case T::SHOW_LOGS: {
				change_with_save([&](config_json_table& cfg) {
					bool& f = cfg.session.show_logs;
					f = !f;

					if (f) {
						cfg.session.show_performance = false;
					}
				});

				break;
			}

			case T::TOGGLE_STREAMER_MODE: {
				bool& f = config.streamer_mode;
				f = !f;
				break;
			}

			case T::TOGGLE_CINEMATIC_MODE: {
				bool& f = config.drawing.cinematic_mode;
				f = !f;
				break;
			}

			default: break;
		}
	};
	
	WEBSTATIC auto handle_general_gui_intent = [&](const general_gui_intent_type intent) {
		using T = decltype(intent);

		switch (intent) {
			case T::CLEAR_DEBUG_LINES:
				DEBUG_PERSISTENT_LINES.clear();
				return true;

			case T::TOGGLE_WEAPON_LASER: {
				bool& f = config.drawing.draw_weapon_laser;
				f = !f;
				return true;
			}

			case T::TOGGLE_MOUSE_CURSOR: {
				bool& f = game_gui_mode_flag;
				f = !f;
				return true;
			}
			
			default: return false;
		}
	};
 
	WEBSTATIC auto main_ensure_handler = [&]() {
		if (has_current_setup() || has_main_menu()) {
			/*
				ensures could be called in the middle of constructing a setup,
				which is why we need to check if a setup exists.
			*/
			visit_current_setup(
				[&](auto& setup) {
					setup.ensure_handler();
				}
			);
		}
	};

	::ensure_handler = main_ensure_handler;

	WEBSTATIC bool should_quit = false;

	WEBSTATIC augs::event::state common_input_state;
	common_input_state.mouse.pos = window.get_last_mouse_pos();

	WEBSTATIC std::function<void()> request_quit;

	WEBSTATIC auto do_main_menu_option = [&](const main_menu_button_type t) {
		LOG("Menu option: %x", augs::enum_to_string(t));
		
		using T = decltype(t);

		switch (t) {
			case T::DISCORD:
				augs::open_url("https://discord.com/invite/YC49E4G");
				break;

			case T::GITHUB:
				augs::open_url("https://github.com/TeamHypersomnia/Hypersomnia");
				break;

			case T::STEAM:
				augs::open_url("https://store.steampowered.com/app/2660970/Hypersomnia/");
				break;

			case T::DOWNLOAD_MAPS:
				map_catalogue_gui.open();
				break;

			case T::BROWSE_SERVERS:
#if BUILD_NETWORKING
				browse_servers_gui.open();
#endif

				break;

			case T::QUICK_PLAY:
				if (common_input_state[augs::event::keys::key::LSHIFT]) {
#if !IS_PRODUCTION_BUILD
					client_start_requested = true;
#endif
				}
				else {
					start_client_gui.open();
					start_client_gui.current_tab = start_client_tab_type::BEST_SERVER;
				}

				break;

#if !WEB_LOWEND
			case T::CONNECT_TO_SERVER:
				if (common_input_state[augs::event::keys::key::LSHIFT]) {
#if !IS_PRODUCTION_BUILD
					client_start_requested = true;
#endif
				}
				else {
					start_client_gui.open();
					start_client_gui.current_tab = start_client_tab_type::CUSTOM_ADDRESS;
				}

				break;
#endif
				
			case T::HOST_SERVER:
				start_server_gui.open();

				if (common_input_state[augs::event::keys::key::LSHIFT]) {
					server_start_requested = true;
				}

				break;

			case T::SHOOTING_RANGE:
				launch_setup(activity_type::SHOOTING_RANGE);
				break;

			case T::TUTORIAL:
				launch_setup(activity_type::TUTORIAL);
				break;

#if !WEB_CRAZYGAMES
			case T::EDITOR:
#if WEB_LOWEND
				augs::open_url("https://play.hypersomnia.io/editor/official");
#else
				launch_setup(activity_type::EDITOR_PROJECT_SELECTOR);
#endif
#endif

				break;

			case T::SETTINGS:
				settings_gui.open();
				break;

#if !WEB_LOWEND
			case T::CREDITS:
				augs::open_url("https://hypersomnia.io/credits");
				break;
#endif

#if !PLATFORM_WEB
			case T::QUIT:
				LOG("Quitting due to Quit pressed in main menu.");
				request_quit();
				break;
#endif

			default: break;
		}
	};

	WEBSTATIC auto do_ingame_menu_option = [&](const ingame_menu_button_type t, const bool allow_popup = true) {
		using T = decltype(t);

		switch (t) {
			case T::DISCORD:
				augs::open_url("https://discord.com/invite/YC49E4G");
				break;

			case T::GITHUB:
				augs::open_url("https://github.com/TeamHypersomnia/Hypersomnia");
				break;

			case T::STEAM:
				augs::open_url("https://store.steampowered.com/app/2660970/Hypersomnia/");
				break;

			case T::INVITE_TO_JOIN:
				if (is_during_tutorial()) {
					std::get<test_scene_setup>(*current_setup).request_checkpoint_restart();
					ingame_menu.show = false;
				}
				else if (is_shooting_range()) {
					web_sdk_request_ad();

					std::get<test_scene_setup>(*current_setup).request_checkpoint_restart();
					ingame_menu.show = false;
				}
				else {
					visit_current_setup([&]<typename T>(const T& setup) {
						if constexpr(is_one_of_v<T, server_setup, client_setup>) {
							const auto address = setup.get_connect_string();

							if (address == "") {
								return;
							}

#if WEB_SINGLETHREAD
							return;
#else
							browse_servers_gui.open_matching_server_entry(
								get_browse_servers_input(),
								setup.get_connect_string()
							);
#endif
						}
					});
				}

				break;

			case T::BROWSE_SERVERS:
#if BUILD_NETWORKING
				browse_servers_gui.open();
#endif
				break;

			case T::RESUME:
				ingame_menu.show = false;
				break;

			case T::QUIT_TO_MENU:
				if (allow_popup) {
					if (would_abandon_ranked_match()) {
						make_abandon_popup(T::QUIT_TO_MENU);
						break;
					}
				}

				if (background_setup != nullptr) {
					restore_background_setup();
				}
				else {
					if (has_current_setup() && std::holds_alternative<editor_setup>(*current_setup)) {
						launch_setup(activity_type::EDITOR_PROJECT_SELECTOR);
					}
					else {
#if WEB_CRAZYGAMES
						const bool skip_ad = is_during_tutorial();
						LOG_NVPS(skip_ad);

						if (!skip_ad) {
							web_sdk_request_ad();
						}
#endif

						launch_setup(activity_type::MAIN_MENU);
					}
				}

				break;

			case T::SETTINGS:
				settings_gui.open();
				break;

#if !PLATFORM_WEB
			case T::QUIT_GAME:
				if (allow_popup) {
					if (would_abandon_ranked_match()) {
						make_abandon_popup(T::QUIT_GAME);
						break;
					}
				}

				LOG("Quitting due to Quit pressed in ingame menu.");
				request_quit();
				break;
#endif

			default: break;
		}
	};

	WEBSTATIC auto setup_pre_solve = [&](auto...) {
		get_general_renderer().save_debug_logic_step_lines_for_interpolation(DEBUG_LOGIC_STEP_LINES);
		DEBUG_LOGIC_STEP_LINES.clear();
	};

	WEBSTATIC visible_entities all_visible;

	WEBSTATIC auto get_character_camera = [&](const config_json_table& viewing_config) -> character_camera {
		return { get_viewed_character(), { get_camera_eye(viewing_config), logic_get_screen_size() } };
	};

	WEBSTATIC auto reacquire_visible_entities = [&](
		const vec2i& screen_size,
		const const_entity_handle& viewed_character,
		const config_json_table& viewing_config
	) {
		auto scope = measure_scope(game_thread_performance.camera_visibility_query);

		auto queried_eye = get_camera_eye(viewing_config);
		queried_eye.zoom /= viewing_config.session.camera_query_aabb_mult;

		const auto queried_cone = camera_cone(queried_eye, screen_size);
		const auto& cosm = viewed_character.get_cosmos();

		all_visible.reacquire_all({ 
			cosm, 
			queried_cone, 
			accuracy_type::PROXIMATE,
			get_render_layer_filter(),
			tree_of_npo_filter::all()
		});

		all_visible.sort(cosm);

		game_thread_performance.num_visible_entities.measure(all_visible.count_all());
	};

	WEBSTATIC auto calc_pre_step_crosshair_displacement = [&](const config_json_table& viewing_config) {
		if (get_viewed_character() != get_controlled_character()) {
			return vec2::zero;
		}

		return visit_current_setup([&](const auto& setup) {
			using T = remove_cref<decltype(setup)>;

			if constexpr(!std::is_same_v<T, main_menu_setup>) {
				const auto& total_collected = setup.get_entropy_accumulator();

				const auto input_cfg = get_current_input_settings(viewing_config);

				if (const auto motion = total_collected.calc_motion(
					get_viewed_character(), 
					game_motion_type::MOVE_CROSSHAIR,
					entropy_accumulator::input {
						input_cfg, 
						get_nonzoomedout_visible_world_area(viewing_config)
					}
				)) {
					return vec2(motion->offset) * input_cfg.character.crosshair_sensitivity;
				}
			}

			return vec2::zero;
		});
	};

	WEBSTATIC bool pending_new_state_sample = true;
	WEBSTATIC auto last_sampled_cosmos = cosmos_id_type(-1);

	WEBSTATIC augs::timer state_changed_timer;

	WEBSTATIC auto audiovisual_step = [&](
		const augs::audio_renderer* audio_renderer,
		const augs::delta frame_delta,
		const double speed_multiplier,
		const config_json_table& viewing_config
	) {
		const auto screen_size = logic_get_screen_size();
		const auto viewed_character = get_viewed_character();
		const auto& cosm = viewed_character.get_cosmos();
		
		//get_audiovisuals().reserve_caches_for_entities(viewed_character.get_cosmos().get_solvable().get_entity_pool().capacity());
		
		auto& interp = get_audiovisuals().get<interpolation_system>();

		{
			auto scope = measure_scope(get_audiovisuals().performance.interpolation);

			if (pending_new_state_sample) {
				bool use_current_as_previous = false; 

				on_specific_setup([&](client_setup& client) {
					if (client.is_viewing_referential()) {
						use_current_as_previous = true;
					}
				});

				interp.update_desired_transforms(cosm, use_current_as_previous);
			}

			interp.integrate_interpolated_transforms(
				viewing_config.interpolation, 
				cosm, 
				frame_delta, 
				cosm.get_fixed_delta(),
				speed_multiplier,
				get_interpolation_ratio()
			);
		}

		auto nonzoomedout_area = get_nonzoomedout_visible_world_area(viewing_config);

		visit_current_setup([&]<typename S>(const S& setup) {
			if constexpr(is_one_of_v<S, client_setup, server_setup>) {
				const auto currently_viewed = setup.get_viewed_player_nonzoomedout_visible_world_area();

				if (currently_viewed != vec2::zero) {
					nonzoomedout_area = currently_viewed;
				}
			}
		});

		gameplay_camera.tick(
			screen_size,
			nonzoomedout_area,
			interp,
			frame_delta,
			viewing_config.camera,
			viewed_character,
			calc_pre_step_crosshair_displacement(viewing_config),
			get_current_input_settings(viewing_config)
		);

		hud_messages.advance(viewing_config.hud_messages.value);

		reacquire_visible_entities(screen_size, viewed_character, viewing_config);

		const auto inv_tickrate = visit_current_setup([&](const auto& setup) {
			return setup.get_inv_tickrate();
		});

		visit_current_setup([&]<typename S>(const S& setup) {
			const auto now_sampled_cosmos = cosm.get_cosmos_id();

			auto resample = [&]() {
				audio_buffers.finish();
				audio_buffers.stop_all_sources();

				get_audiovisuals().get<sound_system>().clear();
				get_audiovisuals().get<particles_simulation_system>().clear();

				last_sampled_cosmos = now_sampled_cosmos;

				cosm.mark_as_resampled();

#if !IS_PRODUCTION_BUILD
				LOG("RESAMPLE due to a switched cosmos");
#endif
			};

			auto resample_if_different = [&]() {
				const bool requested_resample = cosm.resample_requested();

				if (requested_resample || last_sampled_cosmos != now_sampled_cosmos) {
					resample();
					gameplay_camera.dont_smooth_once = true;
				}
			};

			if constexpr(std::is_same_v<S, client_setup>) {
				const auto& referential = setup.get_arena_handle(client_arena_type::REFERENTIAL).get_cosmos();
				const auto& predicted = setup.get_arena_handle(client_arena_type::PREDICTED).get_cosmos();

				if (referential.resample_requested() || predicted.resample_requested()) {
					resample();

					referential.mark_as_resampled();
					predicted.mark_as_resampled();
				}
			}
			else {
				resample_if_different();
			}
		});

		auto audio_volume = viewing_config.audio_volume;

		if (viewing_config.audio.mute_main_menu_background) {
			if (has_main_menu() && !has_current_setup()) {
				audio_volume.sound_effects = 0.0f;
				audio_volume.music = 0.0f;
			}
		}

		get_audiovisuals().advance(audiovisual_advance_input {
			audio_buffers,
			audio_renderer,
			frame_delta,
			speed_multiplier,
			inv_tickrate,
			continuous_sounds_clock(),
			get_interpolation_ratio(),

			get_character_camera(viewing_config),
			get_queried_cone(viewing_config),
			all_visible,

			get_viewable_defs().particle_effects,
			cosm.get_logical_assets().plain_animations,

			streaming.loaded_sounds,

			audio_volume,
			viewing_config.sound,
			viewing_config.performance,

			streaming.images_in_atlas,
			get_write_buffer().particle_buffers,
			get_general_renderer().dedicated,
			pending_new_state_sample ? state_changed_timer.extract_delta() : std::optional<augs::delta>(),

			viewing_config.damage_indication,

			thread_pool
		});

		pending_new_state_sample = false;
	};

	WEBSTATIC auto setup_post_solve = [&](
		const const_logic_step step, 
		const augs::audio_renderer* audio_renderer,
		const config_json_table& viewing_config,
		const audiovisual_post_solve_settings settings
	) {
		pending_new_state_sample = true;

		{
			const auto& defs = get_viewable_defs();

			get_audiovisuals().standard_post_solve(step, { 
				audio_renderer,
				defs.particle_effects, 
				streaming.loaded_sounds,
				viewing_config.audio_volume,
				viewing_config.sound,
				get_character_camera(viewing_config),
				viewing_config.performance,
				viewing_config.damage_indication,
				settings
			});
		}

		game_gui.standard_post_solve(
			step, 
			{ settings.prediction }
		);

		if (never_predictable_v.should_play(settings.prediction)) {
			hud_messages.standard_post_solve(
				step,
				viewing_config.faction_view,
				viewing_config.hud_messages.value
			);
		}
	};

	WEBSTATIC auto setup_post_cleanup = [&](const auto& cfg, const const_logic_step step) {
		if (cfg.debug.log_solvable_hashes) {
			const auto& cosm = step.get_cosmos();
			const auto ts = cosm.get_timestamp().step;
			const auto h = cosm.calculate_solvable_signi_hash<uint32_t>();

			LOG_NVPS(ts, h);
		}
	};

	WEBSTATIC augs::timer rich_presence_timer;
	WEBSTATIC client_arena_type last_viewed_arena_type = client_arena_type::COUNT;

	WEBSTATIC auto advance_setup = [&](
		const augs::audio_renderer* audio_renderer,
		const augs::delta frame_delta,
		auto& setup,
		const input_pass_result& result
	) {
		const config_json_table& viewing_config = result.viewing_config;

		setup.control(result.motions);
		setup.control(result.intents);

		setup.accept_game_gui_events(game_gui.get_and_clear_pending_events());
		
		auto setup_audiovisual_post_solve = [&](const const_logic_step step, const audiovisual_post_solve_settings settings = {}) {
			setup_post_solve(step, audio_renderer, viewing_config, settings);
		};

		{
			using S = remove_cref<decltype(setup)>;

			auto callbacks = solver_callbacks(
				setup_pre_solve,
				setup_audiovisual_post_solve,
				[&](const const_logic_step& step) { setup_post_cleanup(viewing_config, step); }
			);

			const auto nonzoomedout_zoom = get_camera_eye(viewing_config, no_edge_zoomout_v).zoom;
			const auto input_cfg = get_current_input_settings(viewing_config);

			if constexpr(std::is_same_v<S, client_setup>) {
				/* The client needs more goodies */

				setup.advance(
					{ 
						frame_delta,
						logic_get_screen_size(), 
						input_cfg, 
						nonzoomedout_zoom,
						viewing_config.simulation_receiver, 
						viewing_config.lag_compensation, 
						network_performance,
						network_stats,
						get_audiovisuals().get<interpolation_system>(),
						get_audiovisuals().get<past_infection_system>()
					},
					callbacks
				);

				if (setup.is_replaying() && setup.is_paused()) {
					pending_new_state_sample = true;
				}

				const auto viewed = setup.get_viewed_arena_type();

				if (last_viewed_arena_type != viewed) {
					last_viewed_arena_type = viewed;
					setup.snap_interpolation_of_viewed();
				}
			}
			else if constexpr(std::is_same_v<S, server_setup>) {
#if BUILD_NETWORKING
				setup.advance(
					{ 
						logic_get_screen_size(), 
						input_cfg, 
						nonzoomedout_zoom,
						get_detected_nat(),
						network_performance,
						server_stats
					},
					callbacks
				);
#endif
			}
			else {
#if BUILD_DEBUGGER_SETUP
				if constexpr(std::is_same_v<S, debugger_setup>) {
					if (setup.is_editing_mode()) {
						pending_new_state_sample = true;
					}
				}
#endif

				if constexpr(std::is_same_v<S, editor_setup>) {
					if (!setup.is_playtesting()) {
						pending_new_state_sample = true;
					}
				}

				setup.advance(
					{ 
						frame_delta, 
						logic_get_screen_size(), 
						input_cfg, 
						nonzoomedout_zoom 
					},
					callbacks
				);
			}

#if !PLATFORM_WEB
			const auto passed_secs = rich_presence_timer.get<std::chrono::seconds>();

			if (passed_secs > 2.0 || set_rich_presence_now) {
				set_rich_presence_now = false;
				rich_presence_timer.reset();

				thread_local steam_rich_presence_pairs pairs;
				pairs.clear();

				setup.get_steam_rich_presence_pairs(pairs);

				for (auto& p : pairs) {
					if (p.second.has_value()) {
						::steam_set_rich_presence(p.first.c_str(), p.second.value().c_str());
					}
					else {
						::steam_set_rich_presence(p.first.c_str(), nullptr);
					}
				}
			}
#endif
		}

		audiovisual_step(audio_renderer, frame_delta, setup.get_audiovisual_speed(), viewing_config);
	};

	WEBSTATIC auto advance_current_setup = [&](
		const augs::audio_renderer* audio_renderer,
		const augs::delta frame_delta,
		const input_pass_result& result
	) { 
		/* 
			Advance the current setup's logic,
			and let the audiovisual_state sample the game world 
			that it chooses via get_viewed_cosmos.

			This also advances the audiovisual state, based on the cosmos returned by the setup.
		*/

		auto scope = measure_scope(game_thread_performance.advance_setup);

		visit_current_setup(
			[&](auto& setup) {
				advance_setup(audio_renderer, frame_delta, setup, result);
			}
		);
	};

	{
		auto connect_to = [&](const auto& target) {
			if (!target.empty()) {
				change_with_save([&](config_json_table& cfg) {
					cfg.client_connect = target;
				});
			}

#if PLATFORM_WEB
			/* Defer it to worker thread */
			launch_setup(activity_type::MAIN_MENU);
			client_start_requested = true;
			LOG("Requesting client start.");
#else
			launch_setup(activity_type::CLIENT);
#endif
		};

		const auto steam_cli = steam_get_launch_command_line_string();

		if (steam_cli.length() > 0) {
			LOG("Detected Steam CLI (length: %x): %x", steam_cli.length(), steam_cli);
			connect_to(steam_cli);
		}
		else {
			LOG("No Steam CLI detected.");

			if (!params.debugger_target.empty()) {
				launch_debugger(params.debugger_target);
			}
			else if (params.start_server) {
				launch_setup(activity_type::SERVER);
			}
			else if (params.should_connect) {
				connect_to(params.connect_address);
			}
			else if (params.launch_activity.has_value()) {
				LOG("Activity %x was specified from the command line.", *params.launch_activity);
				launch_setup(*params.launch_activity);

				if (params.launch_activity == activity_type::TUTORIAL) {
					on_specific_setup([&](test_scene_setup& setup) {
						if (params.tutorial_level.has_value()) {
							setup.set_tutorial_level(*params.tutorial_level);
						}
						else if (params.tutorial_challenge) {
							setup.set_tutorial_surfing_challenge();
						}
					});
				}

				if (params.launch_activity == activity_type::EDITOR_PROJECT_SELECTOR) {
					if (params.project_selector_official) {
						on_specific_setup([&](project_selector_setup& setup) {
							setup.set(project_tab_type::OFFICIAL_ARENAS);
						});
					}
				}
			}
			else {
#if PLATFORM_WEB
				if (!config.skip_tutorial) {
					launch_setup(activity_type::TUTORIAL);
				}
				else {
					launch_setup(activity_type::MAIN_MENU);
				}
#else
				if (config.launch_at_startup == launch_type::LAST_ACTIVITY) {
					if (!config.skip_tutorial) {
						launch_setup(activity_type::TUTORIAL);
					}
					else {
						launch_setup(config.get_last_activity());
					}
				}
				else {
					launch_setup(activity_type::MAIN_MENU);
				}
#endif
			}
		}
	}

	/* 
		The main loop variables.
	*/

	WEBSTATIC augs::timer frame_timer;
	
	WEBSTATIC release_flags releases;

	WEBSTATIC auto make_create_game_gui_context = [&](const config_json_table& viewing_config) {
		return [&]() {
			return game_gui.create_context(
				logic_get_screen_size(),
				common_input_state,
				get_game_gui_subject(),
				create_game_gui_deps(viewing_config)
			);
		};
	};

	WEBSTATIC auto make_create_menu_context = [&](const config_json_table& viewing_config) {
		return [&](auto& gui) {
			return gui.create_context(
				logic_get_screen_size(),
				common_input_state,
				create_menu_context_deps(viewing_config)
			);
		};
	};

	WEBSTATIC auto close_next_imgui_window = [&]() {
		ImGuiContext& g = *GImGui;

		if (g.WindowsFocusOrder.size() > 0) {
			if (g.WindowsFocusOrder.back()) {
				augs::imgui::next_window_to_close = g.WindowsFocusOrder.back()->ID; 
			}
		}
	};

	WEBSTATIC auto let_imgui_hijack_mouse = [&](auto&& create_game_gui_context, auto&& create_menu_context) {
		if (!ImGui::GetIO().WantCaptureMouse) {
			return;
		}

		/*
			Since ImGUI has quite a different philosophy about input,
			we will need some ugly inter-op with our GUIs.

			If mouse enters any IMGUI element, rewrite ImGui's mouse position to common_input_state.

			This allows us to keep common_input_state up to date, 
			because mousemotions are eaten from the vector already due to ImGui wanting mouse.
		*/

		common_input_state.mouse.pos = ImGui::GetIO().MousePos;

		/* Neutralize hovers on all GUIs whose focus may have just been stolen. */

		game_gui.world.unhover_and_undrag(create_game_gui_context());

		if (has_main_menu()) {
			main_menu_gui.world.unhover_and_undrag(create_menu_context(main_menu_gui));
		}

		ingame_menu.world.unhover_and_undrag(create_menu_context(ingame_menu));

#if BUILD_DEBUGGER_SETUP
		on_specific_setup([&](debugger_setup& setup) {
			setup.unhover();
		});
#endif

		on_specific_setup([&](editor_setup& setup) {
			setup.unhover();
		});
	};

	WEBSTATIC auto advance_game_gui = [&](const auto context, const auto frame_delta) {
		auto scope = measure_scope(game_thread_performance.advance_game_gui);

		game_gui.advance(context, frame_delta);
		game_gui.rebuild_layouts(context);
		game_gui.build_tree_data(context);
	};

	/* 
		MousePos is initially set to negative infinity.
	*/

	ImGui::GetIO().MousePos = { 0, 0 };

	WEBSTATIC cached_visibility_data cached_visibility;
	WEBSTATIC debug_details_summaries debug_summaries;

	WEBSTATIC auto game_thread_result = work_result::SUCCESS;
	WEBSTATIC bool sdk_in_gameplay_state = false;

	WEBSTATIC auto game_thread_worker = [&]() {
		auto prepare_next_game_frame = [&]() {
			auto frame = measure_scope(game_thread_performance.total);

			{
				/* The thread pool is always empty of tasks on the beginning of the game frame. */

				const auto requested_num_workers = config.performance.get_num_pool_workers();
				const auto current_num_workers = static_cast<int>(thread_pool.size());

				if (current_num_workers != requested_num_workers) {
					thread_pool.resize(requested_num_workers);
				}
			}

			/* Setup variables required by the lambdas */

			const auto screen_size = logic_get_screen_size();

			ingame_menu.root.scale = get_gui_fonts_ratio();
			main_menu_gui.root.scale = get_gui_fonts_ratio();

			const auto frame_delta = frame_timer.extract_delta();
			const auto current_frame_num = current_frame.load();
			auto game_gui_mode = game_gui_mode_flag;

			auto& write_buffer = get_write_buffer();

			auto& game_gui_renderer = write_buffer.renderers.all[renderer_type::GAME_GUI];
			auto& post_game_gui_renderer = write_buffer.renderers.all[renderer_type::POST_GAME_GUI];
			auto& debug_details_renderer = write_buffer.renderers.all[renderer_type::DEBUG_DETAILS];

			/* Logic lambdas */

			auto get_current_frame_num = [&]() {
				return current_frame_num;
			};

			auto should_quit_due_to_signal = [&]() {
#if PLATFORM_UNIX
				if (signal_status != 0) {
					const auto sig = signal_status.load();

					LOG("%x received.", strsignal(sig));

					if(
						sig == SIGINT
						|| sig == SIGSTOP
						|| sig == SIGTERM
					) {
						LOG("Gracefully shutting down.");
						request_quit();
						
						return true;
					}
				}
#endif

				return false;
			};

			auto perform_input_pass = [&]() -> input_pass_result {
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

				input_pass_result out;

				augs::local_entropy new_window_entropy;

				/* Generate release events if the previous frame so requested. */

				releases.append_releases(new_window_entropy, common_input_state);
				releases = {};

				if (ad_state == ad_state_type::NONE) {
					concatenate(new_window_entropy, write_buffer.new_window_entropy);
				}

				on_specific_setup([&](editor_setup& editor) {
					if (editor.warp_cursor_once) {
						editor.warp_cursor_once = false;

						common_input_state.mouse.pos = ImGui::GetIO().MousePos;
					}
				});

				{
					const bool sdk_in_gameplay = 
						has_current_setup()
						&& !ingame_menu.show
					;

					if (sdk_in_gameplay_state != sdk_in_gameplay) {
						sdk_in_gameplay_state = sdk_in_gameplay;

						if (sdk_in_gameplay) {
							web_sdk_gameplay_start();
						}
						else {
							web_sdk_gameplay_stop();
						}
					}
				}

				if (get_viewed_character().dead()) {
					game_gui_mode = true;
				}

				const bool in_direct_gameplay =
					!game_gui_mode
					&& has_current_setup()
					&& !ingame_menu.show
				;

				/*
					Top-level events, higher than IMGUI.
				*/
				
				{
					auto simulated_input_state = common_input_state;

					erase_if(new_window_entropy, [&](const augs::event::change e) {
						using namespace augs::event;
						using namespace augs::event::keys;

						simulated_input_state.apply(e);

						if (e.msg == message::activate || e.msg == message::click_activate) {
							if (config.content_regeneration.rescan_assets_on_window_focus) {
								streaming.request_rescan();
							}

							write_buffer.should_recheck_current_context = true;
							map_catalogue_gui.request_rescan();
						}

						if (e.msg == message::deactivate) {
							releases.set_all();
						}

						if (e.is_exit_message()) {
							LOG("Window closing due to message: %x. Shutting down.", augs::enum_to_string(e.msg));
							request_quit();
							return true;
						}
						
						{
							const bool toggle_fullscreen = 
								e.was_pressed(key::F11)
#if PLATFORM_WINDOWS
								|| (e.was_pressed(key::ENTER) && common_input_state[augs::event::keys::key::LALT])
#endif
							;

							if (toggle_fullscreen) {
								bool& f = config.window.fullscreen;
								f = !f;
								return true;
							}
						}

						if (settings_gui.should_hijack_key()) {
							if (e.was_any_key_pressed()) {
								settings_gui.set_hijacked_key(e.get_key());
								return true;
							}
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
					We "pause" the mouse cursor's position when we are in direct gameplay,
					so that when switching to GUI, the cursor appears exactly where it had disappeared.
					(it does not physically freeze the cursor, it just remembers the position)
				*/

				write_buffer.should_pause_cursor = in_direct_gameplay;

				const bool was_any_imgui_popup_opened = ImGui::IsPopupOpen(0u, ImGuiPopupFlags_AnyPopupId);

				do_imgui_pass(get_current_frame_num(), new_window_entropy, frame_delta, in_direct_gameplay);
#if !PLATFORM_WEB
				process_steam_callbacks();
#endif

				const auto viewing_config = get_setup_customized_config();
				out.viewing_config = viewing_config;

				configurables.apply(viewing_config);
				write_buffer.new_settings = viewing_config.window;
				write_buffer.browser_location = visit_current_setup([&](const auto& setup) { return setup.get_browser_location(); });
				write_buffer.swap_when = viewing_config.performance.swap_window_buffers_when;
				write_buffer.max_fps = viewing_config.window.max_fps;
				write_buffer.max_fps_method = viewing_config.window.max_fps_method;
				decide_on_cursor_clipping(in_direct_gameplay, viewing_config);

				releases.set_due_to_imgui(ImGui::GetIO());

				auto create_menu_context = make_create_menu_context(viewing_config);
				auto create_game_gui_context = make_create_game_gui_context(viewing_config);

				let_imgui_hijack_mouse(create_game_gui_context, create_menu_context);

				/*
					We also need inter-op between our own GUIs, 
					since we have more than just one.
				*/

				if (game_gui_mode && should_draw_game_gui() && game_gui.world.wants_to_capture_mouse(create_game_gui_context())) {
					if (current_setup) {
#if BUILD_DEBUGGER_SETUP
						if (auto* editor = std::get_if<debugger_setup>(&*current_setup)) {
							editor->unhover();
						}
#endif
					}
				}

				/* Maybe the game GUI was deactivated while the button was still hovered. */

				else if (!game_gui_mode && has_current_setup()) {
					game_gui.world.unhover_and_undrag(create_game_gui_context());
				}

				/* Distribution of all the remaining input happens here. */

				for (const auto& e : new_window_entropy) {
					using namespace augs::event;
					using namespace keys;
					
					/* Now is the time to actually track the input state. */
					common_input_state.apply(e);

#if PLATFORM_WEB
					bool ESC_pressed = e.was_released(key::ESC);

#if WEB_CRAZYGAMES
					on_specific_setup([&](test_scene_setup&) {
						ESC_pressed = e.was_pressed(key::TAB) || e.was_released(key::ESC);
					});
#endif
#else
					const bool ESC_pressed = e.was_pressed(key::ESC);
#endif

					if (!was_any_imgui_popup_opened && ESC_pressed) {
						if (has_current_setup()) {
							if (ingame_menu.show) {
								ingame_menu.show = false;
								browse_servers_gui.close();
							}
							else if (!visit_current_setup([&](auto& setup) {
								switch (setup.escape()) {
									case setup_escape_result::LAUNCH_INGAME_MENU: ingame_menu.show = true; return true;
									case setup_escape_result::JUST_FETCH: return true;
									case setup_escape_result::GO_TO_MAIN_MENU: launch_setup(activity_type::MAIN_MENU); return true;
									case setup_escape_result::QUIT_PLAYTESTING: restore_background_setup(); return true;
									default: return false;
								}
							})) {
								/* Setup ignored the ESC button */
								ingame_menu.show = true;
							}

							releases.set_all();
						}
						else {
							close_next_imgui_window();
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
								if (main_menu_gui.show) {
									main_menu_gui.control(create_menu_context(main_menu_gui), e, do_main_menu_option);
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

							if (const bool abandon_pressed = abandon_pending_op.has_value() && !abandon_are_you_sure_popup.has_value()) {
								on_specific_setup([&](client_setup& client) {
									client.request_abandon_ranked_match(*abandon_pending_op);
								});

								abandon_pending_op = std::nullopt;
							}
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

									const auto& general_gui_controls = viewing_config.general_gui_controls;
									const auto& game_control = viewing_config.game_controls;

									return setup.handle_input_before_game({
										general_gui_controls,
										game_control,
										streaming.necessary_images_in_atlas,
										common_input_state,
										e,
										window
									});
								}
							}

							return false;
						})) {
							on_specific_setup([](client_setup& client) {
								client.reset_afk_timer();
							});

							on_specific_setup([](server_setup& client) {
								client.reset_afk_timer();
							});

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
								if (const auto it = mapped_or_nullptr(viewing_config.general_gui_controls, key)) {
									if (was_pressed) {
										if (handle_general_gui_intent(*it)) {
											continue;
										}
									}
								}
								if (const auto it = mapped_or_nullptr(viewing_config.inventory_gui_controls, key)) {
									if (should_draw_game_gui()) {
										const auto input_cfg = get_current_input_settings(viewing_config);
										game_gui.control_hotbar_and_action_button(get_game_gui_subject(), { *it, *key_change }, input_cfg.game_gui);

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
							if (get_game_gui_subject()) {
								if (game_gui.control_gui_world(create_game_gui_context(), e)) {
									continue;
								}
							}
						}
					}
				}

				std::optional<ingame_menu_button_type> requested_op;

				on_specific_setup([&](client_setup& client) {
					requested_op = client.pending_menu_operation();
				});

				if (requested_op.has_value()) {
					do_ingame_menu_option(*requested_op, false);
				}

				/* 
					Notice that window inputs do not propagate
					beyond the closing of this scope.
				*/

				return out;
			};

			auto reload_needed_viewables = [&]() {
				visit_current_setup(
					[&](const auto& setup) {
						using T = remove_cref<decltype(setup)>;
						using S = viewables_loading_type;

						constexpr auto s = T::loading_strategy;

						if constexpr(s == S::LOAD_ALL || s == S::LOAD_ALL_ONLY_ONCE) {
							load_all(setup.get_viewable_defs());
						}
						else if constexpr(s == S::LOAD_ONLY_NEAR_CAMERA) {
							static_assert(always_false_v<T>, "Unimplemented");
						}
						else {
							static_assert(always_false_v<T>, "Unknown viewables loading strategy.");
						}
					}
				);
			};

			auto finalize_loading_viewables = [&](const bool measure_atlas_uploading) {
				streaming.finalize_load({
					audio_buffers,
					get_current_frame_num(),
					measure_atlas_uploading,
					get_general_renderer(),
					get_audiovisuals().get<sound_system>()
				});
			};

			auto create_viewing_game_gui_context = [&](augs::renderer& chosen_renderer, const config_json_table& viewing_config) {
				return viewing_game_gui_context {
					make_create_game_gui_context(viewing_config)(),

					{
						get_audiovisuals().get<interpolation_system>(),
						get_audiovisuals().world_hover_highlighter,
						viewing_config.hotbar,
						viewing_config.drawing,
						viewing_config.inventory_gui_controls,
						get_camera_eye(viewing_config),
						get_drawer_for(chosen_renderer)
					}
				};
			};

			/* View lambdas */ 

			auto setup_the_first_fbo = [&](augs::renderer& chosen_renderer) {
				chosen_renderer.set_viewport({ vec2i{0, 0}, screen_size });

				const bool rendering_flash_afterimage = gameplay_camera.is_flash_afterimage_requested();

				if (rendering_flash_afterimage) {
					necessary_fbos.flash_afterimage->set_as_current(chosen_renderer);
				}
				else {
					augs::graphics::fbo::set_current_to_none(chosen_renderer);
				}

				chosen_renderer.clear_current_fbo();
			};

			auto draw_debug_lines = [&](augs::renderer& chosen_renderer, const config_json_table& new_viewing_config) {
				if (DEBUG_DRAWING.enabled) {
					auto scope = measure_scope(game_thread_performance.debug_lines);

					const auto viewed_character = get_viewed_character();

					::draw_debug_lines(
						viewed_character.get_cosmos(),
						chosen_renderer,
						get_interpolation_ratio(),
						get_drawer_for(chosen_renderer).default_texture,
						new_viewing_config,
						get_camera_cone(new_viewing_config)
					);
				}
			};

			auto setup_standard_projection = [&](augs::renderer& chosen_renderer) {
				necessary_shaders.standard->set_projection(chosen_renderer, augs::orthographic_projection(vec2(screen_size)));
			};

			auto draw_game_gui = [&](augs::renderer& chosen_renderer, const config_json_table& viewing_config) {
				auto scope = measure_scope(game_thread_performance.draw_game_gui);

				game_gui.world.draw(create_viewing_game_gui_context(chosen_renderer, viewing_config));
			};

			auto make_draw_setup_gui_input = [&](augs::renderer& chosen_renderer, const config_json_table& new_viewing_config) {
				return draw_setup_gui_input {
					all_visible,
					get_camera_cone(new_viewing_config),
					get_blank_texture(),
					new_viewing_config,
					streaming.necessary_images_in_atlas,
					std::addressof(streaming.get_general_atlas()),
					std::addressof(streaming.avatars.texture),
					streaming.images_in_atlas,
					streaming.avatars.in_atlas,
					chosen_renderer,
					common_input_state.mouse.pos,
					screen_size,
					streaming.get_loaded_gui_fonts(),
					necessary_sounds,
					visit_current_setup([&](auto& setup) { return setup.find_player_metas(); }),
					game_gui_mode_flag,
					is_replaying_demo(),
					new_viewing_config.streamer_mode,
					new_viewing_config.streamer_mode_flags,
					visit_current_setup([&](auto& setup) { return setup.get_scoreboard_caption(); })
				};
			};

			auto draw_setup_custom_gui_over_imgui = [&](augs::renderer& chosen_renderer, const config_json_table& new_viewing_config) {
				visit_current_setup([&]<typename S>(S& setup) {
					if constexpr(std::is_same_v<editor_setup, remove_cref<S>>) {
						setup.draw_custom_gui_over_imgui(make_draw_setup_gui_input(chosen_renderer, new_viewing_config));
						chosen_renderer.call_and_clear_triangles();
					}
				});
			};

			auto draw_mode_and_setup_custom_gui = [&](augs::renderer& chosen_renderer, const config_json_table& new_viewing_config) {
				auto scope = measure_scope(game_thread_performance.draw_setup_custom_gui);

				visit_current_setup([&](auto& setup) {
					setup.draw_custom_gui(make_draw_setup_gui_input(chosen_renderer, new_viewing_config));
					chosen_renderer.call_and_clear_lines();
				});

				if (new_viewing_config.hud_messages.is_enabled) {
					hud_messages.draw(
						chosen_renderer,
						get_blank_texture(),
						streaming.get_loaded_gui_fonts().gui,
						screen_size,
						new_viewing_config.hud_messages.value,
						frame_delta
					);
				}
			};

			auto fallback_overlay_color = [&](augs::renderer& chosen_renderer, const rgba col = darkgray) {
				streaming.get_general_atlas().set_as_current(chosen_renderer);

				necessary_shaders.standard->set_as_current(chosen_renderer);
				setup_standard_projection(chosen_renderer);

				get_drawer_for(chosen_renderer).color_overlay(screen_size, col);
			};

			auto draw_and_choose_menu_cursor = [&](augs::renderer& chosen_renderer, auto&& create_menu_context) {
				auto scope = measure_scope(game_thread_performance.menu_gui);

				auto get_drawer = [&]() {
					return get_drawer_for(chosen_renderer);
				};

				if (has_current_setup()) {
					if (ingame_menu.show) {
						const auto context = create_menu_context(ingame_menu);
						ingame_menu.advance(context, frame_delta);

						return ingame_menu.draw({ context, get_drawer() });
					}

					return assets::necessary_image_id::INVALID;
				}
				else {
					const auto context = create_menu_context(main_menu_gui);

					main_menu_gui.advance(context, frame_delta);

					menu_ltrb = main_menu_gui.root.get_menu_ltrb(context);

#if MENU_ART
					get_drawer().aabb(streaming.necessary_images_in_atlas[assets::necessary_image_id::ART_1], ltrb(0, 0, screen_size.x, screen_size.y), white);
#endif

					const auto cursor = main_menu_gui.draw({ context, get_drawer() });

					main_menu->draw_overlays(
						last_update_result,
						get_drawer(),
						streaming.necessary_images_in_atlas,
						streaming.get_loaded_gui_fonts().gui,
						screen_size
					);

					return cursor;
				}
			};

			auto draw_non_menu_cursor = [&](augs::renderer& chosen_renderer, const config_json_table& viewing_config, const assets::necessary_image_id menu_chosen_cursor) {
				const bool should_draw_our_cursor = viewing_config.window.draws_own_cursor() && !window.is_mouse_pos_paused();
				const auto cursor_drawing_pos = common_input_state.mouse.pos;

				auto get_drawer = [&]() {
					return get_drawer_for(chosen_renderer);
				};

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
					if (get_viewed_character()) {
						const auto& character_gui = game_gui.get_character_gui(get_game_gui_subject());

						character_gui.draw_cursor_with_tooltip(create_viewing_game_gui_context(chosen_renderer, viewing_config), should_draw_our_cursor);
					}
				}
				else {
					if (should_draw_our_cursor) {
						get_drawer().cursor(streaming.necessary_images_in_atlas, assets::necessary_image_id::GUI_CURSOR, cursor_drawing_pos, white);
					}
				}
			};

			auto make_illuminated_rendering_input = [&](augs::renderer& chosen_renderer, const config_json_table& viewing_config) {
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

				const auto viewed_character = get_viewed_character();

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

				auto cone = get_camera_cone(viewing_config);
				cone.eye.transform.pos.discard_fract();

				if (screen_size.x % 2 == 1) {
					cone.eye.transform.pos.x -= 0.5f;
				}

				if (screen_size.y % 2 == 1) {
					cone.eye.transform.pos.y -= 0.5f;
				}

				return illuminated_rendering_input {
					{ viewed_character, cone },
					get_camera_requested_fov_expansion(),
					get_camera_edge_zoomout_mult(),
					get_queried_cone(viewing_config),
					calc_pre_step_crosshair_displacement(viewing_config),
					get_audiovisuals(),
					viewing_config.drawing,
					viewer_is_spectator(),
					streaming.necessary_images_in_atlas,
					streaming.get_loaded_gui_fonts(),
					streaming.images_in_atlas,
					get_interpolation_ratio(),
					chosen_renderer,
					game_thread_performance,
					!has_current_setup() ? std::addressof(streaming.get_general_or_blank()) : std::addressof(streaming.get_general_atlas()),
					necessary_fbos,
					necessary_shaders,
					all_visible,
					viewing_config.performance,
					viewing_config.renderer,
					highlights,
					special_indicators,
					indicator_meta,
					write_buffer.particle_buffers,
					viewing_config.damage_indication,
					cached_visibility.light_requests,
					viewing_config.streamer_mode && viewing_config.streamer_mode_flags.inworld_hud,
					thread_pool
				};
			};

			auto perform_illuminated_rendering = [&](const illuminated_rendering_input& input) {
				auto scope = measure_scope(game_thread_performance.rendering_script);

				illuminated_rendering(input);
			};

			auto draw_call_imgui = [&](augs::renderer& chosen_renderer) {
				chosen_renderer.call_and_clear_triangles();

				auto scope = measure_scope(game_thread_performance.imgui);

				chosen_renderer.draw_call_imgui(
					imgui_atlas, 
					std::addressof(streaming.get_general_atlas()), 
					std::addressof(streaming.avatars.texture), 
					std::addressof(streaming.avatar_preview_tex),
					std::addressof(streaming.ad_hoc.texture)
				);
			};

			auto do_flash_afterimage = [&](augs::renderer& chosen_renderer) {
				const auto flash_mult = gameplay_camera.get_effective_flash_mult();
				const bool rendering_flash_afterimage = gameplay_camera.is_flash_afterimage_requested();

				const auto viewed_character = get_viewed_character();

				auto get_drawer = [&]() {
					return get_drawer_for(chosen_renderer);
				};

				::handle_flash_afterimage(
					chosen_renderer,
					necessary_shaders,
					necessary_fbos,
					streaming.get_general_atlas(),
					viewed_character,
					get_drawer,
					flash_mult,
					rendering_flash_afterimage,
					screen_size
				);
			};

			auto show_performance_details = [&](augs::renderer& chosen_renderer) {
				auto scope = measure_scope(game_thread_performance.debug_details);

				const auto viewed_character = get_viewed_character();

				debug_summaries.acquire(
					viewed_character.get_cosmos(),
					game_thread_performance,
					network_performance,
					network_stats,
					server_stats,
					streaming.performance,
					streaming.general_atlas_performance,
					render_thread_performance,
					get_audiovisuals().performance
				);

				::show_performance_details(
					get_drawer_for(chosen_renderer),
					streaming.get_loaded_gui_fonts().gui,
					screen_size,
					viewed_character,
					debug_summaries
				);
			};

			auto show_recent_logs = [&](augs::renderer& chosen_renderer) {
				::show_recent_logs(
					get_drawer_for(chosen_renderer),
					streaming.get_loaded_gui_fonts().gui,
					screen_size
				);
			};

			/* Flow */

			if (should_quit_due_to_signal()) {
				return;
			}

			ensure_float_flags_hold();

			if (setup_requires_cursor()) {
				game_gui_mode = true;
			}

			reload_needed_viewables();
			finalize_loading_viewables(config.debug.measure_atlas_uploading);

			const auto input_result = perform_input_pass();
			const auto& new_viewing_config = input_result.viewing_config;

			auto audio_renderer = std::optional<augs::audio_renderer>();

			if (const auto audio_buffer = audio_buffers.map_write_buffer()) {
				audio_renderer.emplace(augs::audio_renderer { 
					audio_buffers.num_currently_processed_buffers(),
					*audio_buffer 
				});
			}

			advance_current_setup(
				audio_renderer ? std::addressof(audio_renderer.value()) : nullptr,
				frame_delta,
				input_result
			);

			auto create_menu_context = make_create_menu_context(new_viewing_config);
			auto create_game_gui_context = make_create_game_gui_context(new_viewing_config);

			/* 
				What follows is strictly view part,
				without advancement of any kind.
				
				No state is altered beyond this point,
				except for usage of graphical resources and profilers.
			*/

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

			setup_the_first_fbo(get_general_renderer());

			const auto viewed_character = get_viewed_character();
			const auto& viewed_cosmos = viewed_character.get_cosmos();
			const bool non_zero_cosmos = !viewed_cosmos.completely_unset();

			auto enqueue_visibility_jobs = [&]() {
				const auto& interp = get_audiovisuals().get<interpolation_system>();
				auto& light_requests = cached_visibility.light_requests;
				light_requests.clear();

				::for_each_light_vis_request(
					[&](const visibility_request& request) {
						light_requests.emplace_back(request);
					},

					viewed_cosmos,
					all_visible,

					get_audiovisuals().get<light_system>().per_entity_cache,
					interp,
					get_queried_cone(new_viewing_config).get_visible_world_rect_aabb(),
					std::addressof(get_audiovisuals().get<particles_simulation_system>())
				);

				auto fog_of_war = new_viewing_config.drawing.fog_of_war;
				fog_of_war.size *= get_camera_requested_fov_expansion();

				const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();

#if BUILD_STENCIL_BUFFER
				const bool fog_of_war_effective = 
					viewed_character_transform.has_value() 
					&& fog_of_war.is_enabled()
				;
#else
				const bool fog_of_war_effective = false;
#endif

				::enqueue_visibility_jobs(
					thread_pool,

					viewed_cosmos,
					get_general_renderer().dedicated,
					cached_visibility,

					fog_of_war_effective,
					viewed_character,
					viewed_character_transform ? *viewed_character_transform : transformr(),
					fog_of_war
				);
			};

			const auto illuminated_input = make_illuminated_rendering_input(get_general_renderer(), new_viewing_config);

			auto illuminated_rendering_job = [&]() {
				/* #1 */
				perform_illuminated_rendering(illuminated_input);

				// Call this in case we don't call perform_illuminated_rendering
				// necessary_shaders.standard->set_as_current(get_general_renderer());

				/* #2 */
				draw_debug_lines(get_general_renderer(), new_viewing_config);

				setup_standard_projection(get_general_renderer());
			};

			auto game_gui_job = [&]() {
				advance_game_gui(create_game_gui_context(), frame_delta);
				draw_game_gui(game_gui_renderer, new_viewing_config);
			};

			auto menu_chosen_cursor = assets::necessary_image_id::INVALID;

			auto post_game_gui_job = [&]() {
				auto& chosen_renderer = post_game_gui_renderer;

				if (non_zero_cosmos) {
					/* #4 */
					draw_mode_and_setup_custom_gui(chosen_renderer, new_viewing_config);
				}
				else {
					fallback_overlay_color(chosen_renderer);
				}

				/* #5 */
				menu_chosen_cursor = draw_and_choose_menu_cursor(chosen_renderer, create_menu_context);

				/* #6 */
				draw_call_imgui(chosen_renderer);

				draw_setup_custom_gui_over_imgui(chosen_renderer, new_viewing_config);
			};

			auto place_final_drawcalls_synchronously = [&]() {
				auto& chosen_renderer = post_game_gui_renderer;

				/* #7 */
				draw_non_menu_cursor(chosen_renderer, new_viewing_config, menu_chosen_cursor);

				do_flash_afterimage(chosen_renderer);
			};

			auto show_performance_details_job = [&]() {
				show_performance_details(debug_details_renderer);
			};

			auto show_recent_logs_job = [&]() {
				show_recent_logs(debug_details_renderer);
			};

			enqueue_visibility_jobs();

			if (new_viewing_config.session.show_performance) {
				thread_pool.enqueue(show_performance_details_job);
			}

			if (new_viewing_config.session.show_logs && !config.streamer_mode) {
				thread_pool.enqueue(show_recent_logs_job);
			}

			if (non_zero_cosmos) {
				thread_pool.enqueue(illuminated_rendering_job);

				/* #3 */
				if (should_draw_game_gui()) {
					thread_pool.enqueue(game_gui_job);
				}

				::enqueue_illuminated_rendering_jobs(thread_pool, illuminated_input);
			}

#if WEB_SINGLETHREAD
			/* Prevent UI flashing while loading */
			const bool now_loading = setup_just_launched > 0;

			if (now_loading) {
				--setup_just_launched;

				auto overlay_col = rgba(0, 8, 5, 220);

				on_specific_setup([&](main_menu_setup&) {
					overlay_col = rgba(8, 21, 4, 255);
					setup_just_launched = 0;
				});

				fallback_overlay_color(post_game_gui_renderer, overlay_col);

				using namespace augs::gui::text;

				print_stroked(
					get_drawer_for(post_game_gui_renderer),
					screen_size / 2,
					formatted_string("Loading...", style(streaming.get_loaded_gui_fonts().medium_numbers, white)),
					{ augs::ralign::CX , augs::ralign::CY },
					black
				);
			}
			else {
				if (ad_state != ad_state_type::NONE) {
					auto overlay_col = rgba(0, 8, 5, 200);

					fallback_overlay_color(post_game_gui_renderer, overlay_col);
				}
				else {
					thread_pool.enqueue(post_game_gui_job);
				}
			}
#else
			thread_pool.enqueue(post_game_gui_job);
#endif

			thread_pool.submit();

			auto scope = measure_scope(game_thread_performance.help_tasks);

			thread_pool.help_until_no_tasks();
			thread_pool.wait_for_all_tasks_to_complete();

			/* 
				This task is dependent upon completion of two other tasks: 
				- game_gui_job
				- post_game_gui_job
			*/

			place_final_drawcalls_synchronously();
		};

#if WEB_SINGLETHREAD
#else
		while (!should_quit) 
#endif
		{
			auto extract_num_total_drawn_triangles = [&]() {
				return get_write_buffer().renderers.extract_num_total_triangles_drawn();
			};

			auto finalize_frame_and_swap = [&]() {
				for (auto& r : get_write_buffer().renderers.all) {
					r.call_and_clear_triangles();
				}

				visit_current_setup([&](auto& s) {
					return s.after_all_drawcalls(get_write_buffer());
				});

				game_thread_performance.num_triangles.measure(extract_num_total_drawn_triangles());

#if WEB_SINGLETHREAD
#else
				{
					/*
						On the web, it is the wait_swap that correctly reports the effective FPS,
						because even though render_thread_performance.fps might report north of 1000 FPS,
						the main loop is simply called infrequently.
					*/

					auto scope = measure_scope(game_thread_performance.wait_swap);
					buffer_swapper.wait_swap();
				}

				get_write_buffer().clear_frame();
#endif
			};

			{
				auto scope = measure_scope(game_thread_performance.total);

#if IS_PRODUCTION_BUILD
				try {
					prepare_next_game_frame();
				}
				catch (const entity_creation_error& err) {
					LOG("Failed to create entity: %x", err.what());
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
				catch (const std::ios_base::failure& err) {
					LOG("std::ios_base::failure: %x", err.what());
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
				catch (const std::runtime_error& err) {
					LOG("Runtime error: %x", err.what());
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
				catch (const std::logic_error& err) {
					LOG("Logic error: %x", err.what());
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
				catch (const std::exception& err) {
					LOG("Exception: %x", err.what());
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
				catch (...) {
					LOG("Unknown error.");
					game_thread_result = work_result::FAILURE;
					request_quit();
				}
#else
				prepare_next_game_frame();
#endif
			}

			finalize_frame_and_swap();
		}
	};

#if WEB_SINGLETHREAD
#else
	LOG("Starting game_thread_worker");
	WEBSTATIC auto game_thread = std::thread(game_thread_worker);
#endif

	WEBSTATIC auto audio_thread_joiner = augs::scope_guard([&]() { LOG("audio_thread_joiner"); audio_buffers.quit(); });

#if WEB_SINGLETHREAD
#else
	WEBSTATIC auto game_thread_joiner = augs::scope_guard([&]() { LOG("game_thread_joiner"); game_thread.join(); });
#endif

	request_quit = [&]() {
		get_write_buffer().should_quit = true;
		should_quit = true;
	};

	WEBSTATIC renderer_backend_result rendering_result;

	WEBSTATIC auto game_main_thread_synced_op = [&]() {
		auto scope = measure_scope(game_thread_performance.synced_op);

		/* 
			IMGUI is our top GUI whose priority precedes everything else. 
			It will eat from the window input vector that is later passed to the game and other GUIs.	
		*/
	
		window.handle_pending_requests();
		configurables.apply_main_thread(get_read_buffer().new_settings);

		/*
			Update fonts only after the new requested size
			doesn't change for 0.2 or more.

			This is to accomodate a gradually changing window size.
		*/

		{
			auto new_ratio = get_gui_fonts_ratio();

			if (new_ratio != last_gui_fonts_ratio) {
				when_last_gui_fonts_ratio_changed.reset();
				last_gui_fonts_ratio = new_ratio;
			}

			if (loaded_gui_fonts_ratio != new_ratio) {
				if (when_last_gui_fonts_ratio_changed.get<std::chrono::seconds>() > 0.2) {
					LOG_NVPS(loaded_gui_fonts_ratio, new_ratio);
					loaded_gui_fonts_ratio = new_ratio;

					imgui_atlas = augs::imgui::create_atlas(config.gui_fonts.gui, new_ratio);
				}
			}
		}

		if (config.session.show_performance) {
			const auto viewed_character = get_viewed_character();

			viewed_character.get_cosmos().profiler.prepare_summary_info();
			game_thread_performance.prepare_summary_info();
			network_performance.prepare_summary_info();
			streaming.performance.prepare_summary_info();

			if (streaming.finished_generating_atlas()) {
				streaming.general_atlas_performance.prepare_summary_info();
			}

			render_thread_performance.prepare_summary_info();
			get_audiovisuals().performance.prepare_summary_info();
		}

		for (const auto& f : rendering_result.imgui_lists_to_delete) {
			IM_DELETE(f);
		}

		visit_current_setup([&](auto& s) {
			s.do_game_main_thread_synced_op(rendering_result);
		});
	};

	WEBSTATIC augs::timer this_frame_timer;

	struct main_loop_input {
		augs::window& window;
		game_frame_buffer_swapper& buffer_swapper;
		augs::graphics::renderer_backend& renderer_backend;
		augs::thread_pool& thread_pool;
		augs::timer& this_frame_timer;
		augs::timer& until_first_swap;
		session_profiler& render_thread_performance;
		bool& until_first_swap_measured;
		renderer_backend_result& rendering_result;
		std::atomic<augs::frame_num_type>& current_frame;

		decltype(get_read_buffer)& get_read_buffer_fun;
		decltype(game_main_thread_synced_op)& game_main_thread_synced_op_fun;
	};

	WEBSTATIC auto main_loop_in = main_loop_input {
		window,
		buffer_swapper,
		renderer_backend,
		thread_pool,
		this_frame_timer,
		until_first_swap,
		render_thread_performance,
		until_first_swap_measured,
		rendering_result,
		current_frame,

		get_read_buffer,
		game_main_thread_synced_op
	};

	WEBSTATIC auto main_loop_in_ptr = reinterpret_cast<void*>(&main_loop_in);

#if PLATFORM_WEB
	window.swap_buffers();

	WEBSTATIC bool swapped_once = false;
	WEBSTATIC std::string current_browser_location = "/";

	if (params.is_crazygames) {
		current_browser_location = "";
	}
#endif

	static auto main_loop_iter = [](void* arg) -> bool {
		auto& mi = *reinterpret_cast<main_loop_input*>(arg);

		auto& buffer_swapper = mi.buffer_swapper;
		auto& game_main_thread_synced_op = mi.game_main_thread_synced_op_fun;
		auto& thread_pool = mi.thread_pool;
		auto& renderer_backend = mi.renderer_backend;
		auto& window = mi.window;

		auto& this_frame_timer = mi.this_frame_timer;
		(void)this_frame_timer;
		auto& until_first_swap = mi.until_first_swap;
		auto& render_thread_performance = mi.render_thread_performance;
		auto& until_first_swap_measured = mi.until_first_swap_measured;
		auto& rendering_result = mi.rendering_result;
		auto& current_frame = mi.current_frame;

		auto& get_read_buffer = mi.get_read_buffer_fun;

		auto collect_window_entropy = [&]() {
			auto& read_buffer = get_read_buffer();

			auto scope = measure_scope(render_thread_performance.local_entropy);

			auto& next_entropy = read_buffer.new_window_entropy;
			next_entropy.clear();

			window.collect_entropy(next_entropy);

			read_buffer.screen_size = window.get_screen_size();
		};

		auto swap_window_buffers = [&]() {
			auto scope = measure_scope(render_thread_performance.swap_window_buffers);

#if PLATFORM_WEB
			if (!swapped_once) {
				call_hideProgress();
				swapped_once = true;
			}
#endif

			window.swap_buffers();

			if (!until_first_swap_measured) {
				LOG("Time until first swap: %x ms", until_first_swap.extract<std::chrono::milliseconds>());
				until_first_swap_measured = true;

				program_log::get_current().mark_last_init_log();
			}
		};


#if PLATFORM_WEB
		(void)thread_pool;

		main_thread_queue::process_tasks();

		thread_pool.help_until_no_tasks();

		if (!buffer_swapper.can_swap_buffers()) {
			return true;
		}

		auto scope = measure_scope(render_thread_performance.fps);

		collect_window_entropy();

#if WEB_SINGLETHREAD
		game_thread_worker();
#endif

		buffer_swapper.swap_buffers(game_main_thread_synced_op);

		{
			std::string url_to_open;

			{
				auto lock = augs::scoped_lock(open_url_on_main_lk);

				if (!open_url_on_main.empty()) {
					url_to_open = open_url_on_main;
					open_url_on_main.clear();
				}
			}

			if (!url_to_open.empty()) {
				call_openUrl(url_to_open.c_str());
			}
		}

		{
			const auto& read_buffer = get_read_buffer();

			const auto location = [&]() {
				if (params.is_crazygames) {
					auto location = read_buffer.browser_location;

					if (begins_with(location, "game/")) {
						cut_preffix(location, "game/");
					}
					else {
						location = "";
					}

					return location;
				}
				else {
					return "/" + read_buffer.browser_location;
				}
			}();

			if (current_browser_location != location) {
				current_browser_location = location;

				LOG("Setting browser location to: \"%x\"", current_browser_location);

				call_setBrowserLocation(current_browser_location.c_str());
			}
		}

#else
		auto scope = measure_scope(render_thread_performance.fps);
#endif

		{
			auto& read_buffer = get_read_buffer();

			if (read_buffer.should_recheck_current_context) {
				augs::window::get_current().check_current_context();

				read_buffer.should_recheck_current_context = false;
			}

			const auto swap_when = read_buffer.swap_when;

			if (swap_when == swap_buffers_moment::AFTER_HELPING_LOGIC_THREAD) {
				swap_window_buffers();
			}

			{
				auto scope = measure_scope(render_thread_performance.renderer_commands);

				rendering_result.clear();

				for (auto& r : read_buffer.renderers.all) {
					renderer_backend.perform(
						rendering_result,
						r.commands.data(),
						r.commands.size(),
						r.dedicated
					);
				}

				read_buffer.clear_frame();

				current_frame.fetch_add(1, std::memory_order_relaxed);
			}

			if (swap_when == swap_buffers_moment::AFTER_GL_COMMANDS) {
				swap_window_buffers();
			}
		}

#if !PLATFORM_WEB
		collect_window_entropy();

		{
			auto scope = measure_scope(render_thread_performance.render_help);
			thread_pool.help_until_no_tasks();
		}

		{
			auto scope = measure_scope(render_thread_performance.render_wait);
			buffer_swapper.swap_buffers(game_main_thread_synced_op);

			const auto max_fps = get_read_buffer().max_fps;

			if (max_fps.is_enabled && max_fps.value >= 10) {
				const auto target_delay_ms = 1000.0 / max_fps.value;
				const auto method = get_read_buffer().max_fps_method;

				auto passed_ms = this_frame_timer.get<std::chrono::milliseconds>();

				while (passed_ms < target_delay_ms) {
					switch (method) {
						case augs::max_fps_type::SLEEP:
						{
							const auto to_sleep_ms = target_delay_ms - passed_ms;

							std::this_thread::sleep_for(std::chrono::duration<double>(to_sleep_ms / 1000.0));
							break;
						}
						case augs::max_fps_type::SLEEP_ZERO:
							augs::sleep(0);
							break;
						case augs::max_fps_type::YIELD:
							std::this_thread::yield();
							break;
						case augs::max_fps_type::BUSY:
							break;
						default:
							break;
					}

					passed_ms = this_frame_timer.get<std::chrono::milliseconds>();
				}

				this_frame_timer.reset();
			}
		}
#endif

		auto& read_buffer = get_read_buffer();

		if (window.is_active() && read_buffer.should_clip_cursor) {
			window.set_cursor_clipping(true);
			window.set_cursor_visible(false);
		}
		else {
			window.set_cursor_clipping(false);
			window.set_cursor_visible(true);
		}

		window.set_mouse_pos_paused(read_buffer.should_pause_cursor);

		if (read_buffer.should_quit) {
			return false;
		}

		return true;
	};

#if PLATFORM_WEB
	static auto main_loop_iter_em = [](void* arg) {
		if (!main_loop_iter(arg)) {
			emscripten_cancel_main_loop();
		}
	};

	LOG("Calling emscripten_set_main_loop_arg.");

	web_sdk_loading_stop();

#if WEB_SINGLETHREAD
	emscripten_set_main_loop_arg(main_loop_iter_em, main_loop_in_ptr, 0, 1);
#else
	emscripten_set_main_loop_arg(main_loop_iter_em, main_loop_in_ptr, 0, 0);
#endif

#else
	for (;;) {
		if (!main_loop_iter(main_loop_in_ptr)) {
			break;
		}
	}
#endif

	return game_thread_result;
#endif
}
catch (const config_read_error& err) {
	LOG("Failed to read the initial config for the game!\n%x", err.what());
	return work_result::FAILURE;
}
#if HEADLESS
#else
catch (const augs::imgui_init_error& err) {
	LOG("Failed init imgui:\n%x", err.what());
	return work_result::FAILURE;
}
catch (const augs::audio_error& err) {
	LOG("Failed to establish the audio context:\n%x", err.what());
	return work_result::FAILURE;
}
catch (const augs::window_error& err) {
	LOG("Failed to create an OpenGL window:\n%x", err.what());
	return work_result::FAILURE;
}
catch (const augs::graphics::renderer_error& err) {
	LOG("Failed to initialize the renderer: %x", err.what());
	return work_result::FAILURE;
}
catch (const necessary_resource_loading_error& err) {
	LOG("Failed to load a resource necessary for the game to function!\n%x", err.what());
	return work_result::FAILURE;
}
#endif
catch (const augs::unit_test_session_error& err) {
	LOG("Unit test session failure:\n%x\ncout:%x\ncerr:%x\nclog:%x\n", 
		err.what(), err.cout_content, err.cerr_content, err.clog_content
	);

	return work_result::FAILURE;
}
catch (const netcode_socket_raii_error& err) {
	LOG("Failed to create a socket for server browser:\n%x", err.what());
	return work_result::FAILURE;
}