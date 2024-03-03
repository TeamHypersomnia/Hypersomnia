#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <shlobj.h>
#undef min
#undef max

#include "augs/misc/scope_guard.h"

namespace augs {
	GLFWwindow* get_glfw_window(const window::platform_data& d);

	HWND get_hwnd(const window::platform_data& d) {
		return glfwGetWin32Window(get_glfw_window(d));
	}

	static auto get_filter(const std::vector<window::file_dialog_filter>& filters) {
		std::wstring filter;

		auto to_reserve = std::size_t{ 0 };

		for (const auto& f : filters) {
			to_reserve += f.description.length();
			to_reserve += f.extension.length() + 1;
			to_reserve += 2;
		}

		filter.reserve(to_reserve);

		for (const auto& f : filters) {
			const auto description = widen(f.description);
			const auto extension = widen(f.extension);

			filter += widen(f.description);
			filter.push_back(L'\0');
			filter += L'*';
			filter += widen(f.extension);
			filter.push_back(L'\0');
		}

		return filter;
	}

	std::optional<path_type> window::open_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto filter = get_filter(filters);
		const auto title = widen(custom_title);
		
		OPENFILENAME ofn;       // common dialog box structure
		std::array<wchar_t, 400> szFile;
		fill_range(szFile, 0);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile.data();
		ofn.hwndOwner = get_hwnd(*platform);
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = static_cast<DWORD>(szFile.size());
		ofn.lpstrFilter = filter.data();
		ofn.lpstrTitle = title.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;

		// Display the Open dialog box. 

		auto show_after = scope_guard([this]() {
			ShowWindow(get_hwnd(*platform), SW_SHOW);
		});

		if (GetOpenFileName(&ofn) == TRUE) {
			return augs::path_type(ofn.lpstrFile);
		}
		else {
			return std::nullopt;
		}
	}

	static augs::path_type PickContainer(const std::wstring& custom_title) {
		augs::path_type result;
		IFileDialog *pfd;
		
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
			{
				pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}
			
			pfd->SetTitle(custom_title.c_str());

			if (SUCCEEDED(pfd->Show(NULL)))
			{
				IShellItem *psi;
				if (SUCCEEDED(pfd->GetResult(&psi)))
				{
					LPWSTR g_path = nullptr;
					if(!SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &g_path)))
					{
						MessageBox(NULL, L"GetIDListName() failed", NULL, NULL);
					}

					result = augs::path_type(g_path);

					psi->Release();
				}
			}
			pfd->Release();
		}

		return result;
	}

	std::optional<path_type> window::choose_directory_dialog(
		const std::string& custom_title
	) {
		const auto title = widen(custom_title);
		const auto choice = PickContainer(title);

		if (!choice.empty()) {
			return choice;
		}

		return std::nullopt;
	}

	std::optional<path_type> window::save_file_dialog(
		const std::vector<file_dialog_filter>& filters,
		const std::string& custom_title
	) {
		const auto filter = get_filter(filters);
		const auto title = widen(custom_title);

		OPENFILENAME ofn;       // common dialog box structure
		std::array<wchar_t, 400> szFile;
		fill_range(szFile, 0);

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile.data();
		ofn.hwndOwner = get_hwnd(*platform);
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = static_cast<DWORD>(szFile.size());
		ofn.lpstrFilter = filter.data();
		ofn.lpstrTitle = title.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		// Display the Open dialog box. 

		auto show_after = scope_guard([this]() {
			ShowWindow(get_hwnd(*platform), SW_SHOW);
		});

		if (GetSaveFileName(&ofn) == TRUE) {
			auto result = augs::path_type(ofn.lpstrFile);
			const auto supposed_extension = filters[ofn.nFilterIndex - 1].extension;

			if (supposed_extension != ".*") {
				if (result.extension() != supposed_extension) {
					result += filters[ofn.nFilterIndex - 1].extension;
				}
			}

			return result;
		}
		else {
			return std::nullopt;
		}
	}

	static void BrowseToFile(LPCTSTR filename)
	{
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		const auto pidl = ILCreateFromPath(filename);
		
		if (pidl) {
			LOG("pidl non-null");
			SHOpenFolderAndSelectItems(pidl,0,0,0);
			ILFree(pidl);
		}
		else {
			LOG("pidl is null");
		}

		CoUninitialize();
	}

	void window::reveal_in_explorer(const augs::path_type& p) {
		auto absolute_path = std::filesystem::absolute(p);
		const auto wide_path = absolute_path.wstring();
		LOG_NVPS(absolute_path.string());
		BrowseToFile(wide_path.c_str());
	}

	static auto to_message_box_button(const int button_code) {
		switch (button_code) {
			case IDCANCEL:
				return message_box_button::CANCEL;
			case IDRETRY:
				return message_box_button::RETRY;
			default:
				return message_box_button::CANCEL;
		}
	}

	message_box_button window::retry_cancel(
		const std::string& caption,
		const std::string& text
	) {
		const auto wide_caption = widen(caption);
		const auto wide_text = widen(text);

		return to_message_box_button(MessageBox(get_hwnd(*platform), wide_text.c_str(), wide_caption.c_str(), MB_RETRYCANCEL | MB_ICONEXCLAMATION));
	}
}
