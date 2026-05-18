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

	/*
		HACK: hardcoded version data to exactly match the 2.0.0-pre1 binary
		on production for crash core debugging. String lengths in constructor
		determine .text address layout, so they must match byte-for-byte.
	*/
	std::string version_string = "2.0.0-pre1";
	std::cout << "Detected version: " << version_string << std::endl;

	std::string git_commit_message = "allow -pre* vers";
	const std::string git_commit_date = "Mon May 18 00:55:37 2026";
	const std::string git_commit_hash = "4df328bacd9974dba4cd8ad4ac71f5bafcaa3f84";

	std::istringstream git_working_tree_changes ("");

	std::string git_working_tree_changes_lines;

	for (std::string line; std::getline(git_working_tree_changes, line);) {
		git_working_tree_changes_lines += "\t\t\"" + line + "\",\n";
	}

	const auto output_file_contents = typesafe_sprintf(
		input_file_contents,
		"[disabled]",//augs::date_time().get_timestamp(),
		argc,
		debug_argv_content,
		version_string,
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