#pragma once
#include <functional>
#include "3rdparty/imgui/imgui.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/graphics/rgba.h"

namespace augs {
	namespace imgui {
		bool selectable_with_icon(
			const augs::atlas_entry& icon,
			const std::string& label,
			float size_mult,
			vec2 padding_mult,
			rgba label_color,
			std::array<rgba, 3> bg_cols,
			float rotation = 0.0f,
			bool pad_from_left = true,
			std::function<void()> after_cb = nullptr
		);
	}
}

namespace ImGui {
	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder = NULL);

	bool BeginTabMenuBar(const float y);
	void EndTabMenuBar();

	IMGUI_API bool          DragIntN(const char* label, int* v, int components, float v_speed, int* v_min, int* v_max, const char* display_format);
}