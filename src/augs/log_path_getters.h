#pragma once
#include <string>

#include "augs/filesystem/path_declaration.h"

augs::path_type get_log_file_path(const std::string& name);

augs::path_type get_exit_success_path();
augs::path_type get_dumped_log_path();
augs::path_type get_error_log_path();
augs::path_type get_startup_log_path();
