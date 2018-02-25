#pragma once
#include "application/setups/editor/editor_tab.h"

struct editor_significant {
	tab_index_type current_index = static_cast<tab_index_type>(-1);

	std::vector<editor_tab> tabs;
	std::vector<std::unique_ptr<intercosm>> works;
};
