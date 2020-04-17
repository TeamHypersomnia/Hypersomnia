#include "imgui_controls.h"
#include <imgui/imgui_internal.h>

namespace ImGui {
	bool BeginTabMenuBar(const float y) {
		ImGuiContext& g = *GImGui;
		SetNextWindowPos(ImVec2(0.0f, y));
		SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f + 1.f));

		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));

		if (!Begin("##MainMedasdsanuBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)
			|| !BeginMenuBar())
		{
			End();
			PopStyleVar(4);
			return false;
		}
		g.CurrentWindow->DC.MenuBarOffset.x += g.Style.DisplaySafeAreaPadding.x;
		return true;
	}

	void EndTabMenuBar() {
		EndMenuBar();
		End();
		PopStyleVar(4);
	}

	bool DragIntN(const char* label, int* v, int components, float v_speed, int* v_min, int* v_max, const char* display_format) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
		return false;

		ImGuiContext& g = *GImGui;
		bool value_changed = false;
		BeginGroup();
		PushID(label);
		PushMultiItemsWidths(components, CalcItemWidth());
		for (int i = 0; i < components; i++)
		{
			PushID(i);
			value_changed |= DragInt("##v", &v[i], v_speed, v_min[i], v_max[i], display_format);
			SameLine(0, g.Style.ItemInnerSpacing.x);
			PopID();
			PopItemWidth();
		}
		PopID();

		TextUnformatted(label, FindRenderedTextEnd(label));
		EndGroup();

		return value_changed;
	}
}
