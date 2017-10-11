#pragma once
#include <imgui/imgui.h>

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);

	bool BeginTabMenuBar(const float y);
	void EndTabMenuBar();
}