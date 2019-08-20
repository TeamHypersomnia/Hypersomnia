#pragma once
#include <string>

std::string get_path_in_log_files(const std::string& name);

std::string get_ensure_failed_path();
std::string get_exit_success_path();
std::string get_exit_failure_path();
std::string get_dumped_log_path();

std::string find_last_incorrect_exit();
