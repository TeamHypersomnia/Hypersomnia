#pragma once
#include <tuple>
#include <vector>

#include "augs/templates/for_each_std_get.h"
#include "augs/templates/folded_finders.h"
#include "augs/templates/container_templates.h"

namespace augs {
	template<class... Queues>
	class storage_for_message_queues {
		using tuple_type = std::tuple<std::vector<Queues>...>;
		tuple_type queues;

		template <class T>
		static void check_valid() {
			static_assert(is_one_of_v<T, Queues...>, "Unknown message type!");
		}

	public:
		template <class T>
		void post(const T& message_object) {
			check_valid<T>();
			get_queue<T>().push_back(message_object);
		}

		template <class T>
		void post(const std::vector<T>& messages) {
			check_valid<T>();
			concatenate(get_queue<T>(), messages);
		}

		template <class T>
		std::vector<T>& get_queue() {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <class T>
		const std::vector<T>& get_queue() const {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <class T>
		void clear_queue() {
			check_valid<T>();
			return get_queue<T>().clear();
		}

		void flush_queues() {
			for_each_through_std_get(queues, [](auto& q) { q.clear(); });
		}
	};
}