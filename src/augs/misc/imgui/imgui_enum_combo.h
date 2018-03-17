#include "augs/templates/enum_introspect.h"

namespace augs {
	namespace imgui {
		template <class T, class... Args>
		auto enum_combo(const std::string& label, T& into, Args&&... args) {
			thread_local const auto combo_names = []() {
				std::vector<char> names;

				for_each_enum_except_bounds([&names](const T e) {
					concatenate(
						names,
						format_enum(e)
					);

					names.push_back('\0');
				});
				
				names.push_back('\0');

				return names;
			}();

			auto current = static_cast<int>(into);
			const auto result = ImGui::Combo(label.c_str(), &current, combo_names.data());
			into = static_cast<T>(current);

			return result;
		}
	}
}
