#define FORCE_DISABLE_ENSURE 1

#include <iostream>
#include <chrono>

using namespace std::chrono;

#include "augs/misc/scope_guard.h"
#include "augs/window_framework/exec.h"
#include "augs/filesystem/file.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/string/string_templates.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#include "augs/filesystem/winapi_exists.hpp"
#endif

int main(int argc, char** argv) {
	auto start = high_resolution_clock::now();

	std::cout << "------------\nversion_file_generator run" << std::endl;

	auto end_message = augs::scope_guard([&]() {
		std::cout << "Run time: " << duration_cast<duration<double, milliseconds::period>>(high_resolution_clock::now() - start).count() << " ms" << std::endl << "------------\n";
	});

	if (argc < 2) {
		std::cout << "Path to GIT executable was not specified!\nFailure";
		return 1;
	}

	if (argc < 3) {
		std::cout << "Path to the input file was not specified!\nFailure";
		return 1;
	}

	if (argc < 4) {
		std::cout << "Path to the output file was not specified!\nFailure";
		return 1;
	}
	
	const auto git_executable_path = augs::path_type("\"" + std::string(argv[1]) + "\"");
	const auto input_file_path = augs::path_type(argv[2]);
	const auto output_file_path = augs::path_type(argv[3]);

	const auto input_file_contents = augs::file_to_string(input_file_path);
	
	std::string debug_argv_content;

	for (int i = 0; i < argc; ++i) {
		debug_argv_content += std::string("\n\t\t") + argv[i];
	}

	debug_argv_content += "\n";

	const auto git_commit_number = augs::exec(git_executable_path.string() + " rev-list --count master");
	
	auto git_commit_message = augs::exec(git_executable_path.string() + " log -1 --format=%s");
	// We shall add the backslash both before \ and " to avoid compilation errors
	str_ops(git_commit_message)
		.replace_all("\\", "\\\\")
		.replace_all("\"", "\\\"")
	;

	const auto git_commit_date = augs::exec(git_executable_path.string() + " log -1 --format=%ad --date=local");
	const auto git_commit_hash = augs::exec(git_executable_path.string() + " rev-parse --verify HEAD");

	std::istringstream git_working_tree_changes (augs::exec(git_executable_path.string() + " diff --name-only"));

	std::string git_working_tree_changes_lines;

	for (std::string line; std::getline(git_working_tree_changes, line);) {
		git_working_tree_changes_lines += "\t\t\"" + line + "\",\n";
	}

	const auto output_file_contents = typesafe_sprintf(
		input_file_contents,
		"[disabled]",//augs::date_time().get_timestamp(),
		argc,
		debug_argv_content,
		git_commit_number,
		git_commit_message,
		git_commit_date,
		git_commit_hash,
		git_working_tree_changes_lines
	);

	augs::save_as_text_if_different(
		output_file_path,
		output_file_contents
	);

	std::cout << "Success" << std::endl;
	return 0;
}