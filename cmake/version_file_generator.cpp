#include "augs/window_framework/exec.h"
#include "augs/filesystem/file.h"
#include "augs/misc/typesafe_sprintf.h"
#include "augs/misc/time_utils.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		throw std::runtime_error("Path to GIT executable was not specified!");
	}

	if (argc < 3) {
		throw std::runtime_error("Path to the input file was not specified!");
	}

	if (argc < 4) {
		throw std::runtime_error("Path to the output file was not specified!");
	}
	
	const std::string git_executable_path = "\"" + std::string(argv[1]) + "\"";
	const std::string input_file_path = argv[2];
	const std::string output_file_path = argv[3];

	const auto input_file_contents = augs::get_file_contents(input_file_path);
	
	std::string debug_argv_content;

	for (int i = 0; i < argc; ++i) {
		debug_argv_content += std::string("\n\t\t") + argv[i];
	}

	debug_argv_content += "\n";

	const auto git_commit_number = augs::exec(git_executable_path + " rev-list --count master");
	const auto git_commit_message = augs::exec(git_executable_path + " log -1 --format=%s");
	const auto git_commit_date = augs::exec(git_executable_path + " log -1 --format=%ad --date=local");
	const auto git_commit_hash = augs::exec(git_executable_path + " rev-parse --verify HEAD");

	std::istringstream git_working_tree_changes (augs::exec(git_executable_path + " diff --name-only"));

	std::string git_working_tree_changes_lines;

	for (std::string line; std::getline(git_working_tree_changes, line);) {
		git_working_tree_changes_lines += "\t\"" + line + "\",\n";
	}

	const auto output_file_contents = typesafe_sprintf(
		input_file_contents,
		"[disabled]",//augs::get_timestamp(),
		argc,
		debug_argv_content,
		git_commit_number,
		git_commit_message,
		git_commit_date,
		git_commit_hash,
		git_working_tree_changes_lines
	);

	augs::create_text_file_if_different(
		output_file_path,
		output_file_contents
	);

	return 0;
}