#pragma once
#include "3rdparty/imgui/imgui.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/graphics/rgba.h"

namespace augs {
	namespace imgui {
		bool selectable_with_icon(
			const augs::atlas_entry& icon,
			const std::string& label,
			const float size_mult,
			const float padding_mult,
			const rgba label_color,
			const std::array<rgba, 3> bg_cols
		);
	}
}

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);

	bool BeginTabMenuBar(const float y);
	void EndTabMenuBar();

	IMGUI_API bool          DragIntN(const char* label, int* v, int components, float v_speed, int* v_min, int* v_max, const char* display_format);
}