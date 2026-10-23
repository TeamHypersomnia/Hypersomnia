#pragma once
#if HEADLESS
#include <functional>
#include "augs/filesystem/path_declaration.h"
#include "application/setups/server/tournament/tournament_config.h"
#include "work_result.h"

struct config_json_table;
struct packaged_official_content;
struct cmd_line_params;

#if CRASH_RECOVERY
class server_recovery_worker;
#endif

struct run_tournament_input {
	const packaged_official_content& official;
	const config_json_table& config_pattern;
	const config_json_table& canon_config_with_confd;
	const cmd_line_params& params;
#if CRASH_RECOVERY
	server_recovery_worker& recovery_worker;
#endif
	std::function<bool()> handle_sigint;
	tournament_config config;
	augs::path_type state_file_path;
};

work_result run_tournament(const run_tournament_input&);
#endif
