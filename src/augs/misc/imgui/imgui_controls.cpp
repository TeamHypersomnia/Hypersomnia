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
		g.CurrentWindow->DC.MenuBarOffsetX += g.Style.DisplaySafeAreaPadding.x;
		return true;
	}

	void EndTabMenuBar() {
		EndMenuBar();
		End();
		PopStyleVar(4);
	}
}
