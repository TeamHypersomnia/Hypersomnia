#include "imgui_utils.h"
#include "augs/image/image.h"
#include "augs/graphics/texture.h"
#include "augs/window_framework/window.h"

namespace augs {
	namespace imgui {
		void init(
			const char* const ini_filename,
			const char* const log_filename
		) {
			auto& io = ImGui::GetIO();
			
			using namespace augs::event::keys;

			auto map_key = [&io](auto im, auto aug) {
				io.KeyMap[im] = int(aug);
			};

			map_key(ImGuiKey_Tab, key::TAB);
			map_key(ImGuiKey_LeftArrow, key::LEFT);
			map_key(ImGuiKey_RightArrow, key::RIGHT);
			map_key(ImGuiKey_UpArrow, key::UP);
			map_key(ImGuiKey_DownArrow, key::DOWN);
			map_key(ImGuiKey_PageUp, key::PAGEUP);
			map_key(ImGuiKey_PageDown, key::PAGEDOWN);
			map_key(ImGuiKey_Home, key::HOME);
			map_key(ImGuiKey_End, key::END);
			map_key(ImGuiKey_Delete, key::DEL);
			map_key(ImGuiKey_Backspace, key::BACKSPACE);
			map_key(ImGuiKey_Enter, key::ENTER);
			map_key(ImGuiKey_Escape, key::ESC);
			map_key(ImGuiKey_A, key::A);
			map_key(ImGuiKey_C, key::C);
			map_key(ImGuiKey_V, key::V);
			map_key(ImGuiKey_X, key::X);
			map_key(ImGuiKey_Y, key::Y);
			map_key(ImGuiKey_Z, key::Z);

			io.IniFilename = ini_filename;
			io.LogFilename = log_filename;
			io.MouseDoubleClickMaxDist = 100.f;
		}

		void setup_input(
			local_entropy& window_inputs,
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		) {
			using namespace event;
			using namespace event::keys;

			auto& io = ImGui::GetIO();

			io.MouseDrawCursor = false;

			for (const auto& in : window_inputs) {
				if (in.msg == message::mousemotion) {
					io.MousePos = vec2(in.mouse.pos);
				}
				else if (
					in.msg == message::ldown 
					|| in.msg == message::ldoubleclick 
					|| in.msg == message::ltripleclick
				) {
					io.MouseDown[0] = true;
				}
				else if (in.msg == message::lup) {
					io.MouseDown[0] = false;
				}
				else if (
					in.msg == message::rdown 
					|| in.msg == message::rdoubleclick
				) {
					io.MouseDown[1] = true;
				}
				else if (in.msg == message::rup) {
					io.MouseDown[1] = false;
				}
				else if (in.msg == message::wheel) {
					io.MouseWheel = static_cast<float>(in.scroll.amount);
				}
				else if (in.msg == message::keydown) {
					io.KeysDown[static_cast<int>(in.key.key)] = true;
				}
				else if (in.msg == message::keyup) {
					io.KeysDown[static_cast<int>(in.key.key)] = false;
				}
				else if (in.msg == message::character) {
					io.AddInputCharacter(in.character.utf16);
				}

				io.KeyCtrl = io.KeysDown[static_cast<int>(keys::key::LCTRL)] || io.KeysDown[static_cast<int>(keys::key::RCTRL)];
				io.KeyShift = io.KeysDown[static_cast<int>(keys::key::LSHIFT)] || io.KeysDown[static_cast<int>(keys::key::RSHIFT)];
				io.KeyAlt = io.KeysDown[static_cast<int>(keys::key::LALT)] || io.KeysDown[static_cast<int>(keys::key::RALT)];
			}

			io.DeltaTime = delta_seconds;
			io.DisplaySize = vec2(screen_size);
		}

#if 0
		void setup_input(
			event::state& state,
			const decltype(ImGuiIO::DeltaTime) delta_seconds,
			const vec2i screen_size
		) {
			using namespace event;
			using namespace event::keys;

			auto& io = ImGui::GetIO();

			io.MouseDrawCursor = false;
			io.MousePos = vec2(state.mouse.pos);
			io.MouseDown[0] = state.keys[key::LMOUSE];
			io.MouseDown[1] = state.keys[key::RMOUSE];
				else if (
					in.msg == message::ldown
					|| in.msg == message::ldoubleclick
					|| in.msg == message::ltripleclick
					) {
					io.MouseDown[0] = true;
				}
				else if (in.msg == message::lup) {
					io.MouseDown[0] = false;
				}
				else if (
					in.msg == message::rdown
					|| in.msg == message::rdoubleclick
					) {
					io.MouseDown[1] = true;
				}
				else if (in.msg == message::rup) {
					io.MouseDown[1] = false;
				}
				else if (in.msg == message::wheel) {
					io.MouseWheel = static_cast<float>(in.scroll.amount);
				}
				else if (in.msg == message::keydown) {
					io.KeysDown[static_cast<int>(in.key.key)] = true;
				}
				else if (in.msg == message::keyup) {
					io.KeysDown[static_cast<int>(in.key.key)] = false;
				}
				else if (in.msg == message::character) {
					io.AddInputCharacter(in.character.utf16);
				}
			}

