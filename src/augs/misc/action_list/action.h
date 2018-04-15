#pragma once

namespace augs {
	class delta;

	class action {
		bool has_started = false;
		friend class action_list;

	public:
		bool is_blocking = true;

		virtual bool is_complete() const = 0;
		virtual void on_update(const delta) = 0;
		virtual void on_enter() = 0;

		virtual ~action() {}
	};
}