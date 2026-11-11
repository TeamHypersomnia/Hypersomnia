#pragma once
#include <string>
#include <optional>

#include "augs/filesystem/path_declaration.h"

augs::path_type get_log_file_path(const std::string& name);
std::string get_path_in_log_files(const std::string& name);

std::string get_exit_success_path();
std::string get_dumped_log_path();
augs::path_type get_error_log_path();
augs::path_type get_startup_log_path();