			io.KeyCtrl = io.KeysDown[static_cast<int>(keys::key::LCTRL)] || io.KeysDown[static_cast<int>(keys::key::RCTRL)];
			io.KeyShift = io.KeysDown[static_cast<int>(keys::key::LSHIFT)] || io.KeysDown[static_cast<int>(keys::key::RSHIFT)];
			io.KeyAlt = io.KeysDown[static_cast<int>(keys::key::LALT)] || io.KeysDown[static_cast<int>(keys::key::RALT)];

			io.DeltaTime = delta_seconds;
			io.DisplaySize = vec2(screen_size);
		}
#endif
		void render(const ImGuiStyle& style) {
			ImGui::GetStyle() = style;
			ImGui::Render();
		}

		void neutralize_mouse() {
			auto& io = ImGui::GetIO();

			io.MousePos = { -1, -1 };

			for (auto& m : io.MouseDown) {
				m = false;
			}
		}
		
		image create_atlas_image() {
			auto& io = ImGui::GetIO();

			unsigned char* pixels = nullptr;
			int width = 0;
			int height = 0;

			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			io.Fonts->TexID = reinterpret_cast<void*>(0);

			return { pixels, 4, 0, vec2i{ width, height } };
		}

		graphics::texture create_atlas() {
			return create_atlas_image();
		}
	}
}

namespace ImGui {
	// thanks to https://github.com/ocornut/imgui/issues/261#issuecomment-239710635
	/*
	tabLabels: name of all tabs involved
	tabSize: number of elements
	tabIndex: holds the current active tab
	tabOrder: optional array of integers from 0 to tabSize-1 that maps the tab label order. If one of the numbers is replaced by -1 the tab label is not visible (closed). It can be read/modified at runtime.

	// USAGE EXAMPLE
	static const char* tabNames[] = {"First tab","Second tab","Third tab"};
	static int tabOrder[] = {0,1,2};
	static int tabSelected = 0;
	const bool tabChanged = ImGui::TabLabels(tabNames,sizeof(tabNames)/sizeof(tabNames[0]),tabSelected,tabOrder);
	ImGui::Text("\nTab Page For Tab: \"%s\" here.\n",tabNames[tabSelected]);
	*/

	IMGUI_API bool TabLabels(const char **tabLabels, int tabSize, int &tabIndex, int *tabOrder) {
		ImGuiStyle& style = ImGui::GetStyle();

		const ImVec2 itemSpacing = style.ItemSpacing;
		const ImVec4 color = style.Colors[ImGuiCol_Button];
		const ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		const ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];
		const ImVec4 colorText = style.Colors[ImGuiCol_Text];
		style.ItemSpacing.x = 0;
		style.ItemSpacing.y = 0;
		const ImVec4 colorSelectedTab = ImVec4(color.x, color.y, color.z, color.w*0.f);
		const ImVec4 colorSelectedTabHovered = ImVec4(colorHover.x, colorHover.y, colorHover.z, colorHover.w*0.7f);
		const ImVec4 colorSelectedTabText = colorText;

		if (tabSize>0 && (tabIndex<0 || tabIndex >= tabSize)) {
			if (!tabOrder)  tabIndex = 0;
			else tabIndex = -1;
		}

		float windowWidth = 0.f, sumX = 0.f;
		windowWidth = ImGui::GetWindowWidth() - style.WindowPadding.x - (ImGui::GetScrollMaxY()>0 ? style.ScrollbarSize : 0.f);

		static int draggingTabIndex = -1; int draggingTabTargetIndex = -1;   // These are indices inside tabOrder
		static ImVec2 draggingtabSize(0, 0);
		static ImVec2 draggingTabOffset(0, 0);

		const bool isMMBreleased = ImGui::IsMouseReleased(2);
		const bool isMouseDragging = ImGui::IsMouseDragging(0, 2.f);
		int justClosedTabIndex = -1, newtabIndex = tabIndex;


