#pragma once
#include "augs/filesystem/path.h"
#include "augs/app_type.h"
#include "augs/network/network_types.h"
#include "application/activity_type.h"
#include "augs/string/typesafe_sscanf.h"

struct cmd_line_params {
	std::string complete_command_line;
	std::string parsed_as;

	std::string live_log_path;

	std::optional<augs::path_type> calling_cwd;
	augs::path_type appdata_dir;

	augs::path_type exe_path;
	augs::path_type appimage_path;
	augs::path_type debugger_target;
	augs::path_type editor_target;
	augs::path_type consistency_report;
	bool unit_tests_only = false;
	bool help_only = false;
	bool version_only = false;
	bool version_line_only = false;
	bool start_server = false;
#if HEADLESS
	app_type type = app_type::DEDICATED_SERVER;
#else
	app_type type = app_type::GAME_CLIENT;
#endif
	bool upgraded_successfully = false;
	bool should_connect = false;
	bool only_check_update_availability_and_quit = false;
	bool sync_external_arenas = false;
	bool sync_external_arenas_and_quit = false;
	std::optional<int> autoupdate_delay;

	augs::path_type apply_config;
	augs::path_type assign_teams;

	int test_fp_consistency = -1;
	std::string connect_address;

	bool no_router = false;
	bool as_service = false;

	bool suppress_server_webhook = false;

	bool no_update_on_launch = false;
	bool update_once_now = false;
	bool daily_autoupdate = false;

	std::optional<port_type> first_udp_command_port;
	std::optional<port_type> server_list_port;
	std::optional<port_type> server_port;

	bool is_updater = false;
	augs::path_type verified_archive;
	augs::path_type verified_signature;

	std::optional<activity_type> launch_activity;
	std::optional<int> tutorial_level;
	bool tutorial_challenge = false;

	std::string guest;

	void parse(const int argc, const char* const * const argv, const int start_i) {
		for (int i = start_i; i < argc; ++i) {
			const auto a = std::string(argv[i]);

			complete_command_line += " " + a;
			parsed_as += typesafe_sprintf("%x=%x ", i, a);
		}

		for (int i = start_i; i < argc; ++i) {
			auto get_next = [&i, argc, argv]() {
				if (i + 1 < argc) {
					return argv[++i];
				}

				return "";
			};

			const auto a = std::string(argv[i]);

			if (begins_with(a, "-psn")) {
				/* MacOS process identifier. Ignore. */
				continue;
			}
			else if (a.size() > 1 && a[0] == '/') {
				/* URL location argument. */

				std::string loc;
				std::string query;

				if (!typesafe_sscanf(a, "%x?%x", loc, query)) {
					loc = a;
				}

				if (begins_with(loc, "/game")) {
					std::string webrtc_id;

					if (1 == typesafe_sscanf(loc, "/game/%x", webrtc_id)) {
						should_connect = true;
						connect_address = webrtc_id;
					}
				}
				else if (begins_with(loc, "/host")) {
					start_server = true;
				}
				else if (begins_with(loc, "/tutorial")) {
					launch_activity = activity_type::TUTORIAL;

					uint32_t level = 0;
					std::string keyword;

					if (1 == typesafe_sscanf(loc, "/tutorial/%x", level)) {
						tutorial_level = level;
					}
					else if (1 == typesafe_sscanf(loc, "/tutorial/%x", keyword)) {
						if (keyword == "challenge") {
							tutorial_challenge = true;
						}
					}
				}
				else if (begins_with(loc, "/menu")) {
					launch_activity = activity_type::MAIN_MENU;
				}
				else if (begins_with(loc, "/editor")) {
					launch_activity = activity_type::EDITOR_PROJECT_SELECTOR;
				}
				else if (begins_with(loc, "/range")) {
					launch_activity = activity_type::SHOOTING_RANGE;
				}

				if (!query.empty()) {
					typesafe_sscanf(query, "guest=%x", guest);
				}

				LOG_NVPS(a, loc, query, guest);
			}
			else if (a == "--appimage-path") {
				appimage_path = get_next();
				exe_path = appimage_path;
			}
			else if (a == "--unit-tests-only") {
				unit_tests_only = true;
			}
			else if (a == "--help" || a == "-h") {
				help_only = true;
			}
			else if (a == "--version-line") {
				version_line_only = true;
			}
			else if (a == "--version" || a == "-v") {
				version_only = true;
			}
			else if (a == "--is-update-available") {
				only_check_update_availability_and_quit = true;
			}
			else if (a == "--as-service") {
				as_service = true;
			}
			else if (a == "--server") {
				start_server = true;
			}
			else if (a == "--upgraded-successfully") {
				upgraded_successfully = true;
			}
			else if (a == "--dedicated-server") {
				type = app_type::DEDICATED_SERVER;
			}
			else if (a == "--delayed-autoupdate") {
				daily_autoupdate = true;
				autoupdate_delay = std::atoi(get_next());
			}
			else if (a == "--server-port") {
				server_port = std::atoi(get_next());
			}
			else if (a == "--no-router") {
				no_router = true;
			}
			else if (a == "--no-update-on-launch") {
				no_update_on_launch = true;
			}
			else if (a == "--daily-autoupdate") {
				daily_autoupdate = true;
			}
			else if (a == "--update-once-now") {
				update_once_now = true;
			}
			else if (a == "--suppress-server-webhook") {
				suppress_server_webhook = true;
			}
			else if (a == "--masterserver") {
				type = app_type::MASTERSERVER;
			}
			else if (a == "--test-fp-consistency") {
				test_fp_consistency = std::atoi(get_next());
			}
			else if (a == "--nat-punch-port") {
				first_udp_command_port = std::atoi(get_next());
			}
			else if (a == "--server-list-port") {
				server_list_port = std::atoi(get_next());
			}
			else if (a == "--consistency-report") {
				consistency_report = get_next();
			}
			else if (a == "--verify-updater") {
				is_updater = true;
				verified_archive = get_next();
			}
			else if (a == "--verify") {
				verified_archive = get_next();
			}
			else if (a == "--apply-config") {
				apply_config = get_next();
			}
			else if (a == "--assign-teams") {
				assign_teams = get_next();
			}
			else if (a == "--edit") {
				editor_target = get_next();
			}
			else if (a == "--signature") {
				verified_signature = get_next();
			}
			else if (a == "--connect") {
				should_connect = true;
				connect_address = get_next();
			}
			else if (a == "--live-log") {
				live_log_path = get_next();
			}
			else if (a == "--sync-external-arenas") {
				sync_external_arenas = true;
			}
			else if (a == "--sync-external-arenas-and-quit") {
				sync_external_arenas_and_quit = true;
			}
			else if (a == "--appdata-dir") {
				appdata_dir = get_next();
			}
			else if (a == "--calling-cwd") {
				calling_cwd = get_next();
			}
			else {

			}
		}
	}

	cmd_line_params(const int argc, const char* const * const argv) {
		exe_path = argv[0];
		complete_command_line = exe_path.string();

		parse(argc, argv, 1);
	}

	bool is_cli_tool() const {
		return type == app_type::MASTERSERVER || type == app_type::DEDICATED_SERVER;
	}
};