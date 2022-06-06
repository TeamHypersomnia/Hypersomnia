#pragma once
#include "application/setups/debugger/editor_folder.h"

struct editor_significant {
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<editor_folder> folders;
};