		bool selection_changed = false; bool noButtonDrawn = true;
		for (int j = 0, i; j < tabSize; j++)
		{
			i = tabOrder ? tabOrder[j] : j;
			if (i == -1) continue;

			if (sumX > 0.f) {
				sumX += style.ItemSpacing.x;   // Maybe we can skip it if we use SameLine(0,0) below
				sumX += ImGui::CalcTextSize(tabLabels[i]).x + 2.f*style.FramePadding.x;
				if (sumX>windowWidth) sumX = 0.f;
				else ImGui::SameLine();
			}

			if (i != tabIndex) {
				// Push the style
				style.Colors[ImGuiCol_Button] = colorSelectedTab;
				style.Colors[ImGuiCol_ButtonActive] = colorActive;
				style.Colors[ImGuiCol_ButtonHovered] = colorHover;
				style.Colors[ImGuiCol_Text] = colorSelectedTabText;
			}
			// Draw the button
			ImGui::PushID(i);   // otherwise two tabs with the same name would clash.
			if (ImGui::Button(tabLabels[i])) { selection_changed = (tabIndex != i); newtabIndex = i; }
			ImGui::PopID();
			if (i != tabIndex) {
				// Reset the style
				style.Colors[ImGuiCol_Button] = color;
				style.Colors[ImGuiCol_ButtonActive] = colorActive;
				style.Colors[ImGuiCol_ButtonHovered] = colorHover;
				style.Colors[ImGuiCol_Text] = colorText;
			}
			noButtonDrawn = false;

			if (sumX == 0.f) sumX = style.WindowPadding.x + ImGui::GetItemRectSize().x; // First element of a line

			if (ImGui::IsItemHoveredRect()) {
				if (tabOrder) {
					// tab reordering
					if (isMouseDragging) {
						if (draggingTabIndex == -1) {
							draggingTabIndex = j;
							draggingtabSize = ImGui::GetItemRectSize();
							const ImVec2& mp = ImGui::GetIO().MousePos;
							const ImVec2 draggingTabCursorPos = ImGui::GetCursorPos();
							draggingTabOffset = ImVec2(
								mp.x + draggingtabSize.x*0.5f - sumX + ImGui::GetScrollX(),
								mp.y + draggingtabSize.y*0.5f - draggingTabCursorPos.y + ImGui::GetScrollY()
							);

						}
					}
					else if (draggingTabIndex >= 0 && draggingTabIndex<tabSize && draggingTabIndex != j) {
						draggingTabTargetIndex = j; // For some odd reasons this seems to get called only when draggingTabIndex < i ! (Probably during mouse dragging ImGui owns the mouse someway and sometimes ImGui::IsItemHovered() is not getting called)
					}
				}
			}

		}

		tabIndex = newtabIndex;

		// Draw tab label while mouse drags it
		if (draggingTabIndex >= 0 && draggingTabIndex<tabSize) {
			const ImVec2& mp = ImGui::GetIO().MousePos;
			const ImVec2 wp = ImGui::GetWindowPos();
			ImVec2 start(wp.x + mp.x - draggingTabOffset.x - draggingtabSize.x*0.5f, wp.y + mp.y - draggingTabOffset.y - draggingtabSize.y*0.5f);
			const ImVec2 end(start.x + draggingtabSize.x, start.y + draggingtabSize.y);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const float draggedBtnAlpha = 0.65f;
			const ImVec4& btnColor = style.Colors[ImGuiCol_Button];
			drawList->AddRectFilled(start, end, ImColor(btnColor.x, btnColor.y, btnColor.z, btnColor.w*draggedBtnAlpha), style.FrameRounding);
			start.x += style.FramePadding.x; start.y += style.FramePadding.y;
			const ImVec4& txtColor = style.Colors[ImGuiCol_Text];
			drawList->AddText(start, ImColor(txtColor.x, txtColor.y, txtColor.z, txtColor.w*draggedBtnAlpha), tabLabels[tabOrder[draggingTabIndex]]);

			ImGui::SetMouseCursor(ImGuiMouseCursor_Move);
		}

		// Drop tab label
		if (draggingTabTargetIndex != -1) {
			// swap draggingTabIndex and draggingTabTargetIndex in tabOrder
			const int tmp = tabOrder[draggingTabTargetIndex];
			tabOrder[draggingTabTargetIndex] = tabOrder[draggingTabIndex];
			tabOrder[draggingTabIndex] = tmp;
			//fprintf(stderr,"%d %d\n",draggingTabIndex,draggingTabTargetIndex);
			draggingTabTargetIndex = draggingTabIndex = -1;
		}

		// Reset draggingTabIndex if necessary
		if (!isMouseDragging) draggingTabIndex = -1;

		// Change selected tab when user closes the selected tab
		if (tabIndex == justClosedTabIndex && tabIndex >= 0) {
			tabIndex = -1;
			for (int j = 0, i; j < tabSize; j++) {
				i = tabOrder ? tabOrder[j] : j;
				if (i == -1) continue;
				tabIndex = i;
				break;
			}
		}

		// Restore the style
		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		style.Colors[ImGuiCol_Text] = colorText;
		style.ItemSpacing = itemSpacing;

		return selection_changed;
	}
}