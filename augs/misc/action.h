#pragma once

namespace augs {
	class delta;

	class action {
	public:
		bool is_blocking = true;

		virtual bool is_complete() const = 0;
		virtual void on_update(const delta) = 0;
	};
}