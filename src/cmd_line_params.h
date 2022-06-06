#pragma once
#include "augs/filesystem/path.h"
#include "augs/app_type.h"
#include "augs/network/network_types.h"

struct cmd_line_params {
	augs::path_type exe_path;
	augs::path_type debugger_target;
	augs::path_type consistency_report;
	bool unit_tests_only = false;
	bool help_only = false;
	bool version_only = false;
	bool start_server = false;
	app_type type = app_type::GAME_CLIENT;
	bool upgraded_successfully = false;
	bool should_connect = false;
	bool keep_cwd = false;
	int test_fp_consistency = -1;
	std::string connect_address;

	bool disallow_nat_traversal = false;

	std::optional<port_type> first_udp_command_port;
	std::optional<port_type> server_list_port;

	bool is_updater = false;
	augs::path_type verified_archive;
	augs::path_type verified_signature;

	cmd_line_params(const int argc, const char* const * const argv) {
		exe_path = argv[0];

		for (int i = 1; i < argc;) {
			const auto a = std::string(argv[i++]);

			if (begins_with(a, "-psn")) {
				continue;
			}
			else if (a == "--unit-tests-only") {
				unit_tests_only = true;
				keep_cwd = true;
			}
			else if (a == "--help" || a == "-h") {
				help_only = true;
			}
			else if (a == "--version" || a == "-v") {
				version_only = true;
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
			else if (a == "--disallow-nat-traversal") {
				disallow_nat_traversal = true;
			}
			else if (a == "--masterserver") {
				type = app_type::MASTERSERVER;
			}
			else if (a == "--test-fp-consistency") {
				test_fp_consistency = std::atoi(argv[i++]);
				keep_cwd = true;
			}
			else if (a == "--nat-punch-port") {
				first_udp_command_port = std::atoi(argv[i++]);
			}
			else if (a == "--server-list-port") {
				server_list_port = std::atoi(argv[i++]);
			}
			else if (a == "--consistency-report") {
				consistency_report = argv[i++];
			}
			else if (a == "--verify-updater") {
				is_updater = true;
				verified_archive = argv[i++];
			}
			else if (a == "--verify") {
				verified_archive = argv[i++];
			}
			else if (a == "--signature") {
				verified_signature = argv[i++];
			}
			else if (a == "--connect") {
				should_connect = true;
				
				if (argc > 2) {
					connect_address = argv[i++];
				}
			}
			else {
				debugger_target = a;
			}
		}
	}
};