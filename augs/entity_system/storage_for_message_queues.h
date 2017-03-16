#pragma once
#include <tuple>
#include <vector>
#include "augs/templates/tuple_of.h"
#include "augs/templates/for_each_in_types.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/type_mod_templates.h"
#include "augs/ensure.h"

namespace augs {
	template<class... Queues>
	class storage_for_message_queues {
		typedef tuple_of_t<make_vector, Queues...> tuple_type;
		tuple_type queues;

	public:
		template <typename T>
		void post(const T& message_object) {
			get_queue<T>().push_back(message_object);
		}

		template <typename T>
		void post(const std::vector<T>& messages) {
			concatenate(get_queue<T>(), messages);
		}

		template <typename T>
		std::vector<T>& get_queue() {
			return std::get<std::vector<T>>(queues);
		}

		template <typename T>
		const std::vector<T>& get_queue() const {
			return std::get<std::vector<T>>(queues);
		}

		template <typename T>
		void clear_queue() {
			return get_queue<T>().clear();
		}

		template <typename T>
		void delete_marked(std::vector<T>& messages) {
			messages.erase(std::remove_if(messages.begin(), messages.end(), [](const T& msg) {
				return msg.delete_this_message;
			}), messages.end());
		}

		template <typename T>
		void delete_marked() {
			delete_marked_messages(get_queue<T>());
		}

		void ensure_all_empty() {
			for_each_in_tuple(queues, [this](auto& q) { ensure(q.empty()); });
		}

		void flush_queues() {
			for_each_in_tuple(queues, [this](auto& q) { q.clear(); });
		}
	};
}