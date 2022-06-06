#pragma once
#include "3rdparty/imgui/imgui.h"

template <class T>
bool detail_select_none(T& id) {
	const bool result = ImGui::Selectable("(None)", id == T());

	if (result) {
		id = {};
	}

	ImGui::Separator();
	return result;
}

