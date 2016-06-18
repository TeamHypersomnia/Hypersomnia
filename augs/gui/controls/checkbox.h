#pragma once
#include "gui/rect.h"
#include "gui/text_drawer.h"
#include <functional>

namespace augs {
	namespace gui {
		namespace controls {
			class checkbox : public rect {
				bool set;
			public:
				std::function<void(bool)> callback;

				virtual void on_change(bool set);

				bool get_state() const;
				operator bool()  const;

				void set_state(bool);

				checkbox(const rect& = rect(), bool set = false,
					const std::function<void(bool)>& callback = nullptr);

				virtual void consume_gui_event(event_info e) override;
			};

			class checklabel : public checkbox {
				void stretch_rc();

			public:
				text_drawer active_text;
				text_drawer inactive_text;

				checklabel(const checkbox&, const std::wstring& label, const text::style& style_active, const text::style& style_inactive);
				checklabel(const checkbox&, const text::fstr& active_str, const text::fstr& inactive_str);

				virtual void on_change(bool set) override;

				void get_member_children(std::vector<rect_id>&) override;

				text_drawer& active_label();
			};
		}
	}
}