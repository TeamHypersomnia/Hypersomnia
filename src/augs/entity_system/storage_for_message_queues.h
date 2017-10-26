#pragma once
#include <tuple>
#include <vector>
#include "augs/templates/for_each_std_get.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/ensure.h"

namespace augs {
	template<class... Queues>
	class storage_for_message_queues {
		typedef std::tuple<std::vector<Queues>...> tuple_type;
		tuple_type queues;

		template <typename T>
		static void check_valid() {
			static_assert(is_one_of_v<T, Queues...>, "Unknown message type!");
		}

	public:
		template <typename T>
		void post(const T& message_object) {
			check_valid<T>();
			get_queue<T>().push_back(message_object);
		}

		template <typename T>
		void post(const std::vector<T>& messages) {
			check_valid<T>();
			concatenate(get_queue<T>(), messages);
		}

		template <typename T>
		std::vector<T>& get_queue() {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <typename T>
		const std::vector<T>& get_queue() const {
			check_valid<T>();
			return std::get<std::vector<T>>(queues);
		}

		template <typename T>
		void clear_queue() {
			check_valid<T>();
			return get_queue<T>().clear();
		}

		void ensure_all_empty() {
			for_each_through_std_get(queues, [this](auto& q) { ensure(q.empty()); });
		}

		void flush_queues() {
			for_each_through_std_get(queues, [this](auto& q) { q.clear(); });
		}
	};
}