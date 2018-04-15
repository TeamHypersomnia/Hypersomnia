#pragma once
#include <imgui/imgui.h>

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);

	bool BeginTabMenuBar(const float y);
	void EndTabMenuBar();

	IMGUI_API bool          DragIntN(const char* label, int* v, int components, float v_speed, int* v_min, int* v_max, const char* display_format);
}