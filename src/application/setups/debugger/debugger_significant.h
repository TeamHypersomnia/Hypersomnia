#pragma once
#include "application/setups/debugger/debugger_folder.h"

struct debugger_significant {
	folder_index current_index = static_cast<folder_index>(-1);
	std::vector<debugger_folder> folders;
};
